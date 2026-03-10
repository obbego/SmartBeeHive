// DATI MOCK
const alertsHistory = [
  { hive: 'Arnia 02', msg: 'Temperatura critica', time: 'Oggi 14:23', status: 'open' },
  { hive: 'Arnia 04', msg: 'Calo peso anomalo', time: 'Oggi 11:15', status: 'open' },
  { hive: 'Arnia 01', msg: 'Umidità bassa', time: 'Ieri 16:40', status: 'closed' },
  { hive: 'Arnia 03', msg: 'Vibrazione anomala', time: '17/01/26', status: 'closed' },
  { hive: 'Arnia 05', msg: 'Batteria scarica', time: '15/01/26', status: 'closed' }
];

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

    // AGGIUNTA: onclick="location.href='...'" rende tutta la card cliccabile
    return `
    <div class="col-12 ${desktopClass}">
      <div class="glass-panel hive-card h-100" 
           onclick="window.location.href='arnie.html?id=${hive.id}'"
           style="cursor: pointer; transition: transform 0.2s;">
        <div class="hive-info">
          <div class="hive-header">
            <div class="hive-name text-truncate">${hive.name}</div>
            <div class="status-dot status-${hive.status}"></div>
          </div>
          <div class="hive-metrics">
            <div class="metric-box">
              <div class="metric-label">TEMP</div>
              <div class="metric-val">${hive.t}°C</div>
            </div>
            <div class="metric-box">
              <div class="metric-label">UMIDITÀ</div>
              <div class="metric-val">${hive.h}%</div>
            </div>
            <div class="metric-box">
              <div class="metric-label">PESO</div>
              <div class="metric-val">${hive.w}kg</div>
            </div>
            <div class="metric-box">
              <div class="metric-label">STATO</div>
              <div class="metric-val" style="font-size:12px; line-height:22px;">
                ${hive.status === 'green' ? 'ONLINE' : hive.status === 'yellow' ? 'ATTENZIONE' : 'OFFLINE'}
              </div>
            </div>
          </div>
        </div>
        
        <div class="honey-tank-wrapper ms-3">
          <div class="honey-tank" title="Riempimento: ${hive.pct}%">
            <div class="honey-liquid" style="height: ${hive.pct}%"></div>
          </div>
          <span class="honey-pct">${hive.pct}%</span>
        </div>
      </div>
    </div>
  `}).join('');

  lucide.createIcons();
}

