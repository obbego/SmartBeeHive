<?php
// Impostiamo l'intestazione per dire al browser che restituiremo un JSON
header('Content-Type: application/json');
header('Access-Control-Allow-Origin: *'); // Evita blocchi CORS durante i test

$TB_HOST = "https://eu.thingsboard.cloud";
$TB_USERNAME = "francesco.bego@iisviolamarchesini.edu.it";
$TB_PASSWORD = "ApiApi1234!";

// DEVICE ID messi in sicurezza nel backend
$TB_DEVICES = [
    1 => "83ada8d0-171e-11f1-acb1-ebc343e93a59",
    2 => "0c2d9880-1c67-11f1-a469-05ae34b6a511",
    3 => "19898c50-1c67-11f1-a469-05ae34b6a511",
    4 => "24507d60-1c67-11f1-b0fa-13069a1cfc9d",
    5 => "387ab0d0-1c67-11f1-b0fa-13069a1cfc9d"
];

// 1. LOGIN SU THINGSBOARD TRAMITE cURL
$ch = curl_init("$TB_HOST/api/auth/login");
curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
curl_setopt($ch, CURLOPT_POST, true);
curl_setopt($ch, CURLOPT_POSTFIELDS, json_encode(['username' => $TB_USERNAME, 'password' => $TB_PASSWORD]));
curl_setopt($ch, CURLOPT_HTTPHEADER, ['Content-Type: application/json', 'Accept: application/json']);

// DISABILITA IL CONTROLLO SSL PER IL TESTING LOCALE
curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, false);
curl_setopt($ch, CURLOPT_SSL_VERIFYHOST, false);

$loginResponse = curl_exec($ch);

// Gestione errori cURL per farti capire se c'è un altro problema
if(curl_errno($ch)){
    echo json_encode(['error' => 'Errore cURL nel login: ' . curl_error($ch)]);
    exit;
}
curl_close($ch);

$tokenData = json_decode($loginResponse, true);
$token = isset($tokenData['token']) ? $tokenData['token'] : null;

if (!$token) {
    echo json_encode(['error' => 'Login a ThingsBoard fallito nel backend PHP', 'response' => $loginResponse]);
    exit;
}

// 2. CAPIRE COSA CHIEDE IL FRONTEND (Tutte le arnie o una sola?)
$requestedId = isset($_GET['id']) ? (int)$_GET['id'] : null;
$results = [];

// 3. RECUPERO DEI DATI REALI
foreach ($TB_DEVICES as $hiveId => $deviceId) {
    // Se il frontend ha chiesto un'arnia specifica, salto le altre
    if ($requestedId && $requestedId !== $hiveId) continue;

    $url = "$TB_HOST/api/plugins/telemetry/DEVICE/$deviceId/values/timeseries?keys=tempIn,humidity,weight,battery,honeyPct,tempOut";
    $ch = curl_init($url);
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
    curl_setopt($ch, CURLOPT_HTTPHEADER, [
        "X-Authorization: Bearer $token",
        "Accept: application/json"
    ]);

    // DISABILITA IL CONTROLLO SSL ANCHE QUI
    curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, false);
    curl_setopt($ch, CURLOPT_SSL_VERIFYHOST, false);

    $telemetryResponse = curl_exec($ch);
    curl_close($ch);

    $results[$hiveId] = json_decode($telemetryResponse, true);
}

// 4. MANDO I DATI AL JS
echo json_encode($results);
?>