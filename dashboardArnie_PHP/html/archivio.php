<?php
require_once '../auth.php';
?>
<!DOCTYPE html>
<html lang="it" data-bs-theme="dark">
<head>
    <meta charset="UTF-8" />
    <meta name="viewport" content="width=device-width, initial-scale=1.0" />
    <title>Archivio Allarmi - Smart Hive</title>

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

    <!-- HEADER INFO -->
    <div class="glass-panel p-4 mb-4 d-flex flex-wrap gap-3 align-items-center justify-content-between">
        <div>
            <div style="font-size: 18px; font-weight: 700; color: white;">Allarmi Risolti</div>
            <div style="font-size: 13px; color: var(--text-muted);">Storico degli allarmi marcati come risolti</div>
        </div>
        <div class="d-flex gap-3 flex-wrap align-items-center">
            <div class="alarms-stat stat-closed">
                <span class="num" id="countClosed">0</span>
                <span class="lbl">Risolti</span>
            </div>
        </div>
    </div>

    <!-- BARRA STRUMENTI -->
    <div class="filter-bar mb-4">
        <span style="font-size: 13px; color: var(--text-muted); font-weight: 500;">
            <i data-lucide="check-circle-2" style="width:14px;height:14px;margin-right:4px;color:var(--success);"></i>
            Solo allarmi risolti
        </span>

        <div style="margin-left: auto; display: flex; align-items: center; gap: 10px;">
            <div class="form-check form-switch d-flex align-items-center mb-0">
                <input class="form-check-input me-2" type="checkbox" role="switch" id="mockDataSwitch"
                       style="cursor: pointer; width: 2.5em; height: 1.25em; border-color: rgba(255,255,255,0.5);">
                <label class="form-check-label" for="mockDataSwitch"
                       style="font-size: 13px; cursor: pointer; color: var(--text-muted);">Demo</label>
            </div>
            <button onclick="refreshArchive()"
                    style="background: rgba(255,255,255,0.07); border: 1px solid var(--glass-border); color: var(--text-muted);
                     padding: 6px 14px; border-radius: 8px; cursor: pointer; font-size: 13px; font-family: inherit; transition: 0.2s;"
                    onmouseover="this.style.color='white'" onmouseout="this.style.color='var(--text-muted)'">
                <i data-lucide="refresh-cw" style="width: 13px; height: 13px; margin-right: 5px;"></i>Aggiorna
            </button>
        </div>
    </div>

    <!-- LISTA ARCHIVIO -->
    <div id="archiveContainer">
        <div class="text-center py-5" style="color: var(--text-muted);">
            <div style="font-size: 18px; margin-bottom: 8px;">⏳</div>
            Caricamento archivio...
        </div>
    </div>

</div>

<script src="../js/dati.js"></script>
<script src="../js/alarm_state.js"></script>
<script src="../js/thingsboard.js"></script>
<script src="../js/archivio.js"></script>
<script src="../js/navbar.js"></script>

</body>
</html>