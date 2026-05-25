<?php
error_reporting(E_ALL);
ini_set('display_errors', 1);

mysqli_report(MYSQLI_REPORT_OFF);

// DB LOCALE (scuola - rete interna)
$conn = @mysqli_init();
if ($conn) {
    mysqli_options($conn, MYSQLI_OPT_CONNECT_TIMEOUT, 2);
    $connected = @mysqli_real_connect($conn, '192.168.60.144', 'francesco_bego', 'accaduti.immaginosa.', 'francesco_bego_alveare');
    if (!$connected) $conn = null;
}

// DB REMOTO (hosting scuola - sito online)
// $conn = @mysqli_init();
// if ($conn) {
//     mysqli_options($conn, MYSQLI_OPT_CONNECT_TIMEOUT, 2);
//     $connected = @mysqli_real_connect($conn, 'iisvio-sxtzwa62.db.tb-hosting.com', 'iisvio_sxtzwa62', 'PASSWORD', 'iisvio_sxtzwa62');
//     if (!$connected) $conn = null;
// }

if ($conn) {
    mysqli_set_charset($conn, 'utf8mb4');
    define('OFFLINE_MODE', false);
} else {
    $conn = null;
    define('OFFLINE_MODE', true);
}