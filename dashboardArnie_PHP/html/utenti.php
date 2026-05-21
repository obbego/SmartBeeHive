<?php
error_reporting(E_ALL);
ini_set('display_errors', 1);
require_once '../auth.php';
require_once '../db.php';
requireRole('admin');

if (OFFLINE_MODE) {
	?>
	<!DOCTYPE html>
	<html lang="it" data-bs-theme="dark">
	<head>
		<meta charset="UTF-8">
		<meta name="viewport" content="width=device-width, initial-scale=1.0">
		<title>Gestione Utenti - Smart Hive</title>
		<link href="https://fonts.googleapis.com/css2?family=Inter:wght@400;500;600;700&display=swap" rel="stylesheet">
		<link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.2/dist/css/bootstrap.min.css" rel="stylesheet">
		<script src="https://unpkg.com/lucide@latest"></script>
		<link rel="stylesheet" href="../css/style.css">
		<link rel="stylesheet" href="../css/navbar.css">
	</head>
	<body>
	<?php require_once '../includes/navbar.php'; ?>
	<div class="container pt-5 text-center" style="color:var(--warning); font-size:16px;">
		⚠ Gestione utenti non disponibile in modalità offline.
	</div>
	<script>lucide.createIcons();</script>
	<script src="https://cdn.jsdelivr.net/npm/bootstrap@5.3.2/dist/js/bootstrap.bundle.min.js"></script>
	<script src="../js/dati.js"></script>
	<script src="../js/navbar.js"></script>
	</body>
	</html>
	<?php
	exit;
}

$errore   = '';
$successo = '';

if ($_SERVER['REQUEST_METHOD'] === 'POST') {
	$azione = $_POST['azione'] ?? '';

	// ── CREA UTENTE ────────────────────────────────────────────────
	if ($azione === 'crea') {
		$nome  = trim($_POST['nome'] ?? '');
		$pass  = $_POST['password'] ?? '';
		$ruolo = $_POST['ruolo'] ?? 'viewer';

		if (empty($nome) || empty($pass)) {
			$errore = 'Username e password sono obbligatori.';
		} elseif (strlen($pass) < 8) {
			$errore = 'La password deve essere di almeno 8 caratteri.';
		} elseif (!in_array($ruolo, ['admin', 'operator', 'viewer'])) {
			$errore = 'Ruolo non valido.';
		} else {
			$check = mysqli_prepare($conn, "SELECT id FROM arnie_users WHERE nome = ? LIMIT 1");
			mysqli_stmt_bind_param($check, 's', $nome);
			mysqli_stmt_execute($check);
			mysqli_stmt_store_result($check);

			if (mysqli_stmt_num_rows($check) > 0) {
				$errore = "Username '$nome' già in uso.";
			} else {
				$hash = password_hash($pass, PASSWORD_BCRYPT);
				$stmt = mysqli_prepare($conn,
						"INSERT INTO arnie_users (nome, password_hash, ruolo) VALUES (?, ?, ?)"
				);
				mysqli_stmt_bind_param($stmt, 'sss', $nome, $hash, $ruolo);
				mysqli_stmt_execute($stmt)
						? $successo = "Account '$nome' creato con ruolo '$ruolo'."
						: $errore   = 'Errore: ' . mysqli_stmt_error($stmt);
				mysqli_stmt_close($stmt);
			}
			mysqli_stmt_close($check);
		}
	}

	// ── CAMBIA RUOLO ───────────────────────────────────────────────
	if ($azione === 'cambia_ruolo') {
		$id    = (int)($_POST['user_id'] ?? 0);
		$ruolo = $_POST['ruolo'] ?? '';

		if ($id === $utente_id) {
			$errore = 'Non puoi cambiare il tuo stesso ruolo.';
		} elseif (!in_array($ruolo, ['admin', 'operator', 'viewer'])) {
			$errore = 'Ruolo non valido.';
		} else {
			$stmt = mysqli_prepare($conn, "UPDATE arnie_users SET ruolo = ? WHERE id = ?");
			mysqli_stmt_bind_param($stmt, 'si', $ruolo, $id);
			mysqli_stmt_execute($stmt)
					? $successo = 'Ruolo aggiornato.'
					: $errore   = 'Errore aggiornamento ruolo.';
			mysqli_stmt_close($stmt);
		}
	}

	// ── ELIMINA ────────────────────────────────────────────────────
	if ($azione === 'elimina') {
		$id = (int)($_POST['user_id'] ?? 0);

		if ($id === $utente_id) {
			$errore = 'Non puoi eliminare il tuo stesso account.';
		} else {
			$stmt = mysqli_prepare($conn, "DELETE FROM arnie_users WHERE id = ?");
			mysqli_stmt_bind_param($stmt, 'i', $id);
			mysqli_stmt_execute($stmt)
					? $successo = 'Utente eliminato.'
					: $errore   = 'Errore eliminazione.';
			mysqli_stmt_close($stmt);
		}
	}

	// ── RESET PASSWORD (da admin) ──────────────────────────────────
	if ($azione === 'reset_password') {
		$id        = (int)($_POST['user_id']          ?? 0);
		$pass_new  = $_POST['nuova_password']          ?? '';
		$pass_conf = $_POST['conferma_password']       ?? '';

		if ($id === $utente_id) {
			$errore = 'Per cambiare la tua password usa la pagina Profilo.';
		} elseif (strlen($pass_new) < 8) {
			$errore = 'La password deve essere di almeno 8 caratteri.';
		} elseif ($pass_new !== $pass_conf) {
			$errore = 'Le due password non coincidono.';
		} else {
			$hash = password_hash($pass_new, PASSWORD_BCRYPT);
			$stmt = mysqli_prepare($conn, "UPDATE arnie_users SET password_hash = ? WHERE id = ?");
			mysqli_stmt_bind_param($stmt, 'si', $hash, $id);
			mysqli_stmt_execute($stmt)
					? $successo = 'Password reimpostata con successo.'
					: $errore   = 'Errore aggiornamento password.';
			mysqli_stmt_close($stmt);
		}
	}
}

