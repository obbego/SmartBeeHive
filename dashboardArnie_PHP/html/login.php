<?php
session_start();

// Se già loggato, vai alla dashboard
if (isset($_SESSION['utente_id'])) {
    header('Location: index.php');
    exit;
}

require_once '../db.php';

$errore = '';

if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    $nome = trim($_POST['nome'] ?? '');
    $pass = $_POST['password'] ?? '';

    if (empty($nome) || empty($pass)) {
        $errore = 'Inserisci username e password.';
    } else {
        $stmt = mysqli_prepare($conn,
            "SELECT id, nome, password_hash, ruolo FROM arnie_users WHERE nome = ? LIMIT 1"
        );
        mysqli_stmt_bind_param($stmt, 's', $nome);
        mysqli_stmt_execute($stmt);
        $result = mysqli_stmt_get_result($stmt);
        $utente = mysqli_fetch_assoc($result);
        mysqli_stmt_close($stmt);

        if ($utente && password_verify($pass, $utente['password_hash'])) {
            // Login ok — imposta sessione
            $_SESSION['utente_id']   = $utente['id'];
            $_SESSION['utente_nome'] = $utente['nome'];
            $_SESSION['utente_ruolo'] = $utente['ruolo'];

            header('Location: index.php');
            exit;
        } else {
            $errore = 'Username o password errati.';
        }
    }

    mysqli_close($conn);
}
?>
<!DOCTYPE html>
<html lang="it">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Login</title>
    <link rel="preconnect" href="https://fonts.googleapis.com">
    <link rel="preconnect" href="https://fonts.gstatic.com" crossorigin>
    <link href="https://fonts.googleapis.com/css2?family=Inter:wght@400;500;600;700&display=swap" rel="stylesheet">
    <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.2/dist/css/bootstrap.min.css" rel="stylesheet">
    <script src="https://unpkg.com/lucide@latest"></script>
    <link rel="stylesheet" href="../css/style.css">
    <link rel="stylesheet" href="../css/login_signup.css">
</head>
<body>

<header class="mb-4 py-3 border-bottom border-secondary border-opacity-25"
        style="background: rgba(15, 23, 42, 0.3);">
    <div class="container d-flex justify-content-between align-items-center">
        <h1>
            <i data-lucide="layout-grid" class="brand-icon me-2"></i>
            FrontEnd Managment Arnie
        </h1>
    </div>
</header>

<div class="container login-wrapper">
    <div class="glass-panel stat-card login-card p-4">
        <div class="text-center mb-4">
            <i data-lucide="shield" style="width:40px;height:40px;"></i>
            <h3 class="mt-2">Accesso Dashboard</h3>
            <p style="color:var(--text-muted);font-size:14px;">
                Inserisci le tue credenziali
            </p>
        </div>

        <?php if ($errore): ?>
        <div class="mb-3 p-3 text-center"
             style="background: rgba(239,68,68,0.1); border: 1px solid rgba(239,68,68,0.3); border-radius: 10px; color: var(--danger); font-size: 14px;">
            <?= htmlspecialchars($errore) ?>
        </div>
        <?php endif; ?>

        <form method="POST">
            <div class="mb-3">
                <label class="form-label">Username</label>
                <input type="text" name="nome" class="form-control"
                       placeholder="Il tuo username"
                       value="<?= htmlspecialchars($_POST['nome'] ?? '') ?>"
                       required autocomplete="username">
            </div>

            <div class="mb-4">
                <label class="form-label">Password</label>
                <input type="password" name="password" class="form-control"
                       placeholder="Password" required autocomplete="current-password">
            </div>

            <div class="d-grid">
                <button type="submit" class="btn btn-honey fw-semibold">
                    Accedi
                </button>
            </div>

            <br>
            <div class="text-center mb-4">
                <p style="color:var(--text-muted);font-size:14px;">
                    Non hai un account?
                    <a href="signup.php" style="color:var(--text-muted); text-decoration:underline;">
                        Registrati
                    </a>
                </p>
            </div>
        </form>
    </div>
</div>

<script>lucide.createIcons();</script>
<script src="https://cdn.jsdelivr.net/npm/bootstrap@5.3.2/dist/js/bootstrap.bundle.min.js"></script>
</body>
</html>