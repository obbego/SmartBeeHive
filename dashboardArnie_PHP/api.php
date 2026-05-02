<?php
date_default_timezone_set('Europe/Rome'); // <-- AGGIUNGI QUESTA RIGA
header('Content-Type: application/json');
header('Access-Control-Allow-Origin: *');
ini_set('display_errors', '0'); // Nasconde i warning PHP testuali

session_start();
require 'config.php';

session_start();
require 'config.php';

$requestedId = isset($_GET['id']) ? (int)$_GET['id'] : null;

// --- 1. CACHE ---
if (!$requestedId && isset($_SESSION['data_cache'], $_SESSION['data_time'])) {
    if (time() - $_SESSION['data_time'] < 5) {
        echo json_encode($_SESSION['data_cache']);
        exit;
    }
}

// --- 2. TOKEN CACHE ---
function getToken($host, $username, $password) {
    if (isset($_SESSION['token'], $_SESSION['token_time'])) {
        if (time() - $_SESSION['token_time'] < 3600) {
            return $_SESSION['token'];
        }
    }

    $ch = curl_init("$host/api/auth/login");
    curl_setopt_array($ch, [
        CURLOPT_RETURNTRANSFER => true,
        CURLOPT_POST => true,
        CURLOPT_POSTFIELDS => json_encode(["username" => $username, "password" => $password]),
        CURLOPT_HTTPHEADER => ['Content-Type: application/json'],
        CURLOPT_SSL_VERIFYPEER => false,
        CURLOPT_SSL_VERIFYHOST => false
    ]);

    $response = curl_exec($ch);
    $data = json_decode($response, true);

    if (!isset($data['token'])) return null;

    $_SESSION['token'] = $data['token'];
    $_SESSION['token_time'] = time();
    return $data['token'];
}

$token = getToken($TB_HOST, $TB_USERNAME, $TB_PASSWORD);

// --- 3. MULTI CURL ---
$mh = curl_multi_init();
$handles = [];

foreach ($TB_DEVICES as $hiveId => $deviceId) {
    if ($requestedId && $requestedId !== $hiveId) continue;

    $interval = $_GET['interval'] ?? 'latest';

    if ($interval === 'latest') {
        // ENDPOINT CORRETTO PER LE LATEST TELEMETRY (Senza startTs, endTs o limit)
        $url = "$TB_HOST/api/plugins/telemetry/DEVICE/$deviceId/values/timeseries"
            . "?keys=tempIn,humidity,honeyWeightKg,battery,honeyPct,tempOut,peakFreq";
    } else {
        // ENDPOINT PER LO STORICO
        $endTs = round(microtime(true) * 1000);
        $startTs = 0;
        $limit = 100;

        switch ($interval) {
            case '24':
            case '24h':
                $startTs = $endTs - (24 * 60 * 60 * 1000);
                $limit = 1000;
                break;
            case '7d':
                $startTs = $endTs - (7 * 24 * 60 * 60 * 1000);
                $limit = 5000;
                break;
            case '30d':
                $startTs = $endTs - (30 * 24 * 60 * 60 * 1000);
                $limit = 15000;
                break;
        }
        $url = "$TB_HOST/api/plugins/telemetry/DEVICE/$deviceId/values/timeseries"
            . "?keys=tempIn,humidity,honeyWeightKg,battery,honeyPct,tempOut,peakFreq"
            . "&startTs=$startTs&endTs=$endTs&limit=$limit&orderBy=DESC";
    }

    $ch = curl_init($url);
    curl_setopt_array($ch, [
        CURLOPT_RETURNTRANSFER => true,
        CURLOPT_HTTPHEADER => ["X-Authorization: Bearer $token", "Accept: application/json"],
        CURLOPT_SSL_VERIFYPEER => false,
        CURLOPT_SSL_VERIFYHOST => false
    ]);

    curl_multi_add_handle($mh, $ch);
    $handles[$hiveId] = $ch;
}

$active = null;
do { $status = curl_multi_exec($mh, $active); } while ($active);

$results = [];
foreach ($handles as $hiveId => $ch) {
    $response = curl_multi_getcontent($ch);
    $data = json_decode($response, true);

    // --- LOGICA CONTROLLO TEMPO REALE E STALE DATA ---
    if (!empty($data) && !isset($data['status'])) {
        $latestTs = 0;
        foreach ($data as $key => $values) {
            if (is_array($values) && isset($values[0]['ts']) && $values[0]['ts'] > $latestTs) {
                $latestTs = $values[0]['ts'];
            }
        }

        if ($latestTs > 0) {
            $data['is_stale'] = ((time() * 1000) - $latestTs) > 86400000; // 24 ore
            $data['last_ts_human'] = date("d/m H:i", (int)($latestTs / 1000));
        } else {
            // Se non c'è nessun timestamp valido, i dati mancano del tutto
            $data['is_stale'] = true;
            $data['last_ts_human'] = "Mai";
        }
    } else {
        // Nessun dato o errore API
        $data = ['is_stale' => true, 'last_ts_human' => "Errore"];
    }

    $results[$hiveId] = $data;
    curl_multi_remove_handle($mh, $ch);
}
curl_multi_close($mh);

// --- 5. ALLARMI (Spostati PRIMA dell'output finale) ---
$alarmUrl = "$TB_HOST/api/alarms?searchStatus=ANY&fetchOriginator=true&pageSize=50&page=0";
$ch = curl_init($alarmUrl);
curl_setopt_array($ch, [
    CURLOPT_RETURNTRANSFER => true,
    CURLOPT_HTTPHEADER => [
        "X-Authorization: Bearer $token",
        "Accept: application/json"
    ],
    CURLOPT_SSL_VERIFYPEER => false,
    CURLOPT_SSL_VERIFYHOST => false
]);
$alarmResponse = curl_exec($ch);

$alarmData = json_decode($alarmResponse, true);
// Aggiungiamo gli allarmi all'array principale
$results['alarms'] = $alarmData['data'] ?? [];


// --- 6. SALVATAGGIO IN CACHE E OUTPUT UNICO ---
if (!$requestedId) {
    $_SESSION['data_cache'] = $results;
    $_SESSION['data_time'] = time();
}

// Spediamo un unico JSON pulito che contiene sia telemetrie che allarmi
echo json_encode($results);
exit;
?>
