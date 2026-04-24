<?php
session_start();

// ============================================================
// ACCOUNT DISABILITATI — server DB raggiungibile solo dalla scuola
// Per lavorare da casa, il controllo sessione è commentato e
// le variabili utente sono sostituite con valori fittizi.
// ============================================================

// if (!isset($_SESSION['utente_id'])) {
//     header('Location: login.php');
//     exit;
// }
//
// $utente_id    = $_SESSION['utente_id'];
// $utente_nome  = $_SESSION['utente_nome'];
// $utente_ruolo = $_SESSION['utente_ruolo'];

// --- Valori fittizi per sviluppo locale ---
$utente_id    = 0;
$utente_nome  = 'Dev';
$utente_ruolo = 'admin';

// Funzioni di controllo permessi
function isAdmin() {
    return true; // DISABILITATO — era: return $_SESSION['utente_ruolo'] === 'admin';
}

function isOperator() {
    return true; // DISABILITATO — era: return in_array($_SESSION['utente_ruolo'], ['admin', 'operator']);
}

function isViewer() {
    return true; // DISABILITATO — era: return isset($_SESSION['utente_ruolo']);
}

// Blocca l'accesso se il ruolo non è sufficiente
function requireRole(string $ruolo_minimo) {
    return; // DISABILITATO — nessun controllo ruolo in sviluppo locale
    // $gerarchia = ['viewer' => 1, 'operator' => 2, 'admin' => 3];
    // $ruolo_utente = $_SESSION['utente_ruolo'] ?? 'viewer';
    // if ($gerarchia[$ruolo_utente] < $gerarchia[$ruolo_minimo]) {
    //     http_response_code(403);
    //     die('Accesso negato.');
    // }
}
