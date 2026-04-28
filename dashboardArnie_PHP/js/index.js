// DATI MOCK PER GLI ALLARMI
const mockAlertsHistory = [
  { hive: 'Arnia 02', msg: 'Temperatura critica', time: 'Oggi 14:23', status: 'open' },
  { hive: 'Arnia 04', msg: 'Calo peso anomalo', time: 'Oggi 11:15', status: 'open' },
  { hive: 'Arnia 01', msg: 'Umidità bassa', time: 'Ieri 16:40', status: 'closed' },
  { hive: 'Arnia 03', msg: 'Vibrazione anomala', time: '17/01/26', status: 'closed' },
  { hive: 'Arnia 05', msg: 'Batteria scarica', time: '15/01/26', status: 'closed' }
];
let alertsHistory = [];

// ─────────────────────────────────────────────
// LOGICA COLORI METRICHE (stessi range di arnie.js)
// Restituisce una stringa CSS o '' per il default
// ─────────────────────────────────────────────
function getTempColor(t) {
  const v = parseFloat(t);
  if (isNaN(v) || v <= 0)      return '';
  if (v >= 38.5 || v < 30)     return 'var(--danger)';
  if (v >= 37.5 || v < 32)     return 'var(--warning)';
  return '';
}

function getHumColor(h) {
  const v = parseFloat(h);
  if (isNaN(v) || v <= 0)      return '';
  if (v > 75 || v < 25)        return 'var(--danger)';
  if (v > 65 || v < 35)        return 'var(--warning)';
  return '';
}

function getWeightColor(w) {
  const v = parseFloat(w);
  if (isNaN(v) || v <= 0)      return '';
  if (v < 15)                  return 'var(--danger)';
  if (v < 25)                  return 'var(--warning)';
  return '';
}

function getFreqColor(f) {
  const v = parseFloat(f);
  if (isNaN(v) || v <= 0)      return '';
  if (v >= 380)                return 'var(--danger)';
  if (v >= 270)                return 'var(--warning)';
  return '';
}

// Converte un colore CSS in stile inline (stringa vuota = nessun override)
function colorStyle(color) {
  return color ? `color: ${color}; font-weight: 700;` : '';
}

// ─────────────────────────────────────────────

// CALCOLO STATISTICHE DINAMICHE
function computeStats() {
  const activeHives = hivesData.filter(h => h.status !== 'offline');

  const totalHoney = hivesData.reduce((sum, h) => {
    const honey = parseFloat(h.w);
    return sum + (isNaN(honey) ? 0 : honey);
  }, 0);

  const activeAlarms = hivesData.filter(h => h.status === 'red').length;

  const temps = activeHives.map(h => parseFloat(h.t)).filter(v => !isNaN(v) && v > 0);
  const avgTemp = temps.length > 0 ? (temps.reduce((a, b) => a + b, 0) / temps.length) : 0;

  const hums = activeHives.map(h => parseFloat(h.h)).filter(v => !isNaN(v) && v > 0);
  const avgHum = hums.length > 0 ? (hums.reduce((a, b) => a + b, 0) / hums.length) : 0;

  const firstTOut = hivesData.map(h => parseFloat(h.tOut)).find(v => !isNaN(v) && v !== 0 && v !== undefined);
  const tempOut = (firstTOut !== undefined && firstTOut !== null) ? firstTOut : null;

  return { totalHoney, activeAlarms, avgTemp, avgHum, tempOut };
}

function renderStats() {
  const { totalHoney, activeAlarms, avgTemp, avgHum, tempOut } = computeStats();

  const honeyEl = document.getElementById('statHoney');
  if (honeyEl) honeyEl.innerText = totalHoney.toFixed(1) + ' kg';

  const alarmsEl = document.getElementById('statAlarms');
  if (alarmsEl) alarmsEl.innerText = activeAlarms;

  const tempEl = document.getElementById('statTemp');
  if (tempEl) tempEl.innerText = avgTemp > 0 ? avgTemp.toFixed(1) + '°C' : '--';

  const humEl = document.getElementById('statHum');
  if (humEl) humEl.innerText = avgHum > 0 ? avgHum.toFixed(1) + '%' : '--';

  const tOutEl = document.getElementById('statTempOut');
  if (tOutEl) tOutEl.innerText = tempOut !== null ? tempOut.toFixed(1) + '°C' : '--';

  const alarmsDetail = document.getElementById('statAlarmsDetail');
  if (alarmsDetail) {
    alarmsDetail.innerText = activeAlarms > 0 ? 'Attenzione richiesta' : 'Tutto regolare';
  }
}

