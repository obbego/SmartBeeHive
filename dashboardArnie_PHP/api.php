<?php
header('Content-Type: application/json');
header('Access-Control-Allow-Origin: *');

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
        CURLOPT_POSTFIELDS => json_encode([
            "username" => $username,
            "password" => $password
        ]),
        CURLOPT_HTTPHEADER => [
            'Content-Type: application/json',
            'Accept: application/json'
        ],
        CURLOPT_SSL_VERIFYPEER => false,
        CURLOPT_SSL_VERIFYHOST => false
    ]);

    $response = curl_exec($ch);
    if (curl_errno($ch)) {
        echo json_encode(['error' => 'Errore login']);
        exit;
    }
    curl_close($ch);

    $data = json_decode($response, true);
    if (!isset($data['token'])) {
        echo json_encode(['error' => 'Token non ricevuto']);
        exit;
    }

    $_SESSION['token'] = $data['token'];
    $_SESSION['token_time'] = time();

    return $data['token'];
}

$token = getToken($TB_HOST, $TB_USERNAME, $TB_PASSWORD);

// --- 3. MULTI CURL ---
$results = [];
$mh = curl_multi_init();
$handles = [];

foreach ($TB_DEVICES as $hiveId => $deviceId) {
    if ($requestedId && $requestedId !== $hiveId) continue;

    $interval = $_GET['interval'] ?? '24h';
    $endTs = round(microtime(true) * 1000);

    // Intervalli (ma senza limiti aggressivi)
    switch ($interval) {
        case '7d':
            $startTs = $endTs - (7 * 24 * 60 * 60 * 1000);
            break;
        case '30d':
            $startTs = $endTs - (30 * 24 * 60 * 60 * 1000);
            break;
        default:
            $startTs = $endTs - (24 * 60 * 60 * 1000);
    }

    // In api.php, aggiungi &useStrictDataTypes=true e limit=1 se vuoi solo l'ultimo
    $url = "$TB_HOST/api/plugins/telemetry/DEVICE/$deviceId/values/timeseries"
        . "?keys=tempIn,humidity,weight,battery,honeyPct,tempOut,peakFreq"
        . "&startTs=$startTs&endTs=$endTs&limit=1&orderBy=DESC";

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
    $handles[$hiveId] = $ch;
}

// Esecuzione parallela
$active = null;
do {
    $status = curl_multi_exec($mh, $active);
    if ($active) curl_multi_select($mh);
} while ($active && $status == CURLM_OK);

// Raccolta
foreach ($handles as $hiveId => $ch) {
    $response = curl_multi_getcontent($ch);
    $results[$hiveId] = json_decode($response, true);
    curl_multi_remove_handle($mh, $ch);
}
curl_multi_close($mh);

// --- 4. CACHE ---
if (!$requestedId) {
    $_SESSION['data_cache'] = $results;
    $_SESSION['data_time'] = time();
}

// --- 5. ALLARMI ---
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
curl_close($ch);
$alarmData = json_decode($alarmResponse, true);
$results['alarms'] = $alarmData['data'] ?? [];

// --- 6. CACHE (dopo gli allarmi!) ---
if (!$requestedId) {
    $_SESSION['data_cache'] = $results;
    $_SESSION['data_time'] = time();
}

echo json_encode($results);
?>