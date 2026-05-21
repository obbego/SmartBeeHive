<?php
require_once '../auth.php';
require_once '../db.php';

$errore  = '';
$successo = '';

// ── CAMBIO PASSWORD ─────────────────────────────────────────────────────────
if ($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_POST['azione'])) {

	// Cambio password dell'utente stesso
	if ($_POST['azione'] === 'cambia_password') {

		if (OFFLINE_MODE) {
			$errore = 'Cambio password non disponibile in modalità offline.';

		} else {
			$pass_attuale = $_POST['pass_attuale']  ?? '';
			$pass_nuova   = $_POST['pass_nuova']    ?? '';
			$pass_conferma= $_POST['pass_conferma'] ?? '';

			if (empty($pass_attuale) || empty($pass_nuova) || empty($pass_conferma)) {
				$errore = 'Compila tutti i campi.';
			} elseif (strlen($pass_nuova) < 8) {
				$errore = 'La nuova password deve essere di almeno 8 caratteri.';
			} elseif ($pass_nuova !== $pass_conferma) {
				$errore = 'Le due nuove password non coincidono.';
			} else {
				// Verifica password attuale
				$stmt = mysqli_prepare($conn,
						"SELECT password_hash FROM arnie_users WHERE id = ? LIMIT 1"
				);
				mysqli_stmt_bind_param($stmt, 'i', $utente_id);
				mysqli_stmt_execute($stmt);
				$result = mysqli_stmt_get_result($stmt);
				$row    = mysqli_fetch_assoc($result);
				mysqli_stmt_close($stmt);

				if (!$row || !password_verify($pass_attuale, $row['password_hash'])) {
					$errore = 'La password attuale non è corretta.';
				} else {
					$nuovo_hash = password_hash($pass_nuova, PASSWORD_BCRYPT);
					$upd = mysqli_prepare($conn,
							"UPDATE arnie_users SET password_hash = ? WHERE id = ?"
					);
					mysqli_stmt_bind_param($upd, 'si', $nuovo_hash, $utente_id);
					mysqli_stmt_execute($upd)
							? $successo = 'Password aggiornata con successo.'
							: $errore   = 'Errore durante il salvataggio. Riprova.';
					mysqli_stmt_close($upd);
				}
			}
		}
	}
}

// ── CARICA DATI UTENTE DAL DB ────────────────────────────────────────────────
$dati_utente = null;
if (!OFFLINE_MODE && $conn) {
	$stmt = mysqli_prepare($conn,
			"SELECT nome, ruolo, created_at FROM arnie_users WHERE id = ? LIMIT 1"
	);
	mysqli_stmt_bind_param($stmt, 'i', $utente_id);
	mysqli_stmt_execute($stmt);
	$result = mysqli_stmt_get_result($stmt);
	$dati_utente = mysqli_fetch_assoc($result);
	mysqli_stmt_close($stmt);
}

// Fallback se offline o query fallita
$nome_display   = $dati_utente['nome']     ?? $utente_nome;
$ruolo_display  = $dati_utente['ruolo']    ?? $utente_ruolo;
$created_at_raw = $dati_utente['created_at'] ?? null;

// Formatta la data in italiano
$data_iscrizione = '–';
if ($created_at_raw) {
	$mesi = ['', 'Gennaio','Febbraio','Marzo','Aprile','Maggio','Giugno',
			'Luglio','Agosto','Settembre','Ottobre','Novembre','Dicembre'];
	$ts   = strtotime($created_at_raw);
	$data_iscrizione = intval(date('d', $ts)) . ' ' . $mesi[intval(date('m', $ts))] . ' ' . date('Y', $ts);
}

$iniziale = strtoupper(substr($nome_display, 0, 1));

