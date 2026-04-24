<?php
// ============================================================
// DATABASE DISABILITATO — server raggiungibile solo dalla scuola
// Tutte le query al DB (login, signup) non funzioneranno finché
// questo blocco è commentato.
// ============================================================

// $conn = mysqli_connect(
//     'iisvio-sxtzwa62.db.tb-hosting.com',
//     'iisvio_sxtzwa62',
//     'iX*Yyg68',
//     'iisvio_sxtzwa62'
// );
//
// if (!$conn) {
//     http_response_code(500);
//     die(json_encode(['errore' => 'Connessione al database fallita.']));
// }
//
// mysqli_set_charset($conn, 'utf8mb4');

$conn = null; // Connessione disabilitata in modalità sviluppo locale