// RENDER ARNIE
function renderHives() {
  const grid = document.getElementById('hivesGrid');

  grid.innerHTML = hivesData.map((hive) => {

    // Testo e colore stato per l'header
    let statusText  = 'OFFLINE';
    let statusColor = 'var(--warning)';
    if (hive.status === 'green')  { statusText = 'ONLINE';     statusColor = 'var(--success)'; }
    if (hive.status === 'yellow') { statusText = 'ATTENZIONE'; statusColor = 'var(--warning)'; }
    if (hive.status === 'red')    { statusText = 'ALLARME';    statusColor = 'var(--danger)';  }

    // Colori metriche — stessa logica di arnie.js
    const freqVal    = parseFloat(hive.peakFreq);
    const freqDisplay = (!isNaN(freqVal) && freqVal > 0) ? freqVal + ' Hz' : '--';

    const tempStyle   = colorStyle(getTempColor(hive.t));
    const humStyle    = colorStyle(getHumColor(hive.h));
    const weightStyle = colorStyle(getWeightColor(hive.w));
    const freqStyle   = colorStyle(getFreqColor(hive.peakFreq));

    return `
    <div class="col-12">
      <div class="glass-panel hive-card h-100"
           onclick="window.location.href='arnie.php?id=${hive.id}'"
           style="cursor: pointer; transition: transform 0.2s;">

        <div class="hive-info">

          <div class="hive-header">
            <div class="hive-name text-truncate">${hive.name}</div>
            <div class="d-flex align-items-center gap-2">
              <span style="font-size: 11px; font-weight: 600; color: ${statusColor}; letter-spacing: 0.04em;">
                ${statusText}
              </span>
              <div class="status-dot status-${hive.status === 'offline' ? 'yellow' : hive.status}"></div>
            </div>
          </div>

          <div class="columns_arnie">
            <div class="honey-tank-wrapper me-3">
              <div class="honey-tank" title="Riempimento: ${hive.pct}%">
                <div class="honey-liquid" style="height: ${hive.pct}%"></div>
              </div>
              <span class="honey-pct">${hive.pct}%</span>
            </div>

            <div class="hive-metrics">
              <div class="metric-box">
                <div class="metric-label">PESO</div>
                <div class="metric-val" style="${weightStyle}">${hive.w}kg</div>
              </div>
              <div class="metric-box">
                <div class="metric-label">FREQ. PICCO</div>
                <div class="metric-val" style="${freqStyle}">${freqDisplay}</div>
              </div>
              <div class="metric-box">
                <div class="metric-label">TEMP</div>
                <div class="metric-val" style="${tempStyle}">${hive.t}°C</div>
              </div>
              <div class="metric-box">
                <div class="metric-label">UMIDITÀ</div>
                <div class="metric-val" style="${humStyle}">${hive.h}%</div>
              </div>
            </div>
          </div>

        </div>
      </div>
    </div>
  `}).join('');

  lucide.createIcons();
  renderStats();
}
function loadLocalAlarmStates() {
  try { return JSON.parse(localStorage.getItem('alarmStates') || '{}'); } catch(e) { return {}; }
}
function saveLocalAlarmStates(states) {
  localStorage.setItem('alarmStates', JSON.stringify(states));
}
function getEffectiveAlarmStatus(alarmId, fallback) {
  const states = loadLocalAlarmStates();
  return states[alarmId] !== undefined ? states[alarmId] : fallback;
}
function cycleAlarmStatus(alarmId, fallback) {
  const states = loadLocalAlarmStates();
  const current = states[alarmId] !== undefined ? states[alarmId] : fallback;
  const cycle = { system: 'open', open: 'closed', closed: 'system' };
  states[alarmId] = cycle[current] || 'system';
  saveLocalAlarmStates(states);
  renderHistory();
}

function renderHistory() {
  const list = document.getElementById('historyList');
  if (alertsHistory.length === 0) {
    list.innerHTML = '<div class="text-center text-muted py-4" style="font-size: 14px;">Nessun allarme registrato.</div>';
    return;
  }
  list.innerHTML = alertsHistory.map(alert => {
    const alarmId = alert.id || (alert.hive + '_' + alert.time);
    const effectiveStatus = getEffectiveAlarmStatus(alarmId, alert.status);
    const statusMap = {
      system: { cls: 'tag-system', label: '⚙ DA GESTIRE' },
      open:   { cls: 'tag open',   label: '● APERTO'     },
      closed: { cls: 'tag closed', label: '✓ RISOLTO'    },
    };
    const s = statusMap[effectiveStatus] || statusMap.system;
    return `
    <div class="history-item">
      <div>
        <div style="font-weight:600; color:white;">${alert.hive} - ${alert.msg}</div>
        <div style="font-size:12px; color:var(--text-muted);">${alert.time}</div>
      </div>
      <button class="${s.cls}"
        onclick="cycleAlarmStatus('${alarmId}', '${alert.status}')"
        style="background:none;border:none;cursor:pointer;font-family:inherit;padding:0;"
        title="Clicca per cambiare stato">
        ${s.label}
      </button>
    </div>
  `}).join('');
}