$ruolo_label = [
		'admin'    => 'Amministratore',
		'operator' => 'Operatore',
		'viewer'   => 'Visualizzatore',
][$ruolo_display] ?? $ruolo_display;
?>
<!DOCTYPE html>
<html lang="it" data-bs-theme="dark">
<head>
	<meta charset="UTF-8">
	<meta name="viewport" content="width=device-width, initial-scale=1.0">
	<title>Profilo • Smart Hive</title>

	<link rel="preconnect" href="https://fonts.googleapis.com">
	<link href="https://fonts.googleapis.com/css2?family=Inter:wght@400;500;600;700;800&display=swap" rel="stylesheet">
	<link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.2/dist/css/bootstrap.min.css" rel="stylesheet">
	<script src="https://unpkg.com/lucide@latest"></script>
	<link rel="stylesheet" href="../css/style.css">
	<link rel="stylesheet" href="../css/navbar.css">
	<link rel="stylesheet" href="../css/profile.css">

	<style>
        /* ── COMING SOON OVERLAY ───────────────────────────── */
        .coming-soon-wrapper {
            position: relative;
        }
        .coming-soon-overlay {
            position: absolute;
            inset: 0;
            z-index: 10;
            border-radius: 20px;
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            gap: 10px;
            background: rgba(15, 23, 42, 0.72);
            backdrop-filter: blur(6px);
            -webkit-backdrop-filter: blur(6px);
            border: 1px dashed rgba(251, 191, 36, 0.35);
        }
        .coming-soon-badge {
            display: inline-flex;
            align-items: center;
            gap: 8px;
            background: rgba(251, 191, 36, 0.12);
            border: 1px solid rgba(251, 191, 36, 0.35);
            color: var(--honey-glow);
            font-size: 13px;
            font-weight: 700;
            letter-spacing: 0.06em;
            text-transform: uppercase;
            padding: 8px 18px;
            border-radius: 999px;
        }
        .coming-soon-sub {
            font-size: 13px;
            color: var(--text-muted);
            text-align: center;
            max-width: 260px;
            line-height: 1.5;
        }

        /* ── FORM FEEDBACK ─────────────────────────────────── */
        .feedback-box {
            padding: 12px 16px;
            border-radius: 10px;
            font-size: 14px;
            font-weight: 500;
            margin-bottom: 20px;
        }
        .feedback-box.success {
            background: rgba(16,185,129,0.1);
            border: 1px solid rgba(16,185,129,0.35);
            color: var(--success);
        }
        .feedback-box.error {
            background: rgba(239,68,68,0.1);
            border: 1px solid rgba(239,68,68,0.35);
            color: var(--danger);
        }

        /* ── FORM INPUT ────────────────────────────────────── */
        .form-control {
            background: rgba(0,0,0,0.25);
            border: 1px solid var(--glass-border);
            color: var(--text-main);
            padding: 12px;
            border-radius: 10px;
            font-family: inherit;
        }
        .form-control:focus {
            border-color: var(--honey-primary);
            background: rgba(0,0,0,0.35);
            outline: none;
            box-shadow: 0 0 0 3px rgba(245,158,11,0.15);
            color: var(--text-main);
        }
        .form-control::placeholder { color: rgba(255,255,255,0.3); }

        /* ── PASSWORD STRENGTH ─────────────────────────────── */
        .pass-strength-bar {
            height: 4px;
            border-radius: 4px;
            background: rgba(255,255,255,0.08);
            margin-top: 8px;
            overflow: hidden;
        }
        .pass-strength-fill {
            height: 100%;
            border-radius: 4px;
            width: 0%;
            transition: width 0.3s ease, background 0.3s ease;
        }
        .pass-strength-label {
            font-size: 11px;
            margin-top: 4px;
            color: var(--text-muted);
        }

        h1,h2,h3,h4,h5,h6,p { margin-bottom: 0; }
        a { text-decoration: none; }
	</style>
</head>

<body>
<?php require_once '../includes/navbar.php'; ?>

