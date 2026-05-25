<?php
require_once '../auth.php';
require_once '../db.php';
?>
<!DOCTYPE html>
<html lang="it" data-bs-theme="dark">
<head>
    <meta charset="UTF-8" />
    <meta name="viewport" content="width=device-width, initial-scale=1.0" />
    <title>Gestione Allarmi - Smart Hive</title>

    <link rel="preconnect" href="https://fonts.googleapis.com" />
    <link href="https://fonts.googleapis.com/css2?family=Inter:wght@400;500;600;700&display=swap" rel="stylesheet" />
    <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.2/dist/css/bootstrap.min.css" rel="stylesheet" />
    <script src="https://unpkg.com/lucide@latest"></script>
    <script src="https://cdn.jsdelivr.net/npm/bootstrap@5.3.2/dist/js/bootstrap.bundle.min.js"></script>

    <link rel="stylesheet" href="../css/style.css" />
    <link rel="stylesheet" href="../css/allarmi.css" />
	<link rel="stylesheet" href="../css/navbar.css" />

    <style>
        h1, h2, h3, h4, h5, h6, p { margin-bottom: 0; }
        a { text-decoration: none; }
    </style>
</head>

<body>

<?php require_once '../includes/navbar.php'; ?>

<div class="container pb-5">

    <!-- STATISTICHE HEADER -->
    <div class="glass-panel p-4 mb-4 d-flex flex-column flex-md-row gap-3 align-items-center justify-content-between">
        <div>
            <div style="font-size: 18px; font-weight: 700; color: white;">Riepilogo Allarmi</div>
            <div style="font-size: 13px; color: var(--text-muted);">Aggiornato in tempo reale da ThingsBoard</div>
        </div>
        <div class="d-flex gap-3 flex-wrap">
            <div class="alarms-stat stat-system">
                <span class="num" id="countSystem">0</span>
                <span class="lbl">Da Gestire</span>
            </div>
            <div class="alarms-stat stat-open">
                <span class="num" id="countOpen">0</span>
                <span class="lbl">Aperti</span>
            </div>
            <div class="alarms-stat stat-closed">
                <span class="num" id="countClosed">0</span>
                <span class="lbl">Risolti</span>
            </div>
        </div>
    </div>

    <!-- BARRA FILTRI -->
    <div class="filter-toolbar mb-4">
        <div class="filter-group">
            <span class="filter-label">Filtri</span>
            <button class="filter-btn active"
                    data-filter="all"
                    onclick="applyFilter('all', this)">
                Tutti
            </button>
            <button class="filter-btn f-system"
                    data-filter="system"
                    onclick="applyFilter('system', this)">
                Da gestire
            </button>
            <button class="filter-btn f-open"
                    data-filter="open"
                    onclick="applyFilter('open', this)">
                Aperti
            </button>
        </div>

        <div class="toolbar-actions">
            <div class="demo-switch">
                <span>Demo</span>
                <div class="form-check form-switch mb-0">
                    <input class="form-check-input"
                           type="checkbox"
                           role="switch"
                           id="mockDataSwitch">
                </div>
            </div>
            <a href="archivio.php" class="toolbar-btn">
                <i data-lucide="archive"></i>
                Archivio
            </a>
            <button onclick="refreshAlarms()" class="toolbar-btn">
                <i data-lucide="refresh-cw"></i>
                Aggiorna
            </button>
        </div>
    </div>

    <!-- ALLARMI DIVISI PER ARNIA -->
    <div id="alarmsContainer">
        <div class="text-center py-5" style="color: var(--text-muted);">
            <div style="font-size: 18px; margin-bottom: 8px;">⏳</div>
            Caricamento allarmi...
        </div>
    </div>

</div>

<!-- POPUP MODAL GESTIONE STATO-->
<div class="alarm-modal-overlay" id="alarmModal" onclick="closeModal(event)">
    <div class="alarm-modal">

        <div class="d-flex justify-content-between align-items-start">
            <div>
                <div class="modal-alarm-title" id="modalAlarmTitle">Titolo allarme</div>
                <div class="modal-alarm-meta"  id="modalAlarmMeta">Arnia · Data</div>
            </div>
            <button onclick="closeModalDirect()"
                    style="background: none; border: none; color: var(--text-muted); cursor: pointer; padding: 4px;">
                <i data-lucide="x" style="width: 18px; height: 18px;"></i>
            </button>
        </div>

        <hr class="modal-divider">

        <div class="modal-label">Imposta stato allarme:</div>
        <div class="modal-status-options">
            <div class="modal-status-opt" data-status="system" onclick="selectModalStatus('system')">
                <div class="modal-opt-label" style="color: var(--danger);">Da Gestire</div>
            </div>
            <div class="modal-status-opt" data-status="closed" onclick="selectModalStatus('closed')">
                <div class="modal-opt-label" style="color: var(--success);">Risolto</div>
            </div>
        </div>

        <textarea class="modal-note" id="modalNote" rows="3"
                  placeholder="Aggiungi una nota (opzionale)..."></textarea>

        <div class="modal-footer">
            <button class="btn-modal-cancel" onclick="closeModalDirect()">Annulla</button>
            <button class="btn-modal-save"   onclick="saveAlarmStatus()">Salva</button>
        </div>

    </div>
</div>

<script>const USER_ROLE = '<?= htmlspecialchars($utente_ruolo) ?>';</script>
<script src="../js/dati.js"></script>
<script src="../js/alarm_state.js"></script>
<script src="../js/thingsboard.js"></script>
<script src="../js/allarmi.js"></script>
<script src="../js/navbar.js"></script>

</body>
</html>