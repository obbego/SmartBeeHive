// js/allarmi.js
// Dipende da alarm_state.js (caricato prima in allarmi.php)

// ── DATI MOCK PER MODALITÀ DEMO ──
const mockAlarmsData = [
    { id: 'mock-1', hive: 'Arnia 01', msg: 'Miele pronto per la raccolta',                 time: '14/04/26, 14:33', severity: 'WARNING',  tbStatus: 'ACTIVE_UNACK'  },
    { id: 'mock-2', hive: 'Arnia 01', msg: 'Errore nel caricamento dei dati del dispositivo', time: '23/04/26, 10:27', severity: 'MINOR',    tbStatus: 'ACTIVE_ACK'    },
    { id: 'mock-3', hive: 'Arnia 02', msg: 'Temperatura anomala rispetto alle altre arnie', time: '14/04/26, 17:11', severity: 'CRITICAL', tbStatus: 'ACTIVE_UNACK'  },
    { id: 'mock-4', hive: 'Arnia 02', msg: 'Errore nel caricamento dei dati del dispositivo', time: '24/04/26, 15:31', severity: 'MINOR',    tbStatus: 'CLEARED_ACK'   },
    { id: 'mock-5', hive: 'Arnia 03', msg: 'Frequenza sonora anomala',                      time: '22/04/26, 09:15', severity: 'MAJOR',    tbStatus: 'ACTIVE_UNACK'  },
    { id: 'mock-6', hive: 'Arnia 04', msg: 'Miele pronto per la raccolta',                  time: '09/04/26, 10:24', severity: 'WARNING',  tbStatus: 'CLEARED_UNACK' },
    { id: 'mock-7', hive: 'Arnia 04', msg: 'Peso anomalo rispetto alle altre arnie',        time: '21/04/26, 16:00', severity: 'MAJOR',    tbStatus: 'ACTIVE_ACK'    },
    { id: 'mock-8', hive: 'Arnia 05', msg: 'Miele pronto per la raccolta',                  time: '14/04/26, 17:12', severity: 'WARNING',  tbStatus: 'ACTIVE_UNACK'  },
];

// ── LABEL TIPI ALLARMI ──
const alarmTypeLabels = {
    'HoneyReady':                    'Miele pronto per la raccolta',
    'ErrorDeviceTimeseries':         'Errore nella ricezione dei dati',
    'TelemetryInvalidKey':           'Dato ricevuto non valido',
    'FailedAssetAttributes':         'Errore nel caricamento dei dati del dispositivo',
    'DeviceOldTemperature':          'Temperatura non aggiornata',
    'DeviceOldHumidity':             'Umidità non aggiornata',
    'DeviceOldWeight':               'Peso non aggiornato',
    'DeviceOldNoiseFrequency':       'Frequenza sonora non aggiornata',
    'DeviceOldNoiseIntensity':       'Intensità sonora non aggiornata',
    'DeviceDifferentTemperature':    'Temperatura anomala rispetto alle altre arnie',
    'DeviceDifferentHumidity':       'Umidità anomala rispetto alle altre arnie',
    'DeviceDifferentWeight':         'Peso anomalo rispetto alle altre arnie',
    'DeviceDifferentNoiseFrequency': 'Frequenza sonora anomala',
    'DeviceDifferentNoiseIntensity': 'Intensità sonora anomala',
    'ErrorTimeSeriesWeightDevice':   'Errore nella lettura del peso'
};

// ── STATO CORRENTE ──
let currentFilter = 'all';
let allAlarms     = [];

// ── MODAL ──
let modalAlarmId        = null;
let modalSelectedStatus = null;

// ─────────────────────────────────────────────
// RENDER
// ─────────────────────────────────────────────
function renderAlarms(alarms) {
    allAlarms = alarms;
    updateStats(alarms);
    applyFilterRender();
}

