<?php
// ============================================================
// SIGNUP DISABILITATO — server DB raggiungibile solo dalla scuola
// ============================================================
require_once '../auth.php';
// requireRole('admin');       // DISABILITATO — auth.php già bypassa tutto
// require_once '../db.php';   // DISABILITATO — DB non raggiungibile da casa

$errore = '';
$successo = '';

if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    // --- CODICE ORIGINALE COMMENTATO (richiede DB) ---
    //
    // $nome  = trim($_POST['nome'] ?? '');
    // $pass  = $_POST['password'] ?? '';
    // $ruolo = $_POST['ruolo'] ?? 'viewer';
    //
    // if (empty($nome) || empty($pass)) {
    //     $errore = 'Username e password sono obbligatori.';
    // } elseif (strlen($pass) < 8) {
    //     $errore = 'La password deve essere di almeno 8 caratteri.';
    // } elseif (!in_array($ruolo, ['admin', 'operator', 'viewer'])) {
    //     $errore = 'Ruolo non valido.';
    // } else {
    //     // Controlla se username esiste già
    //     $check = mysqli_prepare($conn, "SELECT id FROM arnie_users WHERE nome = ? LIMIT 1");
    //     mysqli_stmt_bind_param($check, 's', $nome);
    //     mysqli_stmt_execute($check);
    //     mysqli_stmt_store_result($check);
    //
    //     if (mysqli_stmt_num_rows($check) > 0) {
    //         $errore = "Username '$nome' già in uso.";
    //     } else {
    //         $hash = password_hash($pass, PASSWORD_BCRYPT);
    //         $stmt = mysqli_prepare($conn,
    //                 "INSERT INTO arnie_users (nome, password_hash, ruolo) VALUES (?, ?, ?)"
    //         );
    //         mysqli_stmt_bind_param($stmt, 'sss', $nome, $hash, $ruolo);
    //         if (mysqli_stmt_execute($stmt)) {
    //             $successo = "Account '$nome' creato con ruolo '$ruolo'.";
    //         } else {
    //             $errore = 'Errore durante la creazione: ' . mysqli_stmt_error($stmt);
    //         }
    //         mysqli_stmt_close($stmt);
    //     }
    //     mysqli_stmt_close($check);
    // }
    // mysqli_close($conn);

    $errore = 'Funzione non disponibile: database non raggiungibile in sviluppo locale.';
}
?>
<!DOCTYPE html>
<html lang="it" data-bs-theme="dark">
<head>
	<meta charset="UTF-8">
	<meta name="viewport" content="width=device-width, initial-scale=1.0">
	<title>Crea Account</title>
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
			<a href="index.php" class="text-white me-3" style="opacity: 0.8;">
				<i data-lucide="arrow-left"></i>
			</a>
			<i data-lucide="layout-grid" class="brand-icon me-2"></i>
			FrontEnd Managment Arnie
		</h1>
		<div style="font-size: 13px; color: var(--text-muted);">
			Loggato come <strong class="text-white"><?= htmlspecialchars($utente_nome) ?></strong>
			&nbsp;·&nbsp;
			<a href="../logout.php" style="color: var(--text-muted); text-decoration: underline;">Esci</a>
		</div>
	</div>
</header>

<div class="container login-wrapper">
	<div class="glass-panel stat-card login-card p-4">
		<div class="text-center mb-4">
			<i data-lucide="user-plus" style="width:40px;height:40px;"></i>
			<h3 class="mt-2">Crea Nuovo Account</h3>
			<p style="color:var(--text-muted);font-size:14px;">
				Solo gli amministratori possono creare account
			</p>
		</div>

		<?php if ($errore): ?>
			<div class="mb-3 p-3 text-center"
				 style="background: rgba(239,68,68,0.1); border: 1px solid rgba(239,68,68,0.3);
                    border-radius: 10px; color: var(--danger); font-size: 14px;">
				<?= htmlspecialchars($errore) ?>
			</div>
		<?php endif; ?>

		<?php if ($successo): ?>
			<div class="mb-3 p-3 text-center"
				 style="background: rgba(16,185,129,0.1); border: 1px solid rgba(16,185,129,0.3);
                    border-radius: 10px; color: var(--success); font-size: 14px;">
				✓ <?= htmlspecialchars($successo) ?>
			</div>
		<?php endif; ?>

		<form method="POST">
			<div class="mb-3">
				<label class="form-label">Username</label>
				<input type="text" name="nome" class="form-control"
					   placeholder="Username del nuovo utente"
					   value="<?= htmlspecialchars($_POST['nome'] ?? '') ?>"
					   required autocomplete="off">
			</div>

			<div class="mb-3">
				<label class="form-label">Password (min. 8 caratteri)</label>
				<input type="password" name="password" class="form-control"
					   placeholder="Password" required autocomplete="new-password">
			</div>

			<div class="mb-4">
				<label class="form-label">Ruolo</label>
				<select name="ruolo" class="form-control">
					<option value="viewer"   <?= (($_POST['ruolo'] ?? '') === 'viewer')   ? 'selected' : '' ?>>Viewer</option>
					<option value="operator" <?= (($_POST['ruolo'] ?? '') === 'operator') ? 'selected' : '' ?>>Operator</option>
					<option value="admin"    <?= (($_POST['ruolo'] ?? '') === 'admin')    ? 'selected' : '' ?>>Admin</option>
				</select>
			</div>

			<div class="d-grid">
				<button type="submit" class="btn btn-honey fw-semibold">
					Crea Account
				</button>
			</div>
		</form>
	</div>
</div>

<script>lucide.createIcons();</script>
<script src="https://cdn.jsdelivr.net/npm/bootstrap@5.3.2/dist/js/bootstrap.bundle.min.js"></script>
</body>
</html>