<div class="container py-5">

	<div class="mb-5">
		<h1 class="hero-title mb-2">Il mio account</h1>
		<p class="text-muted">Visualizza il tuo profilo e gestisci la sicurezza</p>
	</div>

	<div class="row g-4">

		<!-- ── COL SINISTRA: INFO PROFILO ──────────────────────────── -->
		<div class="col-lg-4">
			<div class="glass-panel text-center h-100">

				<div class="profile-avatar"><?= $iniziale ?></div>
				<h2 class="h4 fw-bold mb-2"><?= htmlspecialchars($nome_display) ?></h2>

				<div class="role-badge mb-4">
					<?php if ($ruolo_display === 'admin'): ?>
						<i data-lucide="shield-check" size="16"></i>
					<?php elseif ($ruolo_display === 'operator'): ?>
						<i data-lucide="wrench" size="16"></i>
					<?php else: ?>
						<i data-lucide="eye" size="16"></i>
					<?php endif; ?>
					<?= htmlspecialchars($ruolo_label) ?>
				</div>

				<div class="text-start mt-4">
					<div class="info-box">
						<div class="info-label">Username</div>
						<div class="info-value mt-1" style="font-weight: 600;"><?= htmlspecialchars($nome_display) ?></div>
					</div>
					<div class="info-box">
						<div class="info-label">Account creato il</div>
						<div class="info-value mt-1" style="font-weight: 600;"><?= htmlspecialchars($data_iscrizione) ?></div>
					</div>
					<?php if (OFFLINE_MODE): ?>
						<div class="info-box" style="border-color: rgba(245,158,11,0.3);">
							<div class="info-label" style="color: var(--warning);">⚠ Modalità offline</div>
							<div class="info-value mt-1" style="font-size: 12px; color: var(--text-muted);">I dati mostrati sono quelli della sessione</div>
						</div>
					<?php endif; ?>

					<div class="mt-4">
						<label class="info-label d-block mb-2">Permessi attivi</label>
						<ul class="permissions-list">
							<li><i data-lucide="check-circle-2"></i> Lettura sensori</li>
							<?php if (isOperator()): ?>
								<li><i data-lucide="check-circle-2"></i> Gestione allarmi</li>
							<?php endif; ?>
							<?php if (isAdmin()): ?>
								<li><i data-lucide="check-circle-2"></i> Gestione utenti</li>
							<?php endif; ?>
						</ul>
					</div>
				</div>

				<a href="../logout.php" class="btn btn-logout w-100 mt-4">
					<i data-lucide="log-out" class="me-2"></i> Disconnetti
				</a>
			</div>
		</div>

		<!-- ── COL DESTRA: SICUREZZA + NOTIFICHE ──────────────────── -->
		<div class="col-lg-8 d-flex flex-column gap-4">

			<!-- CAMBIO PASSWORD -->
			<div class="glass-panel">
				<div class="d-flex align-items-center gap-3 mb-4">
					<div class="mini-badge p-2"><i data-lucide="lock"></i></div>
					<div>
						<h3 class="h5 mb-0 fw-bold">Sicurezza Account</h3>
						<p class="text-muted small mb-0">Modifica la tua password di accesso</p>
					</div>
				</div>

				<?php if ($successo): ?>
					<div class="feedback-box success">
						<i data-lucide="check-circle" style="width:15px;height:15px;margin-right:6px;vertical-align:middle;"></i>
						<?= htmlspecialchars($successo) ?>
					</div>
				<?php endif; ?>
				<?php if ($errore): ?>
					<div class="feedback-box error">
						<i data-lucide="alert-circle" style="width:15px;height:15px;margin-right:6px;vertical-align:middle;"></i>
						<?= htmlspecialchars($errore) ?>
					</div>
				<?php endif; ?>

				<?php if (OFFLINE_MODE): ?>
					<div class="feedback-box error">
						Cambio password non disponibile in modalità offline.
					</div>
				<?php else: ?>
					<form method="POST" id="passForm">
						<input type="hidden" name="azione" value="cambia_password">

						<div class="mb-3">
							<label class="form-label small text-muted">Password attuale</label>
							<input type="password" name="pass_attuale" class="form-control"
								   placeholder="Inserisci la password attuale" required autocomplete="current-password">
						</div>

						<div class="row g-3 mb-1">
							<div class="col-md-6">
								<label class="form-label small text-muted">Nuova password</label>
								<input type="password" name="pass_nuova" id="passNuova" class="form-control"
									   placeholder="Minimo 8 caratteri" required autocomplete="new-password"
									   oninput="checkStrength(this.value)">
								<div class="pass-strength-bar">
									<div class="pass-strength-fill" id="strengthFill"></div>
								</div>
								<div class="pass-strength-label" id="strengthLabel"></div>
							</div>
							<div class="col-md-6">
								<label class="form-label small text-muted">Conferma nuova password</label>
								<input type="password" name="pass_conferma" id="passConferma" class="form-control"
									   placeholder="Ripeti la nuova password" required autocomplete="new-password"
									   oninput="checkMatch()">
								<div class="pass-strength-label" id="matchLabel" style="margin-top:8px;"></div>
							</div>
						</div>

						<div class="mt-4">
							<button type="submit" class="btn btn-honey">
								<i data-lucide="save" style="width:15px;height:15px;margin-right:6px;vertical-align:middle;"></i>
								Aggiorna Password
							</button>
						</div>
					</form>
				<?php endif; ?>
			</div>

			<!-- NOTIFICHE (Coming Soon) -->
			<div class="coming-soon-wrapper">
				<!-- Overlay -->
				<div class="coming-soon-overlay">
                    <span class="coming-soon-badge">
                        <i data-lucide="clock" style="width:14px;height:14px;"></i>
                        In sviluppo
                    </span>
					<p class="coming-soon-sub">
						Le notifiche push per gli allarmi arriveranno in un prossimo aggiornamento.
					</p>
				</div>

				<!-- Contenuto "fantasma" sotto l'overlay -->
				<div class="glass-panel" style="pointer-events: none; user-select: none; opacity: 0.45;">
					<div class="d-flex align-items-center gap-3 mb-4">
						<div class="mini-badge p-2"><i data-lucide="bell"></i></div>
						<div>
							<h3 class="h5 mb-0 fw-bold">Preferenze Notifiche</h3>
							<p class="text-muted small mb-0">Gestisci i canali di avviso</p>
						</div>
					</div>

					<div class="settings-group">
						<div class="setting-item">
							<div>
								<h6 class="mb-1">Notifiche Push (app mobile)</h6>
								<p class="text-muted small mb-0">Ricevi avvisi direttamente sul telefono</p>
							</div>
							<div class="form-check form-switch">
								<input class="form-check-input" type="checkbox" disabled>
							</div>
						</div>
						<div class="setting-item">
							<div>
								<h6 class="mb-1">Allarmi Critici</h6>
								<p class="text-muted small mb-0">Notifica immediata per allarmi di livello alto</p>
							</div>
							<div class="form-check form-switch">
								<input class="form-check-input" type="checkbox" disabled>
							</div>
						</div>
						<div class="setting-item">
							<div>
								<h6 class="mb-1">Riepilogo Giornaliero</h6>
								<p class="text-muted small mb-0">Sommario delle arnie ogni mattina</p>
							</div>
							<div class="form-check form-switch">
								<input class="form-check-input" type="checkbox" disabled>
							</div>
						</div>
					</div>
				</div>
			</div>

		</div><!-- /col-lg-8 -->
	</div><!-- /row -->
