<?php
$conn = mysqli_connect(
    '192.168.60.144',
    'francesco_bego',
    'accaduti.immaginosa.',
    'francesco_bego_alveare'
);

if (!$conn) {
    http_response_code(500);
    die(json_encode(['errore' => 'Connessione al database fallita.']));
}

mysqli_set_charset($conn, 'utf8mb4');