function applyFilterRender() {
    const container = document.getElementById('alarmsContainer');

    // Raggruppa per arnia (esclude sempre i risolti — vanno in archivio.php)
    const byHive = {};
    allAlarms
        .filter(a => getEffectiveAlarmStatus(a.id, a.tbStatus) !== 'closed')
        .forEach(alarm => {
            if (!byHive[alarm.hive]) byHive[alarm.hive] = { name: alarm.hive, alarms: [] };
            byHive[alarm.hive].alarms.push(alarm);
        });

    let html = '';
    Object.keys(byHive).sort().forEach(hiveName => {
        const group = byHive[hiveName];
        let filtered = group.alarms;

        if (currentFilter !== 'all') {
            filtered = filtered.filter(a => getEffectiveAlarmStatus(a.id, a.tbStatus) === currentFilter);
        }
        if (filtered.length === 0) return;

        const openCount   = group.alarms.filter(a => getEffectiveAlarmStatus(a.id, a.tbStatus) === 'open').length;
        const systemCount = group.alarms.filter(a => getEffectiveAlarmStatus(a.id, a.tbStatus) === 'system').length;
        const totalCount  = group.alarms.length;

        let countBadge = '';
        if      (systemCount > 0) countBadge = `<span class="hive-alarm-count count-system">${systemCount} da gestire</span>`;
        else if (openCount   > 0) countBadge = `<span class="hive-alarm-count count-open">${openCount} aperti</span>`;
        else                      countBadge = `<span class="hive-alarm-count count-ok">${totalCount} risolti</span>`;

        html += `
        <div class="hive-section">
          <div class="hive-section-header">
            <div class="hive-section-title">
              <i data-lucide="hexagon" style="width:16px;height:16px;color:var(--honey-glow);"></i>
              ${escHtml(hiveName)}
            </div>
            ${countBadge}
          </div>
          <div class="hive-alarms-body">
            ${filtered.map(a => renderAlarmRow(a)).join('')}
          </div>
        </div>`;
    });

    if (!html) {
        html = `
        <div class="glass-panel p-5 text-center" style="color: var(--text-muted);">
          <i data-lucide="check-circle-2" style="width:36px;height:36px;color:var(--success);margin-bottom:12px;"></i>
          <div style="font-size:16px;font-weight:600;color:white;">Nessun allarme trovato</div>
          <div style="font-size:13px;margin-top:6px;">Cambia filtro o attendi nuovi dati</div>
        </div>`;
    }

    container.innerHTML = html;
    lucide.createIcons();
}

function renderAlarmRow(alarm) {
    const status = getEffectiveAlarmStatus(alarm.id, alarm.tbStatus);
    const statusMap = {
        system: { cls: 'st-system', label: '<i data-lucide="bell-ring" style="width:11px;height:11px;margin-right:4px;"></i>DA GESTIRE' },
        open:   { cls: 'st-open',   label: 'Aperto'  },
        closed: { cls: 'st-closed', label: 'Risolto' },
    };
    const s = statusMap[status] || statusMap.system;
    const dotClass = { system: 'dot-system', open: 'dot-open', closed: 'dot-closed' }[status] || 'dot-system';

    return `
    <div class="alarm-row" data-alarm-id="${escHtml(alarm.id)}">
      <div class="alarm-severity-dot ${dotClass}"></div>
      <div class="alarm-info">
        <div class="alarm-msg">${escHtml(alarm.msg)}</div>
        <div class="alarm-meta">${escHtml(alarm.hive)} · ${escHtml(alarm.time)}</div>
      </div>
      <button class="status-btn ${s.cls}" onclick="openModal('${escHtml(alarm.id)}')">${s.label}</button>
    </div>`;
}

function updateStats(alarms) {
    // Modifica questa riga:
    let sys = 0, open = 0, closed = 0;
    alarms.forEach(a => {
        const st = getEffectiveAlarmStatus(a.id, a.tbStatus);
        if      (st === 'system') sys++;
        else if (st === 'open')   open++;
        else                      closed++;
    });
    document.getElementById('countSystem').innerText = sys;
    document.getElementById('countOpen').innerText   = open;
    document.getElementById('countClosed').innerText = closed;
}

