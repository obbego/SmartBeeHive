<?php
// ============================================================
// LOGOUT DISABILITATO — account non attivi in sviluppo locale
// Redirect diretto alla dashboard invece di distruggere la sessione
// ============================================================

// session_start();
// session_destroy();
// header('Location: html/login.php');
// exit;

header('Location: html/index.php');
exit;
