<?php
session_start();
require_once 'auth.php';   // verifica login e popola $utente_id, $utente_nome, $utente_ruolo
require_once 'db.php';

// Solo gli admin possono usare questa funzione
requireRole('admin');

// Blocca in modalità offline (non c'è il DB per verificare l'utente)
if (OFFLINE_MODE) {
    header('Location: html/utenti.php?imp_error=offline');
    exit;
}

// Impedisce di immedesimarsi in sé stessi
$target_id = (int)($_POST['user_id'] ?? 0);
if ($target_id <= 0 || $target_id === (int)$utente_id) {
    header('Location: html/utenti.php?imp_error=invalid');
    exit;
}

// Recupera i dati dell'utente target dal DB
$stmt = mysqli_prepare($conn, "SELECT id, nome, ruolo FROM arnie_users WHERE id = ? LIMIT 1");
mysqli_stmt_bind_param($stmt, 'i', $target_id);
mysqli_stmt_execute($stmt);
$result = mysqli_stmt_get_result($stmt);
$target  = mysqli_fetch_assoc($result);
mysqli_stmt_close($stmt);
mysqli_close($conn);

if (!$target) {
    header('Location: html/utenti.php?imp_error=notfound');
    exit;
}

// Salva la sessione admin originale (solo se non siamo già in impersonazione)
if (!isset($_SESSION['original_admin'])) {
    $_SESSION['original_admin'] = [
        'id'    => $_SESSION['utente_id'],
        'nome'  => $_SESSION['utente_nome'],
        'ruolo' => $_SESSION['utente_ruolo'],
    ];
}

// Sovrascrive la sessione con i dati dell'utente target
$_SESSION['utente_id']    = (int)$target['id'];
$_SESSION['utente_nome']  = $target['nome'];
$_SESSION['utente_ruolo'] = $target['ruolo'];

// Redirect alla dashboard come se fosse quell'utente
header('Location: html/index.php');
exit;