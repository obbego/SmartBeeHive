<?php
require_once '../auth.php';
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

    <style>
        h1, h2, h3, h4, h5, h6, p { margin-bottom: 0; }
        a { text-decoration: none; }
    </style>
</head>

<body>

<header class="mb-4 py-3 border-bottom border-secondary border-opacity-25" style="background: rgba(15, 23, 42, 0.3);">
    <div class="container d-flex justify-content-between align-items-center">
        <h1>
            <a href="index.php" class="text-white me-3" style="opacity: 0.8; transition: 0.2s;">
                <i data-lucide="arrow-left"></i>
            </a>
            <i data-lucide="bell-ring" style="color: var(--danger); filter: drop-shadow(0 0 8px rgba(239,68,68,0.5));"></i>
            <span style="margin-left: 8px;">Gestione Allarmi</span>
        </h1>
        <div style="font-size: 13px; color: var(--text-muted);">
            <?= htmlspecialchars($utente_nome) ?>
            &nbsp;·&nbsp;
            <a href="../logout.php" style="color: var(--text-muted); text-decoration: underline;">Esci</a>
        </div>
    </div>
</header>

<div class="container pb-5">

    <!-- STATISTICHE HEADER -->
    <div class="glass-panel p-4 mb-4 d-flex flex-wrap gap-3 align-items-center justify-content-between">
        <div>
            <div style="font-size: 18px; font-weight: 700; color: white;">Riepilogo Allarmi</div>
            <div style="font-size: 13px; color: var(--text-muted);">Aggiornato in tempo reale da ThingsBoard</div>
        </div>
        <div class="d-flex gap-3 flex-wrap">
            <div class="alarms-stat" style="background: rgba(245,158,11,0.1); border: 1px solid rgba(245,158,11,0.2);">
                <span class="num" id="countSystem" style="color: var(--warning);">0</span>
                <span class="lbl">DA GESTIRE</span>
            </div>
            <div class="alarms-stat" style="background: rgba(239,68,68,0.1); border: 1px solid rgba(239,68,68,0.2);">
                <span class="num" id="countOpen" style="color: var(--danger);">0</span>
                <span class="lbl">APERTI</span>
            </div>
            <div class="alarms-stat" style="background: rgba(16,185,129,0.1); border: 1px solid rgba(16,185,129,0.2);">
                <span class="num" id="countClosed" style="color: var(--success);">0</span>
                <span class="lbl">RISOLTI</span>
            </div>
        </div>
    </div>

    <!-- BARRA FILTRI -->
    <div class="filter-bar mb-4">
        <span style="font-size: 13px; color: var(--text-muted); font-weight: 500;">Filtra:</span>
        <button class="filter-btn active"   data-filter="all"    onclick="applyFilter('all', this)">Tutti</button>
        <button class="filter-btn f-system" data-filter="system" onclick="applyFilter('system', this)">
            <span style="display:inline-block;width:7px;height:7px;border-radius:50%;background:var(--warning);margin-right:5px;"></span>
            Da gestire
        </button>
        <button class="filter-btn f-open"   data-filter="open"   onclick="applyFilter('open', this)">
            <span style="display:inline-block;width:7px;height:7px;border-radius:50%;background:var(--danger);margin-right:5px;"></span>
            Aperti
        </button>
        <button class="filter-btn f-closed" data-filter="closed" onclick="applyFilter('closed', this)">
            <span style="display:inline-block;width:7px;height:7px;border-radius:50%;background:var(--success);margin-right:5px;"></span>
            Risolti
        </button>

        <div style="margin-left: auto; display: flex; align-items: center; gap: 10px;">
            <div class="form-check form-switch d-flex align-items-center mb-0">
                <input class="form-check-input me-2" type="checkbox" role="switch" id="mockDataSwitch"
                       style="cursor: pointer; width: 2.5em; height: 1.25em; border-color: rgba(255,255,255,0.5);">
                <label class="form-check-label" for="mockDataSwitch"
                       style="font-size: 13px; cursor: pointer; color: var(--text-muted);">Demo</label>
            </div>
            <button onclick="refreshAlarms()"
                    style="background: rgba(255,255,255,0.07); border: 1px solid var(--glass-border); color: var(--text-muted);
                     padding: 6px 14px; border-radius: 8px; cursor: pointer; font-size: 13px; font-family: inherit; transition: 0.2s;"
                    onmouseover="this.style.color='white'" onmouseout="this.style.color='var(--text-muted)'">
                <i data-lucide="refresh-cw" style="width: 13px; height: 13px; margin-right: 5px;"></i>Aggiorna
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

<!-- ═══════════════════════════════════════════
     POPUP MODAL GESTIONE STATO
═══════════════════════════════════════════ -->
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
                <div class="modal-opt-icon">⚙️</div>
                <div class="modal-opt-label" style="color: var(--warning);">DA GESTIRE</div>
            </div>
            <div class="modal-status-opt" data-status="open" onclick="selectModalStatus('open')">
                <div class="modal-opt-icon">🔴</div>
                <div class="modal-opt-label" style="color: var(--danger);">APERTO</div>
            </div>
            <div class="modal-status-opt" data-status="closed" onclick="selectModalStatus('closed')">
                <div class="modal-opt-icon">✅</div>
                <div class="modal-opt-label" style="color: var(--success);">RISOLTO</div>
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

<script src="../js/dati.js"></script>
<script src="../js/thingsboard.js"></script>
<script src="../js/allarmi.js"></script>

</body>
</html>