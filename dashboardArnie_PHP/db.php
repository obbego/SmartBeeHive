<?php
error_reporting(E_ALL);
ini_set('display_errors', 1);

// DB LOCALE (scuola - rete interna)
try {
    $conn = @mysqli_connect(
        '192.168.60.999',
        'francesco_bego',
        'accaduti.immaginosa.',
        'francesco_bego_alveare'
    );
} catch (Exception $e) {
    $conn = null;
}

// DB REMOTO (hosting scuola - sito online)
// try {
//     $conn = @mysqli_connect(
//         'iisvio-sxtzwa62.db.tb-hosting.com',
//         'iisvio_sxtzwa62',
//         'PASSWORD',
//         'iisvio_sxtzwa62'
//     );
// } catch (Exception $e) {
//     $conn = null;
// }

// Se la connessione fallisce (es. da casa) → modalità offline
if ($conn) {
    mysqli_set_charset($conn, 'utf8mb4');
    define('OFFLINE_MODE', false);
} else {
    $conn = null;
    define('OFFLINE_MODE', true);
}