<?php
header('Content-Type: application/json');
header('Access-Control-Allow-Origin: *');

session_start();

// Importiamo le configurazioni e le password in modo sicuro
require 'config.php';

$requestedId = isset($_GET['id']) ? (int)$_GET['id'] : null;

// --- 1. SISTEMA DI CACHE DEI DATI ---
// Se stiamo chiedendo tutte le arnie (homepage) e i dati sono stati scaricati meno di 5 secondi fa, usa la cache
if (!$requestedId && isset($_SESSION['data_cache']) && isset($_SESSION['data_time'])) {
    if (time() - $_SESSION['data_time'] < 5) {
        // Restituisce i dati salvati istantaneamente senza contattare ThingsBoard
        echo json_encode($_SESSION['data_cache']);
        exit;
    }
}

// --- 2. GESTIONE TOKEN CON CACHE ---
function getToken($host, $username, $password) {
    if (isset($_SESSION['token']) && isset($_SESSION['token_time'])) {
        if (time() - $_SESSION['token_time'] < 3600) { // Valido per 1 ora
            return $_SESSION['token'];
        }
    }

    $ch = curl_init("$host/api/auth/login");
    curl_setopt_array($ch, [
        CURLOPT_RETURNTRANSFER => true,
        CURLOPT_POST => true,
        CURLOPT_POSTFIELDS => json_encode(["username" => $username, "password" => $password]),
        CURLOPT_HTTPHEADER => ['Content-Type: application/json', 'Accept: application/json'],
        CURLOPT_SSL_VERIFYPEER => false,
        CURLOPT_SSL_VERIFYHOST => false
    ]);

    $response = curl_exec($ch);
    if (curl_errno($ch)) { echo json_encode(['error' => 'Errore cURL login']); exit; }
    curl_close($ch);

    $data = json_decode($response, true);
    if (!isset($data['token'])) { echo json_encode(['error' => 'Login fallito']); exit; }

    $_SESSION['token'] = $data['token'];
    $_SESSION['token_time'] = time();

    return $data['token'];
}

$token = getToken($TB_HOST, $TB_USERNAME, $TB_PASSWORD);

// --- 3. RICHIESTE PARALLELE CON cURL MULTI ---
$results = [];
$mh = curl_multi_init(); // Inizializza il gestore per richieste multiple
$curl_handles = [];

// Prepariamo tutte le chiamate da fare
foreach ($TB_DEVICES as $hiveId => $deviceId) {
    if ($requestedId && $requestedId !== $hiveId) continue;

    $url = "$TB_HOST/api/plugins/telemetry/DEVICE/$deviceId/values/timeseries?keys=tempIn,humidity,weight,battery,honeyPct,tempOut";
    $ch = curl_init($url);

    curl_setopt_array($ch, [
        CURLOPT_RETURNTRANSFER => true,
        CURLOPT_HTTPHEADER => [
            "X-Authorization: Bearer $token",
            "Accept: application/json"
        ],
        CURLOPT_SSL_VERIFYPEER => false,
        CURLOPT_SSL_VERIFYHOST => false
    ]);

    curl_multi_add_handle($mh, $ch);
    $curl_handles[$hiveId] = $ch;
}

// Eseguiamo tutte le richieste contemporaneamente (super veloce!)
$active = null;
do {
    $status = curl_multi_exec($mh, $active);
    if ($active) {
        curl_multi_select($mh);
    }
} while ($active && $status == CURLM_OK);

// Raccogliamo i risultati e chiudiamo le connessioni
foreach ($curl_handles as $hiveId => $ch) {
    $response = curl_multi_getcontent($ch);
    $results[$hiveId] = json_decode($response, true);
    curl_multi_remove_handle($mh, $ch);
}
curl_multi_close($mh);

// --- 4. SALVATAGGIO IN CACHE E RISPOSTA ---
// Salviamo in cache solo se abbiamo scaricato tutto (non un'arnia singola)
if (!$requestedId) {
    $_SESSION['data_cache'] = $results;
    $_SESSION['data_time'] = time();
}

echo json_encode($results);
?>