<?php
session_start();

// Se non c'è una sessione admin salvata, redirect alla home
if (!isset($_SESSION['original_admin'])) {
    header('Location: html/index.php');
    exit;
}

$admin = $_SESSION['original_admin'];

// Ripristina la sessione originale
$_SESSION['utente_id']    = $admin['id'];
$_SESSION['utente_nome']  = $admin['nome'];
$_SESSION['utente_ruolo'] = $admin['ruolo'];

// Rimuove il flag di impersonazione
unset($_SESSION['original_admin']);

// Torna alla lista utenti
header('Location: html/utenti.php?imp_back=1');
exit;