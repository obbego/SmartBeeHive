<?php
// Connessione al database — includi questo file dove serve
$conn = mysqli_connect(
    'iisvio-sxtzwa62.db.tb-hosting.com',
    'iisvio_sxtzwa62',
    'iX*Yyg68',
    'iisvio_sxtzwa62'
);

if (!$conn) {
    http_response_code(500);
    die(json_encode(['errore' => 'Connessione al database fallita.']));
}

mysqli_set_charset($conn, 'utf8mb4');