</div>

<script>
    lucide.createIcons();

    // ── PASSWORD STRENGTH ─────────────────────────────────────────────
    function checkStrength(val) {
        const fill  = document.getElementById('strengthFill');
        const label = document.getElementById('strengthLabel');
        let score = 0;
        if (val.length >= 8)                         score++;
        if (/[A-Z]/.test(val))                       score++;
        if (/[0-9]/.test(val))                       score++;
        if (/[^A-Za-z0-9]/.test(val))               score++;

        const levels = [
            { pct: '0%',   color: 'transparent', text: '' },
            { pct: '25%',  color: 'var(--danger)',  text: 'Molto debole' },
            { pct: '50%',  color: 'var(--warning)', text: 'Debole' },
            { pct: '75%',  color: '#60a5fa',        text: 'Buona' },
            { pct: '100%', color: 'var(--success)', text: 'Ottima' },
        ];

        const lvl = val.length === 0 ? levels[0] : levels[score];
        fill.style.width      = lvl.pct;
        fill.style.background = lvl.color;
        label.innerText       = lvl.text;
        label.style.color     = lvl.color;
        checkMatch();
    }

    // ── PASSWORD MATCH ────────────────────────────────────────────────
    function checkMatch() {
        const nuova    = document.getElementById('passNuova').value;
        const conferma = document.getElementById('passConferma').value;
        const label    = document.getElementById('matchLabel');
        if (!conferma) { label.innerText = ''; return; }
        if (nuova === conferma) {
            label.innerText    = '✓ Le password coincidono';
            label.style.color  = 'var(--success)';
        } else {
            label.innerText    = '✗ Le password non coincidono';
            label.style.color  = 'var(--danger)';
        }
    }
</script>

<script src="https://cdn.jsdelivr.net/npm/bootstrap@5.3.2/dist/js/bootstrap.bundle.min.js"></script>
<script src="../js/dati.js"></script>
<script src="../js/navbar.js"></script>
</body>
</html>