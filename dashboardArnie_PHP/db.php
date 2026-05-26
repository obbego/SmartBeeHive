<?php
error_reporting(E_ALL);
ini_set('display_errors', 1);
mysqli_report(MYSQLI_REPORT_OFF);

if (session_status() === PHP_SESSION_NONE) session_start();

$RETRY_INTERVAL = 600; // 10 minuti

$conn = null;
$shouldTry = true;

if (isset($_SESSION['db_status']) && $_SESSION['db_status'] === 'offline') {
    if ((time() - $_SESSION['db_offline_since']) < $RETRY_INTERVAL) {
        $shouldTry = false;
    }
}

if ($shouldTry) {
    $conn = @mysqli_init();
    if ($conn) {
        mysqli_options($conn, MYSQLI_OPT_CONNECT_TIMEOUT, 2);

        // DB LOCALE (scuola - rete interna)
        $connected = @mysqli_real_connect($conn, '192.168.60.144', 'francesco_bego', 'accaduti.immaginosa.', 'francesco_bego_alveare');

        // DB REMOTO (hosting scuola - sito online)
        // $connected = @mysqli_real_connect($conn, 'iisvio-sxtzwa62.db.tb-hosting.com', 'iisvio_sxtzwa62', 'PASSWORD', 'iisvio_sxtzwa62');

        if (!$connected) $conn = null;
    }

    if ($conn) {
        mysqli_set_charset($conn, 'utf8mb4');
        if (isset($_SESSION['db_status']) && $_SESSION['db_status'] === 'offline') {
            $_SESSION['db_just_reconnected'] = true;
        }
        unset($_SESSION['db_status'], $_SESSION['db_offline_since']);
        define('OFFLINE_MODE', false);
    } else {
        $_SESSION['db_status'] = 'offline';
        $_SESSION['db_offline_since'] = time();
        define('OFFLINE_MODE', true);
    }
} else {
    define('OFFLINE_MODE', true);
}
?>