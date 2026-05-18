<?php
error_reporting(E_ALL);
ini_set('display_errors', 1);
$conn = mysqli_connect('192.168.60.144', 'francesco_bego', 'accaduti.immaginosa.', 'francesco_bego_alveare');
if (!$conn) die("Errore: " . mysqli_connect_error());
mysqli_set_charset($conn, 'utf8mb4');

$sql = "CREATE TABLE IF NOT EXISTS arnie_users (
    id            INT AUTO_INCREMENT PRIMARY KEY,
    nome          VARCHAR(100) UNIQUE NOT NULL,
    password_hash VARCHAR(255) NOT NULL,
    ruolo         ENUM('admin','operator','viewer') NOT NULL DEFAULT 'viewer'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;";

mysqli_query($conn, $sql) ? print("Tabella creata.") : print("Errore: " . mysqli_error($conn));

// Crea subito il primo admin
$nome_utente = 'prova';
$hash = password_hash('ApiApi1234!', PASSWORD_BCRYPT);
$stmt = mysqli_prepare($conn, "INSERT INTO arnie_users (nome, password_hash, ruolo) VALUES (?, ?, 'admin')");

if (!$stmt) {
    die("Errore prepare: " . mysqli_error($conn));
}

mysqli_stmt_bind_param($stmt, 'ss', $nome_utente, $hash);

if (mysqli_stmt_execute($stmt)) {
    print(" Admin creato.");
} else {
    print(" Errore insert: " . mysqli_stmt_error($stmt));
}