// ─────────────────────────────────────────────
// FILTRI
// ─────────────────────────────────────────────
window.applyFilter = function(filter, btn) {
    currentFilter = filter;
    document.querySelectorAll('.filter-btn').forEach(b => b.classList.remove('active'));
    btn.classList.add('active');
    applyFilterRender();
    updateStats(allAlarms);
};

// ─────────────────────────────────────────────
// MODAL
// ─────────────────────────────────────────────
window.openModal = function(alarmId) {
    const alarm = allAlarms.find(a => a.id === alarmId);
    if (!alarm) return;

    modalAlarmId = alarmId;
    const current = getEffectiveAlarmStatus(alarmId, alarm.tbStatus);
    // Aprendo il modal passiamo da 'system' → 'open' come pre-selezione
    modalSelectedStatus = current === 'system' ? 'open' : current;

    document.getElementById('modalAlarmTitle').innerText = alarm.msg;
    document.getElementById('modalAlarmMeta').innerText  = alarm.hive + ' · ' + alarm.time;
    const states = loadLocalAlarmStates();
    document.getElementById('modalNote').value = states['note_' + alarmId] || '';

    document.querySelectorAll('.modal-status-opt').forEach(el => {
        el.className = 'modal-status-opt';
        if (el.dataset.status === modalSelectedStatus) el.classList.add('selected-' + modalSelectedStatus);
    });

    document.getElementById('alarmModal').classList.add('show');
    lucide.createIcons();
};

window.selectModalStatus = function(status) {
    modalSelectedStatus = status;
    document.querySelectorAll('.modal-status-opt').forEach(el => {
        el.className = 'modal-status-opt';
        if (el.dataset.status === status) el.classList.add('selected-' + status);
    });
};

window.closeModal = function(event) {
    if (event.target === document.getElementById('alarmModal')) closeModalDirect();
};

window.closeModalDirect = function() {
    // Se era 'system' e l'utente chiude con X, segna come 'open'
    if (modalAlarmId) {
        const alarm = allAlarms.find(a => a.id === modalAlarmId);
        const current = alarm ? getEffectiveAlarmStatus(modalAlarmId, alarm.tbStatus) : null;
        if (current === 'system') {
            const states = loadLocalAlarmStates();
            states[modalAlarmId] = 'open';
            saveLocalAlarmStates(states);
            applyFilterRender();
            updateStats(allAlarms);
        }
    }
    document.getElementById('alarmModal').classList.remove('show');
    modalAlarmId        = null;
    modalSelectedStatus = null;
};

// Toast di feedback
function showToast(message, type = 'success') {
    const existing = document.getElementById('alarmToast');
    if (existing) existing.remove();
    const colors = {
        success: { bg: 'rgba(16,185,129,0.15)', border: 'rgba(16,185,129,0.4)', color: 'var(--success)' },
        error:   { bg: 'rgba(239,68,68,0.15)',  border: 'rgba(239,68,68,0.4)',  color: 'var(--danger)'  },
    };
    const c = colors[type] || colors.success;
    const toast = document.createElement('div');
    toast.id = 'alarmToast';
    toast.style.cssText = `
        position:fixed;bottom:28px;right:28px;z-index:99999;
        background:${c.bg};border:1px solid ${c.border};color:${c.color};
        padding:14px 20px;border-radius:12px;font-size:14px;font-weight:600;
        font-family:'Inter',sans-serif;box-shadow:0 8px 32px rgba(0,0,0,0.4);
        backdrop-filter:blur(12px);max-width:360px;
    `;
    toast.innerText = message;
    document.body.appendChild(toast);
    setTimeout(() => {
        toast.style.transition = 'opacity 0.4s';
        toast.style.opacity = '0';
        setTimeout(() => toast.remove(), 400);
    }, 3500);
}

