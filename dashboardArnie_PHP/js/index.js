// DATI MOCK PER GLI ALLARMI
const mockAlertsHistory = [
  { hive: 'Arnia 02', msg: 'Temperatura critica', time: 'Oggi 14:23', status: 'open' },
  { hive: 'Arnia 04', msg: 'Calo peso anomalo', time: 'Oggi 11:15', status: 'open' },
  { hive: 'Arnia 01', msg: 'Umidità bassa', time: 'Ieri 16:40', status: 'closed' },
  { hive: 'Arnia 03', msg: 'Vibrazione anomala', time: '17/01/26', status: 'closed' },
  { hive: 'Arnia 05', msg: 'Batteria scarica', time: '15/01/26', status: 'closed' }
];
let alertsHistory = [];

// RENDER ARNIE
function renderHives() {
  const grid = document.getElementById('hivesGrid');
  const totalHives = hivesData.length;

  grid.innerHTML = hivesData.map((hive, index) => {
    const remainder = totalHives % 3;
    const startOfLastRow = totalHives - remainder;
    let desktopClass = 'col-lg-12';

    if (remainder > 0 && index >= startOfLastRow) {
      if (remainder === 1) desktopClass = 'col-lg-12';
      else if (remainder === 2) desktopClass = 'col-lg-12';
    }

    let statusText = 'OFFLINE';
    if (hive.status === 'green') statusText = 'ONLINE';
    if (hive.status === 'yellow') statusText = 'ATTENZIONE';
    if (hive.status === 'red') statusText = 'ALLARME';

    return `
    <div class="col-12  ${desktopClass}">
      <div class="glass-panel hive-card h-100" 
           onclick="window.location.href='arnie.html?id=${hive.id}'"
           style="cursor: pointer; transition: transform 0.2s;">

        <!-- contenitore con le informazioni delle arnie (nome, stato, metriche) --> 
        <div class="hive-info">

          <!-- nome e pallino -->
          <div class="hive-header">
            <div class="hive-name text-truncate">${hive.name}</div>
            <div class="status-dot status-${hive.status === 'offline' ? 'yellow' : hive.status}"></div>
          </div>

          <div class="columns_arnie">
          <!-- miele -->
            <div class="honey-tank-wrapper me-3">
                <div class="honey-tank" title="Riempimento: ${hive.pct}%">
                    <div class="honey-liquid" style="height: ${hive.pct}%"></div>
                </div>
                <span class="honey-pct">${hive.pct}%</span>
            </div>

            <!-- informazioni  -->
            <div class="hive-metrics">
            <div class="metric-box">
                <div class="metric-label">PESO</div>
                <div class="metric-val">${hive.w}kg</div>
              </div>
              <div class="metric-box">
                <div class="metric-label">STATO</div>
                <div class="metric-val" style="font-size:12px; line-height:22px;">
                  ${statusText}
                </div>
              </div>
              <div class="metric-box">
                <div class="metric-label">TEMP</div>
                <div class="metric-val">${hive.t}°C</div>
              </div>
              <div class="metric-box">
                <div class="metric-label">UMIDITÀ</div>
                <div class="metric-val">${hive.h}%</div>
              </div>
            </div>
          </div>
        
        </div>
      </div>
    </div>
  `}).join('');

  lucide.createIcons();
}

function renderHistory() {
  const list = document.getElementById('historyList');
  if (alertsHistory.length === 0) {
    list.innerHTML = '<div class="text-center text-muted py-4" style="font-size: 14px;">Nessun allarme registrato.</div>';
    return;
  }
  list.innerHTML = alertsHistory.map(alert => `
    <div class="history-item">
      <div>
        <div style="font-weight:600; color:white;">${alert.hive} - ${alert.msg}</div>
        <div style="font-size:12px; color:var(--text-muted);">${alert.time}</div>
      </div>
      <span class="tag ${alert.status}">${alert.status === 'open' ? 'APERTO' : 'RISOLTO'}</span>
    </div>
  `).join('');
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

// INIT PRINCIPALE CON LOGICA SWITCH
document.addEventListener('DOMContentLoaded', () => {
  const mockSwitch = document.getElementById('mockDataSwitch');
  const isMockMode = localStorage.getItem('mockMode') === 'true'; // Legge la scelta salvata

  if (mockSwitch) {
    mockSwitch.checked = isMockMode;
    mockSwitch.addEventListener('change', (e) => {
      localStorage.setItem('mockMode', e.target.checked);
      window.location.reload(); // Ricarica la pagina applicando la nuova modalità
    });
  }

  if (isMockMode) {
    // MODALITA' DEMO: Usiamo i dati mock precaricati da dati.js
    alertsHistory = [...mockAlertsHistory];
    renderHives();
    renderHistory();
    initAllCharts();
    lucide.createIcons();
  } else {
    // MODALITA' REALE: Azzeriamo i dati mock e peschiamo da ThingsBoard
    hivesData.forEach(hive => { hive.t = 0; hive.h = 0; hive.w = 0; hive.pct = 0; hive.status = 'offline'; });
    alertsHistory = []; // Nessun allarme finto
    renderHives();
    renderHistory();
    initAllCharts();
    lucide.createIcons();

    loadRealData();
    setInterval(loadRealData, 30000);
  }
});