$utenti = [];
$result = mysqli_query($conn, "SELECT id, nome, ruolo, created_at FROM arnie_users ORDER BY ruolo, nome");
while ($row = mysqli_fetch_assoc($result)) {
	$utenti[] = $row;
}
mysqli_close($conn);
?>
<!DOCTYPE html>
<html lang="it" data-bs-theme="dark">
<head>
	<meta charset="UTF-8">
	<meta name="viewport" content="width=device-width, initial-scale=1.0">
	<title>Gestione Utenti - Smart Hive</title>
	<link rel="preconnect" href="https://fonts.googleapis.com">
	<link href="https://fonts.googleapis.com/css2?family=Inter:wght@400;500;600;700&display=swap" rel="stylesheet">
	<link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.2/dist/css/bootstrap.min.css" rel="stylesheet">
	<script src="https://unpkg.com/lucide@latest"></script>
	<link rel="stylesheet" href="../css/style.css">
	<link rel="stylesheet" href="../css/login_signup.css">
	<link rel="stylesheet" href="../css/navbar.css">
	<style>
        h1,h2,h3,h4,h5,h6,p { margin-bottom: 0; }
        a { text-decoration: none; }

        .user-row { display:flex; align-items:center; justify-content:space-between; gap:12px; padding:14px 16px; border-bottom:1px solid var(--glass-border); flex-wrap:wrap; }
        .user-row:last-child { border-bottom:none; }
        .user-info { display:flex; align-items:center; gap:12px; flex:1; min-width:0; }
        .user-avatar { width:36px; height:36px; border-radius:50%; background:rgba(251,191,36,0.15); border:1px solid rgba(251,191,36,0.3); display:flex; align-items:center; justify-content:center; color:var(--honey-glow); font-size:15px; font-weight:700; flex-shrink:0; }
        .user-avatar.me { background:rgba(96,165,250,0.15); border-color:rgba(96,165,250,0.3); color:#60a5fa; }
        .user-name { font-weight:600; font-size:14px; color:var(--text-main); }
        .user-meta { font-size:11px; color:var(--text-muted); margin-top:2px; }
        .user-actions { display:flex; align-items:center; gap:8px; flex-shrink:0; }

        .role-badge { font-size:11px; font-weight:600; padding:3px 10px; border-radius:20px; text-transform:uppercase; letter-spacing:0.04em; }
        .role-badge.admin    { background:rgba(239,68,68,0.15);   color:var(--danger);    border:1px solid rgba(239,68,68,0.3); }
        .role-badge.operator { background:rgba(245,158,11,0.15);  color:var(--warning);   border:1px solid rgba(245,158,11,0.3); }
        .role-badge.viewer   { background:rgba(148,163,184,0.15); color:var(--text-muted);border:1px solid rgba(148,163,184,0.2); }

        .btn-icon { background:transparent; border:1px solid var(--glass-border); color:var(--text-muted); border-radius:8px; width:34px; height:34px; display:flex; align-items:center; justify-content:center; cursor:pointer; transition:all 0.2s; }
        .btn-icon:hover { background:rgba(255,255,255,0.08); color:var(--text-main); }
        .btn-icon.danger:hover { background:rgba(239,68,68,0.15); border-color:rgba(239,68,68,0.4); color:var(--danger); }
        .btn-icon.key:hover    { background:rgba(251,191,36,0.15); border-color:rgba(251,191,36,0.4); color:var(--honey-glow); }

        .role-select { background:rgba(30,41,59,0.8); border:1px solid var(--glass-border); color:var(--text-main); border-radius:8px; padding:4px 8px; font-size:13px; font-family:inherit; cursor:pointer; }
        .role-select:focus { outline:none; border-color:var(--honey-glow); }

        /* Modale reset password */
        .reset-modal-bg {
            display:none; position:fixed; inset:0; z-index:9999;
            background:rgba(0,0,0,0.65); backdrop-filter:blur(4px);
            align-items:center; justify-content:center;
        }
        .reset-modal-bg.show { display:flex; }
        .reset-modal {
            background:#1e293b; border:1px solid rgba(255,255,255,0.12);
            border-radius:20px; padding:28px; max-width:400px; width:90%;
            box-shadow:0 24px 60px rgba(0,0,0,0.6);
            animation: modalIn 0.22s cubic-bezier(0.34,1.56,0.64,1);
        }
        @keyframes modalIn {
            from { opacity:0; transform:scale(0.9) translateY(16px); }
            to   { opacity:1; transform:scale(1) translateY(0); }
        }
        .form-control {
            background:rgba(0,0,0,0.25); border:1px solid var(--glass-border);
            color:var(--text-main); padding:10px 14px; border-radius:10px;
            font-family:inherit; font-size:14px;
        }
        .form-control:focus {
            border-color:var(--honey-primary); background:rgba(0,0,0,0.35);
            outline:none; box-shadow:0 0 0 3px rgba(245,158,11,0.15); color:var(--text-main);
        }
        .form-control::placeholder { color:rgba(255,255,255,0.3); }
	</style>
</head>
<body>

<?php require_once '../includes/navbar.php'; ?>

<div class="container pt-4 pb-5">
	<div class="row justify-content-center g-4">

		<!-- LISTA UTENTI -->
		<div class="col-12 col-lg-7">
			<div class="glass-panel p-0">
				<div class="p-4 border-bottom border-white border-opacity-10 d-flex align-items-center justify-content-between">
					<div class="chart-title mb-0">Utenti registrati</div>
					<span style="font-size:13px; color:var(--text-muted);"><?= count($utenti) ?> account</span>
				</div>

				<?php if ($errore): ?>
					<div class="mx-4 mt-3 p-3" style="background:rgba(239,68,68,0.1); border:1px solid rgba(239,68,68,0.3); border-radius:10px; color:var(--danger); font-size:14px;">
						<?= htmlspecialchars($errore) ?>
					</div>
				<?php endif; ?>

				<?php if ($successo): ?>
					<div class="mx-4 mt-3 p-3" style="background:rgba(16,185,129,0.1); border:1px solid rgba(16,185,129,0.3); border-radius:10px; color:var(--success); font-size:14px;">
						✓ <?= htmlspecialchars($successo) ?>
					</div>
				<?php endif; ?>

				<div class="p-2">
					<?php foreach ($utenti as $u):
						$isMe     = $u['id'] === $utente_id;
						$inizialeU = strtoupper(substr($u['nome'], 0, 1));
						// Formatta data creazione
						$dataCreazione = '–';
						if (!empty($u['created_at'])) {
							$ts = strtotime($u['created_at']);
							$dataCreazione = date('d/m/Y', $ts);
						}
						?>
						<div class="user-row">
							<div class="user-info">
								<div class="user-avatar <?= $isMe ? 'me' : '' ?>"><?= $inizialeU ?></div>
								<div>
									<div class="user-name">
										<?= htmlspecialchars($u['nome']) ?>
										<?php if ($isMe): ?>
											<span style="font-size:11px; color:var(--text-muted); font-weight:400;">(tu)</span>
										<?php endif; ?>
									</div>
									<div class="user-meta">Iscritto il <?= $dataCreazione ?></div>
								</div>
							</div>
							<div class="user-actions">
								<?php if ($isMe): ?>
									<span class="role-badge <?= $u['ruolo'] ?>"><?= $u['ruolo'] ?></span>
								<?php else: ?>
									<!-- Cambio ruolo -->
									<form method="POST" style="margin:0;">
										<input type="hidden" name="azione" value="cambia_ruolo">
										<input type="hidden" name="user_id" value="<?= $u['id'] ?>">
										<select name="ruolo" class="role-select" onchange="this.form.submit()">
											<option value="viewer"   <?= $u['ruolo']==='viewer'   ? 'selected':'' ?>>Viewer</option>
											<option value="operator" <?= $u['ruolo']==='operator' ? 'selected':'' ?>>Operator</option>
											<option value="admin"    <?= $u['ruolo']==='admin'    ? 'selected':'' ?>>Admin</option>
										</select>
									</form>
									<!-- Reset password -->
									<button type="button" class="btn-icon key"
											title="Reimposta password"
											onclick="openResetModal(<?= $u['id'] ?>, '<?= htmlspecialchars($u['nome'], ENT_QUOTES) ?>')">
										<i data-lucide="key-round" style="width:15px;height:15px;"></i>
									</button>
									<!-- Elimina -->
									<form method="POST" style="margin:0;"
										  onsubmit="return confirm('Eliminare l\'utente <?= htmlspecialchars($u['nome'], ENT_QUOTES) ?>?')">
										<input type="hidden" name="azione" value="elimina">
										<input type="hidden" name="user_id" value="<?= $u['id'] ?>">
										<button type="submit" class="btn-icon danger" title="Elimina utente">
											<i data-lucide="trash-2" style="width:15px;height:15px;"></i>
										</button>
									</form>
								<?php endif; ?>
							</div>
						</div>
					<?php endforeach; ?>
				</div>
			</div>
		</div>

		<!-- CREA NUOVO ACCOUNT -->
		<div class="col-12 col-lg-4">
			<div class="glass-panel p-4">
				<div class="chart-title mb-4">
					<i data-lucide="user-plus" style="width:18px;height:18px; vertical-align:middle; margin-right:8px;"></i>
					Nuovo Account
				</div>
				<form method="POST">
					<input type="hidden" name="azione" value="crea">
					<div class="mb-3">
						<label class="form-label">Username</label>
						<input type="text" name="nome" class="form-control"
							   placeholder="es. mario.rossi"
							   value="<?= htmlspecialchars($_POST['nome'] ?? '') ?>"
							   required autocomplete="off">
					</div>
					<div class="mb-3">
						<label class="form-label">Password <span style="font-size:11px; color:var(--text-muted);">(min. 8 caratteri)</span></label>
						<input type="password" name="password" class="form-control"
							   placeholder="Password" required autocomplete="new-password">
					</div>
					<div class="mb-4">
						<label class="form-label">Ruolo</label>
						<select name="ruolo" class="form-control">
							<option value="viewer"   <?= (($_POST['ruolo'] ?? '') === 'viewer')   ? 'selected':'' ?>>Viewer — solo lettura</option>
							<option value="operator" <?= (($_POST['ruolo'] ?? '') === 'operator') ? 'selected':'' ?>>Operator — gestione allarmi</option>
							<option value="admin"    <?= (($_POST['ruolo'] ?? '') === 'admin')    ? 'selected':'' ?>>Admin — accesso completo</option>
						</select>
					</div>
					<div class="d-grid">
						<button type="submit" class="btn btn-honey fw-semibold">Crea Account</button>
					</div>
				</form>
			</div>
		</div>

	</div>
</div>

<!-- ── MODALE RESET PASSWORD ──────────────────────────────────────── -->
<div class="reset-modal-bg" id="resetModal" onclick="bgClick(event)">
	<div class="reset-modal">
		<div class="d-flex justify-content-between align-items-start mb-4">
			<div>
				<div style="font-size:16px;font-weight:700;color:white;">Reimposta Password</div>
				<div id="resetModalMeta" style="font-size:13px;color:var(--text-muted);margin-top:3px;"></div>
			</div>
			<button onclick="closeResetModal()"
					style="background:none;border:none;color:var(--text-muted);cursor:pointer;padding:4px;">
				<i data-lucide="x" style="width:18px;height:18px;"></i>
			</button>
		</div>

		<form method="POST" id="resetForm">
			<input type="hidden" name="azione" value="reset_password">
			<input type="hidden" name="user_id" id="resetUserId" value="">

			<div class="mb-3">
				<label class="form-label" style="font-size:13px;color:var(--text-muted);">Nuova password</label>
				<input type="password" name="nuova_password" class="form-control"
					   placeholder="Minimo 8 caratteri" required autocomplete="new-password">
			</div>
			<div class="mb-4">
				<label class="form-label" style="font-size:13px;color:var(--text-muted);">Conferma password</label>
				<input type="password" name="conferma_password" class="form-control"
					   placeholder="Ripeti la nuova password" required autocomplete="new-password">
			</div>
			<div class="d-flex gap-2">
				<button type="button" onclick="closeResetModal()"
						style="flex:1;padding:10px;border-radius:10px;border:1px solid rgba(255,255,255,0.12);
                       background:transparent;color:var(--text-muted);font-family:inherit;cursor:pointer;font-size:14px;">
					Annulla
				</button>
				<button type="submit" class="btn btn-honey" style="flex:1;">
					Reimposta
				</button>
			</div>
		</form>
	</div>
</div>

<script>
    lucide.createIcons();

    function openResetModal(id, nome) {
        document.getElementById('resetUserId').value   = id;
        document.getElementById('resetModalMeta').innerText = 'Utente: ' + nome;
        document.getElementById('resetModal').classList.add('show');
        lucide.createIcons();
    }
    function closeResetModal() {
        document.getElementById('resetModal').classList.remove('show');
        document.getElementById('resetForm').reset();
    }
    function bgClick(e) {
        if (e.target === document.getElementById('resetModal')) closeResetModal();
    }
    // Chiudi con ESC
    document.addEventListener('keydown', e => { if (e.key === 'Escape') closeResetModal(); });
</script>

<script src="https://cdn.jsdelivr.net/npm/bootstrap@5.3.2/dist/js/bootstrap.bundle.min.js"></script>
<script src="../js/dati.js"></script>
<script src="../js/navbar.js"></script>
</body>
</html>