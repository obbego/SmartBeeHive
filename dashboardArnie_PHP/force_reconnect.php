<?php
session_start();
unset($_SESSION['db_status'], $_SESSION['db_offline_since']);
$back = $_SERVER['HTTP_REFERER'] ?? 'html/index.php';
header("Location: $back");
exit;
?>