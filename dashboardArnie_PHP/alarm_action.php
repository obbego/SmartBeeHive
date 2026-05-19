<?php
header('Content-Type: application/json');
session_start();
require 'config.php';

$input   = json_decode(file_get_contents('php://input'), true) ?: [];
$action  = trim($input['action']  ?? '');
$alarmId = trim($input['alarmId'] ?? '');

if (empty($action) || empty($alarmId) || !in_array($action, ['ack', 'clear'])) {
    http_response_code(400);
    echo json_encode(['error' => "Parametri non validi. Servono 'action' (ack|clear) e 'alarmId'."]);
    exit;
}

// Token TB dalla sessione (condivisa con api.php)
function getTbToken($host, $username, $password) {
    if (isset($_SESSION['token'], $_SESSION['token_time']) && time() - $_SESSION['token_time'] < 3600) {
        return $_SESSION['token'];
    }
    $ch = curl_init("$host/api/auth/login");
    curl_setopt_array($ch, [
        CURLOPT_RETURNTRANSFER => true,
        CURLOPT_POST           => true,
        CURLOPT_POSTFIELDS     => json_encode(['username' => $username, 'password' => $password]),
        CURLOPT_HTTPHEADER     => ['Content-Type: application/json'],
        CURLOPT_SSL_VERIFYPEER => false,
        CURLOPT_SSL_VERIFYHOST => false,
        CURLOPT_TIMEOUT        => 10,
    ]);
    $response = curl_exec($ch);
    curl_close($ch);
    $data = json_decode($response, true);
    if (!isset($data['token'])) {
        return null;
    }
    $_SESSION['token']      = $data['token'];
    $_SESSION['token_time'] = time();
    return $data['token'];
}

$token = getTbToken($TB_HOST, $TB_USERNAME, $TB_PASSWORD);
if (!$token) {
    http_response_code(502);
    echo json_encode(['error' => 'Impossibile autenticarsi su ThingsBoard.']);
    exit;
}

// Chiama TB: POST /api/alarm/{id}/ack  oppure  POST /api/alarm/{id}/clear
$ch = curl_init("$TB_HOST/api/alarm/" . rawurlencode($alarmId) . "/$action");
curl_setopt_array($ch, [
    CURLOPT_RETURNTRANSFER => true,
    CURLOPT_POST           => true,
    CURLOPT_POSTFIELDS     => '',
    CURLOPT_HTTPHEADER     => [
        "X-Authorization: Bearer $token",
        "Content-Type: application/json",
    ],
    CURLOPT_SSL_VERIFYPEER => false,
    CURLOPT_SSL_VERIFYHOST => false,
    CURLOPT_TIMEOUT        => 10,
]);
$response = curl_exec($ch);
$httpCode = curl_getinfo($ch, CURLINFO_HTTP_CODE);
$curlErr  = curl_error($ch);
curl_close($ch);

if ($curlErr) {
    http_response_code(502);
    echo json_encode(['error' => "Errore cURL: $curlErr"]);
    exit;
}

if ($httpCode === 200 || $httpCode === 204) {
    echo json_encode(['success' => true, 'action' => $action, 'alarmId' => $alarmId]);
} else {
    if ($httpCode === 401) unset($_SESSION['token'], $_SESSION['token_time']);
    http_response_code(502);
    echo json_encode(['error' => "ThingsBoard ha risposto HTTP $httpCode", 'body' => $response]);
}
?>
