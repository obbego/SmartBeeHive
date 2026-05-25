// js/archivio.js
// Dipende da alarm_state.js (caricato prima in archivio.php)

// ── DATI MOCK ──
const mockArchiveData = [
    { id: 'mock-4', hive: 'Arnia 02', msg: 'Errore attributi asset',       time: '24/04/26, 15:31', severity: 'MINOR',   tbStatus: 'CLEARED_ACK'   },
    { id: 'mock-6', hive: 'Arnia 04', msg: 'Miele pronto da raccogliere',  time: '09/04/26, 10:24', severity: 'WARNING', tbStatus: 'CLEARED_UNACK' },
];

// ── LABEL TIPI ALLARMI ──
const alarmTypeLabels = {
    'HoneyReady':                    'Miele pronto da raccogliere',
    'ErrorDeviceTimeseries':         'Errore lettura telemetria',
    'TelemetryInvalidKey':           'Chiave telemetria non valida',
    'FailedAssetAttributes':         'Errore attributi asset',
    'DeviceOldTemperature':          'Temperatura non aggiornata',
    'DeviceOldHumidity':             'Umidità non aggiornata',
    'DeviceOldWeight':               'Peso non aggiornato',
    'DeviceOldNoiseFrequency':       'Freq. rumore non aggiornata',
    'DeviceOldNoiseIntensity':       'Intensità rumore non aggiornata',
    'DeviceDifferentTemperature':    'Temperatura anomala rispetto alle altre arnie',
    'DeviceDifferentHumidity':       'Umidità anomala rispetto alle altre arnie',
    'DeviceDifferentWeight':         'Peso anomalo rispetto alle altre arnie',
    'DeviceDifferentNoiseFrequency': 'Frequenza rumore anomala',
    'DeviceDifferentNoiseIntensity': 'Intensità rumore anomala',
    'ErrorTimeSeriesWeightDevice':   'Errore lettura peso'
};

let allArchiveAlarms = [];

// ─────────────────────────────────────────────
// RENDER
// ─────────────────────────────────────────────
function renderArchive(alarms) {
    // Teniamo solo i risolti
    allArchiveAlarms = alarms.filter(a => getEffectiveAlarmStatus(a.id, a.tbStatus) === 'closed');

    const container = document.getElementById('archiveContainer');
    const countEl   = document.getElementById('countClosed');
    if (countEl) countEl.innerText = allArchiveAlarms.length;

    if (allArchiveAlarms.length === 0) {
        container.innerHTML = `
        <div class="glass-panel p-5 text-center" style="color: var(--text-muted);">
          <i data-lucide="inbox" style="width:36px;height:36px;color:var(--text-muted);margin-bottom:12px;"></i>
          <div style="font-size:16px;font-weight:600;color:white;">Nessun allarme risolto</div>
          <div style="font-size:13px;margin-top:6px;">Gli allarmi marcati come risolti appariranno qui</div>
        </div>`;
        lucide.createIcons();
        return;
    }

    // Raggruppa per arnia
    const byHive = {};
    allArchiveAlarms.forEach(alarm => {
        if (!byHive[alarm.hive]) byHive[alarm.hive] = { name: alarm.hive, alarms: [] };
        byHive[alarm.hive].alarms.push(alarm);
    });

    let html = '';
    Object.keys(byHive).sort().forEach(hiveName => {
        const group = byHive[hiveName];
        html += `
        <div class="hive-section">
          <div class="hive-section-header">
            <div class="hive-section-title">
              <i data-lucide="hexagon" style="width:16px;height:16px;color:var(--honey-glow);"></i>
              ${escHtml(hiveName)}
            </div>
            <span class="hive-alarm-count count-ok">${group.alarms.length} risolti</span>
          </div>
          <div class="hive-alarms-body">
            ${group.alarms.map(a => renderArchiveRow(a)).join('')}
          </div>
        </div>`;
    });

    container.innerHTML = html;
    lucide.createIcons();
}

function renderArchiveRow(alarm) {
    const note = loadLocalAlarmStates()['note_' + alarm.id];
    return `
    <div class="alarm-row" data-alarm-id="${escHtml(alarm.id)}">
      <div class="alarm-severity-dot dot-closed"></div>
      <div class="alarm-info">
        <div class="alarm-msg">${escHtml(alarm.msg)}</div>
        <div class="alarm-meta">
          ${escHtml(alarm.hive)} · ${escHtml(alarm.time)}
          ${note ? `<span style="margin-left:8px;color:var(--text-muted);font-style:italic;">· ${escHtml(note)}</span>` : ''}
        </div>
      </div>
      ${USER_ROLE === 'viewer'
        ? `<span class="status-btn st-closed" style="opacity:0.4; cursor:default;" title="Permessi insufficienti">✓ Risolto</span>`
        : `<button class="status-btn st-closed" onclick="reopenAlarm('${escHtml(alarm.id)}')" title="Riapri allarme">✓ Risolto</button>`
    }
    </div>`;
}

// ─────────────────────────────────────────────
// RIAPERTURA
// ─────────────────────────────────────────────
window.reopenAlarm = async function(alarmId) {
    if (USER_ROLE === 'viewer') return;
    const isMock = localStorage.getItem('mockMode') === 'true';
    try {
        if (!isMock) await tbAckAlarm(alarmId); // rimanda in ACTIVE_ACK su TB
        const states = loadLocalAlarmStates();
        states[alarmId] = 'open';
        saveLocalAlarmStates(states);
        showToast('Allarme riaperto', 'success');
        // Aggiorna lista — l'allarme scompare dall'archivio
        renderArchive(allArchiveAlarms.concat(
            allArchiveAlarms.map(a => ({ ...a })) // trigger re-filter
        ).filter((a, i, arr) => arr.findIndex(x => x.id === a.id) === i));
        // Più semplice: ricarichiamo direttamente
        await refreshArchive();
    } catch (err) {
        showToast('Errore: ' + err.message, 'error');
    }
};

// ─────────────────────────────────────────────
// CARICAMENTO
// ─────────────────────────────────────────────
window.refreshArchive = async function() {
    const isMock = localStorage.getItem('mockMode') === 'true';

    if (isMock) {
        renderArchive(mockArchiveData);
        return;
    }

    document.getElementById('archiveContainer').innerHTML = `
        <div class="text-center py-5" style="color: var(--text-muted);">
          <div style="font-size: 18px; margin-bottom: 8px;">⏳</div>
          Caricamento archivio...
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
            tbStatus: alarm.status   || ''
        }));

        renderArchive(mapped);
    } catch (err) {
        console.error('Errore caricamento archivio', err);
        document.getElementById('archiveContainer').innerHTML = `
            <div class="glass-panel p-4 text-center" style="color: var(--danger);">
              Errore nel caricamento. Riprova.
            </div>`;
    }
};

// ─────────────────────────────────────────────
// TOAST
// ─────────────────────────────────────────────
function showToast(message, type = 'success') {
    const existing = document.getElementById('archiveToast');
    if (existing) existing.remove();
    const colors = {
        success: { bg: 'rgba(16,185,129,0.15)', border: 'rgba(16,185,129,0.4)', color: 'var(--success)' },
        error:   { bg: 'rgba(239,68,68,0.15)',  border: 'rgba(239,68,68,0.4)',  color: 'var(--danger)'  },
    };
    const c = colors[type] || colors.success;
    const toast = document.createElement('div');
    toast.id = 'archiveToast';
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
    refreshArchive();
});