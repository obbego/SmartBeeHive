<?php
session_start();

if (!isset($_SESSION['utente_id'])) {
    header('Location: login.php');
    exit;
}

// Rendi disponibili i dati utente come variabili globali
$utente_id    = $_SESSION['utente_id'];
$utente_nome  = $_SESSION['utente_nome'];
$utente_ruolo = $_SESSION['utente_ruolo'];

// Funzioni di controllo permessi
function isAdmin() {
    return $_SESSION['utente_ruolo'] === 'admin';
}

function isOperator() {
    return in_array($_SESSION['utente_ruolo'], ['admin', 'operator']);
}

function isViewer() {
    return isset($_SESSION['utente_ruolo']);
}

// Blocca l'accesso se il ruolo non è sufficiente
function requireRole(string $ruolo_minimo) {
    $gerarchia = ['viewer' => 1, 'operator' => 2, 'admin' => 3];
    $ruolo_utente = $_SESSION['utente_ruolo'] ?? 'viewer';

    if ($gerarchia[$ruolo_utente] < $gerarchia[$ruolo_minimo]) {
        http_response_code(403);
        die('Accesso negato.');
    }
}