// RENDER STORICO
function renderHistory() {
  const list = document.getElementById('historyList');
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

// GESTIONE TABS
window.switchTab = function (tabName, btn) {

  document.querySelectorAll('.tab-btn').forEach(b => b.classList.remove('active'));
  btn.classList.add('active');

  document.querySelectorAll('.tab-content').forEach(c => c.classList.remove('active'));
  document.getElementById('tab-' + tabName).classList.add('active');
};

// CONFIGURAZIONE CHART.JS
Chart.defaults.color = '#94a3b8';
Chart.defaults.borderColor = 'rgba(255, 255, 255, 0.05)';
Chart.defaults.font.family = "'Inter', sans-serif";

function initAllCharts() {
  const commonOptions = {
    responsive: true,
    maintainAspectRatio: false,
    plugins: { legend: { display: false } },
    scales: { x: { grid: { display: false } } }
  };

  // Temperatura
  new Chart(document.getElementById('tempChart'), {
    type: 'line',
    data: {
      labels: ['00', '04', '08', '12', '16', '20', '24'],
      datasets: [{
        data: [34, 34.2, 35.5, 36.1, 35.8, 35.0, 34.5],
        borderColor: '#fbbf24',
        backgroundColor: 'rgba(251, 191, 36, 0.1)',
        fill: true, tension: 0.4
      }]
    }, options: commonOptions
  });

  // Umidità
  new Chart(document.getElementById('humidityChart'), {
    type: 'line',
    data: {
      labels: ['00', '04', '08', '12', '16', '20', '24'],
      datasets: [{
        data: [58, 60, 55, 52, 54, 58, 62],
        borderColor: '#10b981',
        tension: 0.4
      }]
    }, options: commonOptions
  });

  // Miele
  new Chart(document.getElementById('honeyChart'), {
    type: 'bar',
    data: {
      labels: ['L', 'M', 'M', 'G', 'V', 'S', 'D'],
      datasets: [{
        data: [1.2, 1.5, 0.8, 2.1, 2.5, 1.9, 1.0],
        backgroundColor: '#fbbf24',
        borderRadius: 4
      }]
    }, options: commonOptions
  });

  // Suono
  new Chart(document.getElementById('soundChart'), {
    type: 'line',
    data: {
      labels: Array.from({ length: 12 }, (_, i) => i * 2),
      datasets: [{
        data: [280, 285, 290, 310, 305, 295, 290, 285, 280, 275, 280, 282],
        borderColor: '#a78bfa',
        tension: 0.4
      }]
    }, options: commonOptions
  });

  // Correlazione
  const scatterData = Array.from({ length: 30 }, () => ({ x: Math.random() * 10 + 15, y: Math.random() * 8 + 30 }));
  new Chart(document.getElementById('correlationChart'), {
    type: 'scatter',
    data: { datasets: [{ data: scatterData, backgroundColor: '#fbbf24' }] },
    options: commonOptions
  });

  // Derivata
  new Chart(document.getElementById('derivative1Chart'), {
    type: 'line',
    data: {
      labels: ['00', '04', '08', '12', '16', '20'],
      datasets: [{
        data: [0.2, 0.5, 0.1, -0.2, -0.4, 0.1],
        borderColor: '#ef4444',
        tension: 0.4
      }]
    }, options: commonOptions
  });

  // Derivata Peso
  new Chart(document.getElementById('weightDerivativeChart'), {
    type: 'bar',
    data: {
      labels: ['L', 'M', 'M', 'G', 'V', 'S', 'D'],
      datasets: [{
        data: [0.1, 0.2, 0.15, 0.3, 0.25, 0.1, 0.05],
        backgroundColor: '#10b981',
        borderRadius: 4
      }]
    }, options: commonOptions
  });
}

// INIT
document.addEventListener('DOMContentLoaded', () => {
  renderHives();
  renderHistory();
  initAllCharts();
  lucide.createIcons(); // Inizializza icone statiche se ce ne sono
});

const HOST = "https://eu.thingsboard.cloud";
const USERNAME = "francesco.bego@iisviolamarchesini.edu.it";
const PASSWORD = "ApiApi1234!";
const DEVICE_ID = "83ada8d0-171e-11f1-acb1-ebc343e93a59";

async function getToken() {

  const res = await fetch(`${HOST}/api/auth/login`, {
    method: "POST",
    headers: {
      "Content-Type": "application/json"
    },
    body: JSON.stringify({
      username: USERNAME,
      password: PASSWORD
    })
  });

  if (!res.ok) {
    throw new Error("Login fallito");
  }

  const data = await res.json();
  return data.token;
}

async function getTelemetry(token) {

  const res = await fetch(
      `${HOST}/api/plugins/telemetry/DEVICE/${DEVICE_ID}/values/timeseries?keys=temperature,humidity,weight,battery`,
      {
        headers: {
          "X-Authorization": `Bearer ${token}`
        }
      }
  );

  if (!res.ok) {
    throw new Error("Errore recupero telemetria");
  }

  return await res.json();
}

async function loadRealData() {

  try {

    const token = await getToken();
    const telemetry = await getTelemetry(token);

    console.log("Telemetry ricevuta:", telemetry);

    const lastTemp = telemetry.temperature?.slice(-1)[0];
    const lastHum = telemetry.humidity?.slice(-1)[0];
    const lastWeight = telemetry.weight?.slice(-1)[0];

    hivesData[0].t = parseFloat(lastTemp?.value || 0);
    hivesData[0].h = parseFloat(lastHum?.value || 0);
    hivesData[0].w = parseFloat(lastWeight?.value || 0);

    renderHives();

  } catch (error) {

    console.error("Errore ThingsBoard:", error);

    renderHives(); // fallback ai dati statici

  }

}

document.addEventListener('DOMContentLoaded', () => {

  loadRealData();

  // aggiorna ogni 30 secondi
  setInterval(loadRealData, 30000);

});