window.saveAlarmStatus = async function() {
    if (!modalAlarmId || !modalSelectedStatus) return;

    const isMock  = localStorage.getItem('mockMode') === 'true';
    const saveBtn = document.querySelector('.btn-modal-save');
    if (saveBtn) { saveBtn.innerText = 'Salvataggio…'; saveBtn.disabled = true; }

    try {
        if (!isMock) {
            // 'closed' → clear (TB: CLEARED_ACK)
            // 'open'   → ack   (TB: ACTIVE_ACK)
            // 'system' → nessuna chiamata TB
            if (modalSelectedStatus === 'closed') {
                await tbClearAlarm(modalAlarmId);
            } else if (modalSelectedStatus === 'open') {
                await tbAckAlarm(modalAlarmId);
            }
        }

        const states = loadLocalAlarmStates();
        states[modalAlarmId] = modalSelectedStatus;
        const note = document.getElementById('modalNote').value.trim();
        if (note) states['note_' + modalAlarmId] = note;
        else      delete states['note_' + modalAlarmId];
        saveLocalAlarmStates(states);

        // Reset modalAlarmId PRIMA di closeModalDirect per non innescare
        // la logica "se era system → segna open"
        const savedId = modalAlarmId;
        modalAlarmId  = null;
        document.getElementById('alarmModal').classList.remove('show');
        modalSelectedStatus = null;

        applyFilterRender();
        updateStats(allAlarms);

        const labels = { closed: 'Allarme risolto', open: 'Allarme preso in carico', system: 'Stato aggiornato' };
        showToast((labels[modalSelectedStatus] || 'Salvato') + (isMock ? ' (demo)' : ' su ThingsBoard'), 'success');

    } catch (err) {
        console.error('Errore aggiornamento allarme:', err);
        showToast('Errore: ' + err.message, 'error');
    } finally {
        if (saveBtn) { saveBtn.innerText = 'Salva'; saveBtn.disabled = false; }
    }
};

// ─────────────────────────────────────────────
// CARICAMENTO DATI
// ─────────────────────────────────────────────
window.refreshAlarms = async function() {
    const isMock = localStorage.getItem('mockMode') === 'true';

    if (isMock) {
        renderAlarms(mockAlarmsData);
        return;
    }

    document.getElementById('alarmsContainer').innerHTML = `
        <div class="text-center py-5" style="color: var(--text-muted);">
          <div style="font-size: 18px; margin-bottom: 8px;">⏳</div>
          Caricamento allarmi...
        </div>`;

    try {
        const res  = await fetch('../api.php');
        const data = await res.json();
        const raw  = data.alarms || [];

        const mapped = raw.map(alarm => ({
            id:       alarm.id?.id || String(alarm.createdTime),
            hive:     alarm.originatorName || 'Dispositivo sconosciuto',
            msg:      alarmTypeLabels[alarm.type] || alarm.type || 'Allarme sconosciuto',
            time:     alarm.createdTime
                ? new Date(alarm.createdTime).toLocaleString('it-IT', {
                    day: '2-digit', month: '2-digit', year: '2-digit',
                    hour: '2-digit', minute: '2-digit'
                })
                : '--',
            severity: alarm.severity || 'WARNING',
            tbStatus: alarm.status   || ''   // ← status grezzo TB, mai pre-mappato
        }));

        renderAlarms(mapped);
    } catch (err) {
        console.error('Errore caricamento allarmi', err);
        document.getElementById('alarmsContainer').innerHTML = `
            <div class="glass-panel p-4 text-center" style="color: var(--danger);">
              Errore nel caricamento degli allarmi. Riprova.
            </div>`;
    }
};

// ─────────────────────────────────────────────
// UTILITÀ
// ─────────────────────────────────────────────
function escHtml(str) {
    if (!str) return '';
    return String(str)
        .replace(/&/g, '&amp;')
        .replace(/</g, '&lt;')
        .replace(/>/g, '&gt;')
        .replace(/"/g, '&quot;');
}

// ─────────────────────────────────────────────
// INIT
// ─────────────────────────────────────────────
document.addEventListener('DOMContentLoaded', () => {
    const mockSwitch = document.getElementById('mockDataSwitch');
    const isMock = localStorage.getItem('mockMode') === 'true';
    if (mockSwitch) {
        mockSwitch.checked = isMock;
        mockSwitch.addEventListener('change', e => {
            localStorage.setItem('mockMode', e.target.checked);
            window.location.reload();
        });
    }
    lucide.createIcons();
    refreshAlarms();
});