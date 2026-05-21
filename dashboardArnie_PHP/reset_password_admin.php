<?php
// Endpoint AJAX dedicato al reset password da admin
header('Content-Type: application/json');
session_start();
require_once 'auth.php';
require_once 'db.php';

// Solo admin
if ($utente_ruolo !== 'admin') {
    http_response_code(403);
    echo json_encode(['ok' => false, 'error' => 'Accesso negato.']);
    exit;
}

if (OFFLINE_MODE) {
    http_response_code(503);
    echo json_encode(['ok' => false, 'error' => 'Non disponibile in modalità offline.']);
    exit;
}

$input     = json_decode(file_get_contents('php://input'), true) ?: [];
$target_id = (int)($input['user_id']          ?? 0);
$pass_new  = $input['nuova_password']          ?? '';
$pass_conf = $input['conferma_password']       ?? '';

// Validazioni server-side
if ($target_id === $utente_id) {
    echo json_encode(['ok' => false, 'error' => 'Per la tua password usa la pagina Profilo.']);
    exit;
}
if ($target_id <= 0) {
    echo json_encode(['ok' => false, 'error' => 'Utente non valido.']);
    exit;
}
if (strlen($pass_new) < 8) {
    echo json_encode(['ok' => false, 'error' => 'La password deve essere di almeno 8 caratteri.']);
    exit;
}
if ($pass_new !== $pass_conf) {
    echo json_encode(['ok' => false, 'error' => 'Le due password non coincidono.']);
    exit;
}

// Verifica che l'utente target esista
$check = mysqli_prepare($conn, "SELECT id FROM arnie_users WHERE id = ? LIMIT 1");
mysqli_stmt_bind_param($check, 'i', $target_id);
mysqli_stmt_execute($check);
mysqli_stmt_store_result($check);
if (mysqli_stmt_num_rows($check) === 0) {
    mysqli_stmt_close($check);
    echo json_encode(['ok' => false, 'error' => 'Utente non trovato.']);
    exit;
}
mysqli_stmt_close($check);

// Aggiorna
$hash = password_hash($pass_new, PASSWORD_BCRYPT);
$stmt = mysqli_prepare($conn, "UPDATE arnie_users SET password_hash = ? WHERE id = ?");
mysqli_stmt_bind_param($stmt, 'si', $hash, $target_id);

if (mysqli_stmt_execute($stmt)) {
    mysqli_stmt_close($stmt);
    echo json_encode(['ok' => true]);
} else {
    $err = mysqli_stmt_error($stmt);
    mysqli_stmt_close($stmt);
    echo json_encode(['ok' => false, 'error' => 'Errore DB: ' . $err]);
}
