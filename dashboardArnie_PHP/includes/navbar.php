<?php
$just_reconnected = false;
if (!empty($_SESSION['db_just_reconnected'])) {
	$just_reconnected = true;
	unset($_SESSION['db_just_reconnected']);
}
$arnie    = [1, 2, 3, 4, 5];
$iniziale = strtoupper(substr($utente_nome, 0, 1));
?>

<link rel="stylesheet" href="../css/navbar.css">

<!-- ─── TOP BAR ───────────────────────────────────────────── -->
<div class="topbar">
	<div class="d-flex align-items-center gap-3">
		<button class="hamburger-btn" id="hamburgerBtn" aria-label="Apri menu">
			<i data-lucide="menu" style="width:20px;height:20px;"></i>
		</button>
		<a href="index.php" class="topbar-brand">
			<img src="../img/logo.png" alt="Smart Hive" style="height:28px; width:auto;">
			Smart Hive
		</a>
	</div>

	<div class="topbar-right">
		<?php if (OFFLINE_MODE): ?>
			<span style="font-size:11px; background:rgba(245,158,11,0.15); border:1px solid rgba(245,158,11,0.3); color:var(--warning); padding:3px 10px; border-radius:20px; display:flex; align-items:center; gap:6px;">
				⚠ Offline
				<a href="../force_reconnect.php"
				   title="Riprova connessione"
				   style="color:var(--warning); display:flex; align-items:center;"
				   id="reconnectBtn">
					<i data-lucide="refresh-cw" style="width:12px;height:12px;"></i>
				</a>
    		</span>
		<?php endif; ?>
		<div class="topbar-user">
			Ciao, <strong><?= htmlspecialchars($utente_nome) ?></strong>
		</div>
	</div>
</div>

<!-- ─── OVERLAY ───────────────────────────────────────────── -->
<div class="sidebar-overlay" id="sidebarOverlay"></div>

<!-- ─── SIDEBAR ───────────────────────────────────────────── -->
<nav class="sidebar" id="mainSidebar">

	<div class="sidebar-header">
		<a href="index.php" class="sidebar-brand">
			<img src="../img/logo.png" alt="Smart Hive" style="height:28px; width:auto;">
			Smart Hive
		</a>
		<button class="sidebar-close" id="sidebarClose" aria-label="Chiudi menu">
			<i data-lucide="x" style="width:20px;height:20px;"></i>
		</button>
	</div>

	<div class="sidebar-body">

		<a href="index.php" class="sidebar-link" data-page="index.php">
			<i data-lucide="layout-dashboard" style="width:18px;height:18px;"></i>
			Dashboard
		</a>

		<div class="sidebar-divider"></div>

		<div class="sidebar-section-label">Arnie</div>

		<?php foreach ($arnie as $id): ?>
			<a href="arnie.php?id=<?= $id ?>" class="sidebar-link" data-page="arnie.php">
				<i data-lucide="box" style="width:18px;height:18px;"></i>
				Arnia 0<?= $id ?>
				<span class="sidebar-dot offline" id="dot-hive-<?= $id ?>"></span>
			</a>
		<?php endforeach; ?>

		<div class="sidebar-divider"></div>

		<div class="sidebar-section-label">Gestione</div>

		<a href="allarmi.php" class="sidebar-link" data-page="allarmi.php">
			<i data-lucide="bell" style="width:18px;height:18px;"></i>
			Allarmi
		</a>
		<a href="archivio.php" class="sidebar-link" data-page="archivio.php">
			<i data-lucide="archive" style="width:18px;height:18px;"></i>
			Archivio
		</a>
		<a href="profile.php" class="sidebar-link" data-page="profile.php">
			<i data-lucide="user" style="width:18px;height:18px;"></i>
			Il mio profilo
		</a>

		<?php if ($utente_ruolo === 'admin'): ?>
			<div class="sidebar-divider"></div>
			<div class="sidebar-section-label">Admin</div>
			<a href="utenti.php" class="sidebar-link" data-page="utenti.php">
				<i data-lucide="users" style="width:18px;height:18px;"></i>
				Gestione Utenti
			</a>
		<?php endif; ?>

	</div>

	<!-- Footer utente -->
	<div class="sidebar-footer">

		<!-- Switch modalità demo -->
		<div style="display:flex; align-items:center; justify-content:space-between; margin-bottom:14px; padding:10px 12px; background:rgba(255,255,255,0.03); border-radius:10px; border:1px solid var(--glass-border);">
			<div style="display:flex; align-items:center; gap:8px; font-size:13px; color:var(--text-muted);">
				<i data-lucide="database" style="width:15px;height:15px;"></i>
				Dati demo
			</div>
			<div class="form-check form-switch mb-0">
				<input class="form-check-input" type="checkbox" role="switch" id="mockDataSwitch"
					   style="cursor:pointer; width:2.2em; height:1.1em;">
			</div>
		</div>

		<div class="sidebar-user-info">
			<div class="sidebar-avatar"><?= $iniziale ?></div>
			<div>
				<div class="sidebar-user-name"><?= htmlspecialchars($utente_nome) ?></div>
				<div class="sidebar-user-role"><?= htmlspecialchars($utente_ruolo) ?></div>
			</div>
		</div>
		<a href="../logout.php" class="sidebar-logout">
			<i data-lucide="log-out" style="width:15px;height:15px;"></i>
			Esci
		</a>
	</div>
	<?php if ($just_reconnected): ?>
		<script>
            document.addEventListener('DOMContentLoaded', () => {
                const t = document.createElement('div');
                t.style.cssText = `
        position:fixed;bottom:28px;right:28px;z-index:99999;
        background:rgba(16,185,129,0.15);border:1px solid rgba(16,185,129,0.4);
        color:var(--success);padding:14px 20px;border-radius:12px;
        font-size:14px;font-weight:600;font-family:'Inter',sans-serif;
        box-shadow:0 8px 32px rgba(0,0,0,0.4);backdrop-filter:blur(12px);
    `;
                t.innerText = '✓ Database raggiunto — effettua il login con il tuo account reale';
                document.body.appendChild(t);
                setTimeout(() => {
                    t.style.transition = 'opacity 0.4s';
                    t.style.opacity = '0';
                    setTimeout(() => t.remove(), 400);
                }, 6000);
            });
		</script>
	<?php endif; ?>
</nav>