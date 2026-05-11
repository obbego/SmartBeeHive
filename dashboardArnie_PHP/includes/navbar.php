<?php
// auth.php deve essere già stato incluso dalla pagina chiamante
// $utente_nome e $utente_ruolo sono quindi già disponibili

// Lettura arnie da dati.js non è possibile server-side,
// usiamo i nomi fissi (sono sempre 5 arnie hardcoded in dati.js)
$arnie = [1, 2, 3, 4, 5];

// Prima lettera del nome per l'avatar
$iniziale = strtoupper(substr($utente_nome, 0, 1));
?>

<!-- CSS navbar -->
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
        <div class="topbar-user">
            Ciao, <strong><?= htmlspecialchars($utente_nome) ?></strong>
        </div>
    </div>
</div>

<!-- ─── OVERLAY ───────────────────────────────────────────── -->
<div class="sidebar-overlay" id="sidebarOverlay"></div>

<!-- ─── SIDEBAR ───────────────────────────────────────────── -->
<nav class="sidebar" id="mainSidebar">

    <!-- Header -->
    <div class="sidebar-header">
        <a href="index.php" class="sidebar-brand">
            <img src="../img/logo.png" alt="Smart Hive" style="height:28px; width:auto;">
            Smart Hive
        </a>
        <button class="sidebar-close" id="sidebarClose" aria-label="Chiudi menu">
            <i data-lucide="x" style="width:20px;height:20px;"></i>
        </button>
    </div>

    <!-- Corpo -->
    <div class="sidebar-body">

        <!-- Dashboard -->
        <a href="index.php" class="sidebar-link" data-page="index.php">
            <i data-lucide="layout-dashboard" style="width:18px;height:18px;"></i>
            Dashboard
        </a>

        <div class="sidebar-divider"></div>

        <!-- Arnie -->
        <div class="sidebar-section-label">Arnie</div>

        <?php foreach ($arnie as $id): ?>
        <a href="arnie.php?id=<?= $id ?>" class="sidebar-link" data-page="arnie.php">
            <i data-lucide="box" style="width:18px;height:18px;"></i>
            Arnia 0<?= $id ?>
            <span class="sidebar-dot offline" id="dot-hive-<?= $id ?>"></span>
        </a>
        <?php endforeach; ?>

        <div class="sidebar-divider"></div>

        <!-- Gestione — visibile a tutti -->
        <div class="sidebar-section-label">Gestione</div>

        <a href="allarmi.php" class="sidebar-link" data-page="allarmi.php">
            <i data-lucide="bell" style="width:18px;height:18px;"></i>
            Allarmi
        </a>
		<a href="archivio.php" class="sidebar-link" data-page="archivio.php">
			<i data-lucide="archive" style="width:18px;height:18px;"></i>
			Archivio
		</a>

        <!-- Admin — solo per admin -->
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

</nav>

<!-- JS navbar — va dopo lucide.createIcons() quindi lo carichiamo in fondo body -->
