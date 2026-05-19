<?php
require_once 'db.php';

// Cerca l'utente 'prova'
$result = mysqli_query($conn, "SELECT id, nome, password_hash, ruolo FROM arnie_users");
$utenti = mysqli_fetch_all($result, MYSQLI_ASSOC);

echo "<pre>";
echo "Utenti nel DB: " . count($utenti) . "\n\n";
foreach ($utenti as $u) {
    echo "ID: " . $u['id'] . "\n";
    echo "Nome: " . $u['nome'] . "\n";
    echo "Ruolo: " . $u['ruolo'] . "\n";
    echo "Hash: " . $u['password_hash'] . "\n";
    echo "Verifica 'ApiApi1234!': " . (password_verify('ApiApi1234!', $u['password_hash']) ? 'OK' : 'FALLITA') . "\n\n";
}
echo "</pre>";