window.switchTab = function (tabName, btn) {
  document.querySelectorAll('.tab-btn').forEach(b => b.classList.remove('active'));
  btn.classList.add('active');
  document.querySelectorAll('.tab-content').forEach(c => c.classList.remove('active'));
  document.getElementById('tab-' + tabName).classList.add('active');
};

Chart.defaults.color = '#94a3b8';
Chart.defaults.borderColor = 'rgba(255, 255, 255, 0.05)';
Chart.defaults.font.family = "'Inter', sans-serif";

function initAllCharts() {
  const commonOptions = { responsive: true, maintainAspectRatio: false, plugins: { legend: { display: false } }, scales: { x: { grid: { display: false } } } };
  new Chart(document.getElementById('tempChart'), { type: 'line', data: { labels: ['00', '04', '08', '12', '16', '20', '24'], datasets: [{ data: [34, 34.2, 35.5, 36.1, 35.8, 35.0, 34.5], borderColor: '#fbbf24', backgroundColor: 'rgba(251, 191, 36, 0.1)', fill: true, tension: 0.4 }] }, options: commonOptions });
  new Chart(document.getElementById('humidityChart'), { type: 'line', data: { labels: ['00', '04', '08', '12', '16', '20', '24'], datasets: [{ data: [58, 60, 55, 52, 54, 58, 62], borderColor: '#10b981', tension: 0.4 }] }, options: commonOptions });
  new Chart(document.getElementById('honeyChart'), { type: 'bar', data: { labels: ['L', 'M', 'M', 'G', 'V', 'S', 'D'], datasets: [{ data: [1.2, 1.5, 0.8, 2.1, 2.5, 1.9, 1.0], backgroundColor: '#fbbf24', borderRadius: 4 }] }, options: commonOptions });
  new Chart(document.getElementById('soundChart'), { type: 'line', data: { labels: Array.from({ length: 12 }, (_, i) => i * 2), datasets: [{ data: [280, 285, 290, 310, 305, 295, 290, 285, 280, 275, 280, 282], borderColor: '#a78bfa', tension: 0.4 }] }, options: commonOptions });
  const scatterData = Array.from({ length: 30 }, () => ({ x: Math.random() * 10 + 15, y: Math.random() * 8 + 30 }));
  new Chart(document.getElementById('correlationChart'), { type: 'scatter', data: { datasets: [{ data: scatterData, backgroundColor: '#fbbf24' }] }, options: commonOptions });
  new Chart(document.getElementById('derivative1Chart'), { type: 'line', data: { labels: ['00', '04', '08', '12', '16', '20'], datasets: [{ data: [0.2, 0.5, 0.1, -0.2, -0.4, 0.1], borderColor: '#ef4444', tension: 0.4 }] }, options: commonOptions });
  new Chart(document.getElementById('weightDerivativeChart'), { type: 'bar', data: { labels: ['L', 'M', 'M', 'G', 'V', 'S', 'D'], datasets: [{ data: [0.1, 0.2, 0.15, 0.3, 0.25, 0.1, 0.05], backgroundColor: '#10b981', borderRadius: 4 }] }, options: commonOptions });
}

async function loadRealData() {
  try {
    await tbLoadAllHives();
    renderHives();
  } catch (err) {
    console.error("Errore caricamento ThingsBoard", err);
    renderHives();
  }
}

document.addEventListener('DOMContentLoaded', async () => {
  const mockSwitch = document.getElementById('mockDataSwitch');
  const isMockMode = localStorage.getItem('mockMode') === 'true';

  if (mockSwitch) {
    mockSwitch.checked = isMockMode;
    mockSwitch.addEventListener('change', (e) => {
      localStorage.setItem('mockMode', e.target.checked);
      window.location.reload();
    });
  }

  if (isMockMode) {
    alertsHistory = [...mockAlertsHistory];
    renderHives();
    renderHistory();
    initAllCharts();
    lucide.createIcons();
  } else {
    hivesData.forEach(hive => { hive.t = 0; hive.h = 0; hive.w = 0; hive.pct = 0; hive.tOut = 0; hive.peakFreq = 0; hive.status = 'offline'; });
    alertsHistory = [];
    renderHives();
    renderHistory();
    initAllCharts();
    lucide.createIcons();

    loadRealData();
    setInterval(loadRealData, 30000);

    try {
      alertsHistory = await tbLoadAlarms();
      renderHistory();
    } catch (err) {
      console.error("Errore caricamento allarmi", err);
    }
  }
});
