// DATI MOCK PER GLI ALLARMI
const mockAlertsHistory = [
    {hive: 'Arnia 02', msg: 'Temperatura critica', time: 'Oggi 14:23', status: 'open'},
    {hive: 'Arnia 04', msg: 'Calo peso anomalo', time: 'Oggi 11:15', status: 'open'},
    {hive: 'Arnia 01', msg: 'Umidità bassa', time: 'Ieri 16:40', status: 'closed'},
    {hive: 'Arnia 03', msg: 'Vibrazione anomala', time: '17/01/26', status: 'closed'},
    {hive: 'Arnia 05', msg: 'Batteria scarica', time: '15/01/26', status: 'closed'}
];
let alertsHistory = [];

const zoomConfig = {
    pan: {
        enabled: true,
        mode: 'x',
    },
    zoom: {
        wheel: { enabled: true },
        pinch: { enabled: true },
        mode: 'x',
    }
};

// ─────────────────────────────────────────────
// LOGICA COLORI METRICHE
// ─────────────────────────────────────────────
function getTempColor(t) {
    const v = parseFloat(t);
    if (isNaN(v) || v <= 0) return '';
    if (v >= 38.5 || v < 30) return 'var(--danger)';
    if (v >= 37.5 || v < 32) return 'var(--warning)';
    return '';
}
function getHumColor(h) {
    const v = parseFloat(h);
    if (isNaN(v) || v <= 0) return '';
    if (v > 75 || v < 25) return 'var(--danger)';
    if (v > 65 || v < 35) return 'var(--warning)';
    return '';
}
function getWeightColor(w) {
    const v = parseFloat(w);
    if (isNaN(v) || v <= 0) return '';
    if (v < 15) return 'var(--danger)';
    if (v < 25) return 'var(--warning)';
    return '';
}
function getFreqColor(f) {
    const v = parseFloat(f);
    if (isNaN(v) || v <= 0) return '';
    if (v >= 380) return 'var(--danger)';
    if (v >= 270) return 'var(--warning)';
    return '';
}
function colorStyle(color) {
    return color ? `color: ${color}; font-weight: 700;` : '';
}

// ─────────────────────────────────────────────
// STATISTICHE DASHBOARD
// ─────────────────────────────────────────────
function computeStats() {
    const activeHives = hivesData.filter(h => h.status !== 'offline');
    const totalHoney = hivesData.reduce((sum, h) => {
        const honey = parseFloat(h.w);
        return sum + (isNaN(honey) ? 0 : honey);
    }, 0);

    const activeAlarms = alertsHistory.filter(a => {
        const alarmId = a.id || (a.hive + '_' + a.time);
        const status = getEffectiveAlarmStatus(alarmId, a.tbStatus);
        return status === 'system' || status === 'open';
    }).length;  const temps = activeHives.map(h => parseFloat(h.t)).filter(v => !isNaN(v) && v > 0);

    const avgTemp = temps.length > 0 ? temps.reduce((a, b) => a + b, 0) / temps.length : 0;
    const hums = activeHives.map(h => parseFloat(h.h)).filter(v => !isNaN(v) && v > 0);
    const avgHum = hums.length > 0 ? hums.reduce((a, b) => a + b, 0) / hums.length : 0;
    const firstTOut = hivesData.map(h => parseFloat(h.tOut)).find(v => !isNaN(v) && v !== 0 && v !== undefined);
    const tempOut = (firstTOut !== undefined && firstTOut !== null) ? firstTOut : null;
    return {totalHoney, activeAlarms, avgTemp, avgHum, tempOut};
}

function renderStats() {
    const {totalHoney, activeAlarms, avgTemp, avgHum, tempOut} = computeStats();
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
    if (alarmsDetail) alarmsDetail.innerText = activeAlarms > 0 ? 'Attenzione richiesta' : 'Tutto regolare';
}

// ─────────────────────────────────────────────
// RENDER ARNIE
// ─────────────────────────────────────────────
function renderHives() {
    const grid = document.getElementById('hivesGrid');
    grid.innerHTML = hivesData.map((hive) => {
        let statusText = 'OFFLINE';
        let statusColor = 'var(--danger)';
        if (hive.status === 'green')  { statusText = 'ONLINE';     statusColor = 'var(--success)'; }
        if (hive.status === 'yellow') { statusText = 'ATTENZIONE'; statusColor = 'var(--warning)'; }
        if (hive.status === 'red')    { statusText = 'ALLARME';    statusColor = 'var(--danger)';  }

        const freqVal = parseFloat(hive.peakFreq);
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
           <div class="d-flex flex-column align-items-end">
             <div class="d-flex align-items-center gap-2">
               <span style="font-size: 11px; font-weight: 600; color: ${statusColor}; letter-spacing: 0.04em;">
                 ${statusText}
               </span>
               <div class="status-dot status-${hive.status === 'offline' ? 'red' : hive.status}"></div>
             </div>
             ${hive.status === 'yellow' && hive.lastUpdate === 'Dati non aggiornati da oltre 24 ore'
            ? `<div style="font-size: 11px; color: var(--warning); font-style: italic; margin-top: 2px;">
                    i dati risalgono a più di 24h fa
                  </div>`
            : ''}
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
   </div>`;
    }).join('');

    lucide.createIcons();
    renderStats();
}

// ─────────────────────────────────────────────
// STORICO ALLARMI
// ─────────────────────────────────────────────
// loadLocalAlarmStates / saveLocalAlarmStates / getEffectiveAlarmStatus
// sono definite in alarm_state.js (caricato prima in index.php)

function renderHistory() {
    const list = document.getElementById('historyList');

    const active = alertsHistory.filter(alert => {
        const alarmId = alert.id || (alert.hive + '_' + alert.time);
        return getEffectiveAlarmStatus(alarmId, alert.tbStatus) !== 'closed';
    });

    if (active.length === 0) {
        const closedCount = alertsHistory.filter(alert => {
            const alarmId = alert.id || (alert.hive + '_' + alert.time);
            return getEffectiveAlarmStatus(alarmId, alert.tbStatus) === 'closed';
        }).length;
        list.innerHTML = `
      <div class="text-center text-muted py-4" style="font-size: 14px;">
        Nessun allarme attivo.
        ${closedCount > 0 ? `<br><a href="archivio.php" style="color:var(--success);font-size:13px;text-decoration:underline;">${closedCount} risolti in archivio →</a>` : ''}
      </div>`;
        return;
    }

    list.innerHTML = active.map(alert => {
        const alarmId = alert.id || (alert.hive + '_' + alert.time);
        const effectiveStatus = getEffectiveAlarmStatus(alarmId, alert.tbStatus);
        const statusMap = {
            system: { cls: 'status-btn st-system', label: '<i data-lucide="bell-ring" style="width:11px;height:11px;margin-right:4px;"></i>Da Gestire' },
            open:   { cls: 'status-btn st-open',   label: '● Aperto' },
        };
        const s = statusMap[effectiveStatus] || statusMap.system;
        return `
    <div class="history-item">
      <div>
        <div style="font-weight:600; color:white;">${alert.hive} - ${alert.msg}</div>
        <div style="font-size:12px; color:var(--text-muted);">${alert.time}</div>
      </div>
      <button class="${s.cls}" onclick="openIndexModal('${alarmId}')" title="Gestisci allarme">
        ${s.label}
      </button>
    </div>`;
    }).join('');

    lucide.createIcons();
}

// ─────────────────────────────────────────────
// MODAL ALLARMI IN INDEX
// ─────────────────────────────────────────────
let indexModalAlarmId       = null;
let indexModalSelectedStatus = null;

function openIndexModal(alarmId) {
    const alert = alertsHistory.find(a => (a.id || (a.hive + '_' + a.time)) === alarmId);
    if (!alert) return;

    indexModalAlarmId = alarmId;
    const current = getEffectiveAlarmStatus(alarmId, alert.tbStatus);

    // Se era "da gestire" → diventa automaticamente "aperto"
    indexModalSelectedStatus = current === 'system' ? 'open' : current;

    // Se stava diventando "aperto", salvalo subito
    if (current === 'system') {
        const states = loadLocalAlarmStates();
        states[alarmId] = 'open';
        saveLocalAlarmStates(states);
        renderHistory();
    }

    document.getElementById('indexModalTitle').innerText = alert.msg;
    document.getElementById('indexModalMeta').innerText  = alert.hive + ' · ' + alert.time;
    document.getElementById('indexModalNote').value = loadLocalAlarmStates()['note_' + alarmId] || '';

    document.querySelectorAll('#alarmModalIndex .modal-status-opt').forEach(el => {
        el.className = 'modal-status-opt';
        if (el.dataset.status === indexModalSelectedStatus) {
            el.classList.add('selected-' + indexModalSelectedStatus);
        }
    });

    document.getElementById('alarmModalIndex').classList.add('show');
    lucide.createIcons();
}

function selectIndexModalStatus(status) {
    indexModalSelectedStatus = status;
    document.querySelectorAll('#alarmModalIndex .modal-status-opt').forEach(el => {
        el.className = 'modal-status-opt';
        if (el.dataset.status === status) el.classList.add('selected-' + status);
    });
}

function closeIndexModal(event) {
    if (event.target === document.getElementById('alarmModalIndex')) closeIndexModalDirect();
}

function closeIndexModalDirect() {
    // Chiudere con X → se era "da gestire" rimane "aperto" (già salvato in openIndexModal)
    document.getElementById('alarmModalIndex').classList.remove('show');
    indexModalAlarmId        = null;
    indexModalSelectedStatus = null;
}

function saveIndexAlarmStatus() {
    if (!indexModalAlarmId || !indexModalSelectedStatus) return;
    const states = loadLocalAlarmStates();
    states[indexModalAlarmId] = indexModalSelectedStatus;
    const note = document.getElementById('indexModalNote').value.trim();
    if (note) states['note_' + indexModalAlarmId] = note;
    else      delete states['note_' + indexModalAlarmId];
    saveLocalAlarmStates(states);
    closeIndexModalDirect();
    renderHistory();
}

// Switch tra le tab della dashboard (Storico, Panoramica, Analisi)
// e gestione dinamica del selettore 24h / 7d / 30d
window.switchTab = function (tabName, btn) {
    document.querySelectorAll('.tab-btn').forEach(b => b.classList.remove('active'));
    btn.classList.add('active');

    document.querySelectorAll('.tab-content').forEach(c => c.classList.remove('active'));
    document.getElementById('tab-' + tabName).classList.add('active');

    const selector = document.getElementById('globalTimeSelector');

    const show = (tabName === 'overview' || tabName === 'analysis');

    selector.classList.toggle('d-none', !show);

    if (show) {
        renderTimeSelector(tabName);
    }
};

// Genera il selettore temporale globale (24h / 7d / 30d)
// e collega il caricamento dei grafici in base al tab attivo
function renderTimeSelector(type) {
    const container = document.getElementById('globalTimeSelector');
    if (!container) return;

    container.innerHTML = `
    <button class="tab-btn" data-value="24h">24 Ore</button>
    <button class="tab-btn" data-value="7d">7 Giorni</button>
    <button class="tab-btn" data-value="30d">1 Mese</button>
    <button class="tab-btn" data-value="1a">1 Anno</button>
  `;

    // reset + set default active
    const defaultBtn = container.querySelector('[data-value="24h"]');
    if (defaultBtn) defaultBtn.classList.add('active');

    container.querySelectorAll('.tab-btn').forEach(btn => {
        btn.addEventListener('click', async () => {

            container.querySelectorAll('.tab-btn')
                .forEach(b => b.classList.remove('active'));

            btn.classList.add('active');

            const interval = btn.dataset.value;

            if (type === 'overview') {
                await loadOverviewCharts(interval);
            } else {
                await loadAnalysisCharts(interval);
            }
        });
    });
}

Chart.defaults.color = '#94a3b8';
Chart.defaults.borderColor = 'rgba(255, 255, 255, 0.05)';
Chart.defaults.font.family = "'Inter', sans-serif";

// ─────────────────────────────────────────────
// UTILITY SERIE TEMPORALI (condivisa da entrambe le sezioni)
// ─────────────────────────────────────────────

// Arrotonda il timestamp al blocco di X minuti più vicino (es. 15 min)
function roundTs(ts, minutes = 15) {
    const ms = 1000 * 60 * minutes;
    return Math.round(ts / ms) * ms;
}

function parseSeries(series, roundMinutes = 15) {
    if (!series || series.length === 0) return { labels: [], data: [], raw: [] };
    const sorted = [...series].sort((a, b) => a.ts - b.ts);

    // Arrotondiamo i ts per facilitare l'allineamento nei grafici
    const alignedData = sorted.map(p => ({
        ts: roundTs(p.ts, roundMinutes),
        value: parseFloat(p.value)
    }));

    return {
        labels: alignedData.map(p => new Date(p.ts).toLocaleString('it-IT', { day: '2-digit', month: '2-digit', hour: '2-digit', minute: '2-digit' })),
        data: alignedData.map(p => p.value),
        raw: alignedData // Teniamo i dati con ts arrotondato per le aggregazioni
    };
}

function aggregateMean(allSeries, roundMinutes = 15) {
    const grouped = {};
    allSeries.forEach(s => {
        (s || []).forEach(p => {
            const rTs = roundTs(p.ts, roundMinutes);
            if (!grouped[rTs]) grouped[rTs] = [];
            grouped[rTs].push(parseFloat(p.value));
        });
    });

    const sortedTs = Object.keys(grouped).map(Number).sort((a, b) => a - b);
    const data = sortedTs.map(ts => {
        const vals = grouped[ts];
        return vals.reduce((a, b) => a + b, 0) / vals.length;
    });
    const labels = sortedTs.map(ts => new Date(ts).toLocaleString('it-IT', { day: '2-digit', month: '2-digit', hour: '2-digit', minute: '2-digit' }));
    return { labels, data, rawTs: sortedTs };
}

function aggregateSum(allSeries, roundMinutes = 15) {
    const grouped = {};
    allSeries.forEach(s => {
        (s || []).forEach(p => {
            const rTs = roundTs(p.ts, roundMinutes);
            if (!grouped[rTs]) grouped[rTs] = [];
            grouped[rTs].push(parseFloat(p.value));
        });
    });

    const sortedTs = Object.keys(grouped).map(Number).sort((a, b) => a - b);
    const data = sortedTs.map(ts => {
        const vals = grouped[ts];
        return vals.reduce((a, b) => a + b, 0);
    });
    const labels = sortedTs.map(ts => new Date(ts).toLocaleString('it-IT', { day: '2-digit', month: '2-digit', hour: '2-digit', minute: '2-digit' }));
    return { labels, data };
}

// ─────────────────────────────────────────────
// GRAFICI PANORAMICA
// ─────────────────────────────────────────────

const overviewCharts = {};
const HIVE_COLORS = ['#fbbf24', '#60a5fa', '#34d399', '#f87171', '#a78bfa'];

function initOverviewCharts(allTelemetries) {
    const commonOptions = {
        responsive: true,
        maintainAspectRatio: false,
        animation: false,
        scales: {
            x: {grid: {display: false}, ticks: {maxTicksLimit: 8, maxRotation: 0}}
        },
        plugins: {
            zoom: zoomConfig // <--- Aggiunto qui
        }
    };

    // 1. Temperatura per arnia + linea media
    {
        const datasets = allTelemetries
            .map((tel, i) => {
                const hive = hivesData[i];
                const {labels, data} = parseSeries(tel ? tel.tempIn : null);
                return {
                    label: hive ? hive.name : `Arnia ${i + 1}`,
                    data, _labels: labels,
                    borderColor: HIVE_COLORS[i % HIVE_COLORS.length],
                    backgroundColor: 'transparent',
                    tension: 0.4, borderWidth: 1.5, pointRadius: 2
                };
            })
            .filter(ds => ds.data.length > 0);

        const avgSeries = aggregateMean(allTelemetries.map(t => t ? t.tempIn : null));
        if (avgSeries.data.length > 0) {
            datasets.push({
                label: 'Media', data: avgSeries.data,
                borderColor: '#ffffff', backgroundColor: 'rgba(255,255,255,0.06)',
                fill: true, tension: 0.4, borderWidth: 2.5, pointRadius: 0,
                borderDash: [4, 3]
            });
        }
        const labels = avgSeries.labels.length > 0 ? avgSeries.labels : (datasets[0]?._labels || []);
        if (overviewCharts.temp) overviewCharts.temp.destroy();
        overviewCharts.temp = new Chart(document.getElementById('tempChart'), {
            type: 'line',
            data: {labels, datasets},
            options: {
                ...commonOptions,
                plugins: {
                    ...commonOptions.plugins, // Mantiene lo zoom
                    legend: {display: true, labels: {boxWidth: 10, font: {size: 11}}}
                }
            }
        });
    }

    // 2. Umidità media
    {
        const avg = aggregateMean(allTelemetries.map(t => t ? t.humidity : null));
        if (overviewCharts.hum) overviewCharts.hum.destroy();
        overviewCharts.hum = new Chart(document.getElementById('humidityChart'), {
            type: 'line',
            data: {labels: avg.labels, datasets: [{
                    label: 'Umidità Media %', data: avg.data,
                    borderColor: '#10b981', backgroundColor: 'rgba(16,185,129,0.1)',
                    fill: true, tension: 0.4, pointRadius: 2
                }]},
            options: {
                ...commonOptions,
                plugins: {
                    ...commonOptions.plugins, // AGGIUNTO: mantiene lo zoom
                    legend: {display: false}
                }
            }
        });
    }

    // 3. Peso totale
    {
        const total = aggregateSum(allTelemetries.map(t => t ? t.honeyWeightKg : null));
        if (overviewCharts.honey) overviewCharts.honey.destroy();
        overviewCharts.honey = new Chart(document.getElementById('honeyChart'), {
            type: 'line',
            data: {labels: total.labels, datasets: [{
                    label: 'Peso Totale (kg)', data: total.data,
                    borderColor: '#fbbf24', backgroundColor: 'rgba(251,191,36,0.1)',
                    fill: true, tension: 0.4, pointRadius: 2
                }]},
            options: {
                ...commonOptions,
                plugins: {
                    ...commonOptions.plugins, // AGGIUNTO: mantiene lo zoom
                    legend: {display: false}
                }
            }
        });
    }

    // 4. Frequenza picco media
    {
        const avg = aggregateMean(allTelemetries.map(t => t ? t.peakFreq : null));
        if (overviewCharts.freq) overviewCharts.freq.destroy();
        overviewCharts.freq = new Chart(document.getElementById('soundChart'), {
            type: 'line',
            data: {labels: avg.labels, datasets: [{
                    label: 'Freq. Media (Hz)', data: avg.data,
                    borderColor: '#a78bfa', backgroundColor: 'rgba(167,139,250,0.1)',
                    fill: true, tension: 0.4, pointRadius: 2
                }]},
            options: {
                ...commonOptions,
                plugins: {
                    ...commonOptions.plugins, // AGGIUNTO: mantiene lo zoom
                    legend: {display: false}
                },
                scales: {
                    x: {grid: {display: false}, ticks: {maxTicksLimit: 8, maxRotation: 0}},
                    y: {suggestedMin: 150, suggestedMax: 600, ticks: {callback: v => v + ' Hz'}}
                }
            }
        });
    }
}

function initOverviewChartsMock() {
    const ora = Date.now(), h = 3600000;
    const mockTelemetries = hivesData.map(hive => ({
        tempIn:        Array.from({length: 8}, (_, i) => ({ts: ora - (7 - i) * h, value: parseFloat(hive.t)        + (Math.random() - 0.5) * 2})),
        humidity:      Array.from({length: 8}, (_, i) => ({ts: ora - (7 - i) * h, value: parseFloat(hive.h)        + (Math.random() - 0.5) * 4})),
        honeyWeightKg: Array.from({length: 8}, (_, i) => ({ts: ora - (7 - i) * h, value: parseFloat(hive.w)        + (Math.random() - 0.5) * 0.5})),
        peakFreq:      Array.from({length: 8}, (_, i) => ({ts: ora - (7 - i) * h, value: parseFloat(hive.peakFreq) + (Math.random() - 0.5) * 20}))
    }));
    initOverviewCharts(mockTelemetries);
}

async function loadOverviewCharts(interval = '24h') {
    try {
        const telemetries = await Promise.all(
            hivesData.map(hive => tbGetTelemetry(hive.id, interval).catch(() => null))
        );
        initOverviewCharts(telemetries);
    } catch (err) {
        console.error('Errore caricamento grafici panoramica:', err);
    }
}

// ─────────────────────────────────────────────
// GRAFICI ANALISI AVANZATA
// ─────────────────────────────────────────────

const analysisCharts = {};

/** Coefficiente di determinazione R² tra due array numerici. */
function computeR2(xArr, yArr) {
    const n = Math.min(xArr.length, yArr.length);
    if (n < 3) return null;
    const x = xArr.slice(0, n), y = yArr.slice(0, n);
    const mx = x.reduce((a, b) => a + b, 0) / n;
    const my = y.reduce((a, b) => a + b, 0) / n;
    const num = x.reduce((s, xi, i) => s + (xi - mx) * (y[i] - my), 0);
    const den = Math.sqrt(
        x.reduce((s, xi) => s + (xi - mx) ** 2, 0) *
        y.reduce((s, yi) => s + (yi - my) ** 2, 0)
    );
    if (den === 0) return null;
    return (num / den) ** 2;
}

/** Derivata discreta di una serie [{ts,value}]: Δvalue/Δt in unità/ora. */
function computeDerivative(series) {
    if (!series || series.length < 2) return [];
    const sorted = [...series].sort((a, b) => a.ts - b.ts);
    const result = [];
    for (let i = 1; i < sorted.length; i++) {
        const dtH = (sorted[i].ts - sorted[i - 1].ts) / 3600000;
        if (dtH <= 0) continue;
        const dv = parseFloat(sorted[i].value) - parseFloat(sorted[i - 1].value);
        result.push({ts: (sorted[i].ts + sorted[i - 1].ts) / 2, value: dv / dtH});
    }
    return result;
}

function initAnalysisCharts(allTelemetries) {
    const commonOptions = {
        responsive: true,
        maintainAspectRatio: false,
        animation: false,
        plugins: {
            legend: {display: false},
            zoom: zoomConfig // <--- Aggiunto qui
        },
        scales: {x: {grid: {display: false}, ticks: {maxTicksLimit: 8, maxRotation: 0}}}
    };

    // ── 1. SCATTER: Correlazione Temp Interna vs Esterna ─────────────────────
    {
        const pairs = [];
        allTelemetries.forEach(tel => {
            if (!tel || !tel.tempIn || !tel.tempOut) return;

            tel.tempIn.forEach(pin => {
                let closestPout = null;
                let minDiff = 30 * 60 * 1000; // Tolleranza: 30 minuti

                tel.tempOut.forEach(pout => {
                    const diff = Math.abs(pout.ts - pin.ts);
                    if (diff < minDiff) {
                        minDiff = diff;
                        closestPout = pout;
                    }
                });

                if (closestPout) {
                    pairs.push({x: parseFloat(closestPout.value), y: parseFloat(pin.value)});
                }
            });
        });

        const xValues = pairs.map(p => p.x);
        const yValues = pairs.map(p => p.y);
        const r2 = computeR2(xValues, yValues);

        // Aggiorna la scritta nella pagina HTML
        const r2El = document.getElementById('analysisR2');
        if (r2El) {
            r2El.innerText = r2 !== null ? `R² = ${r2.toFixed(3)}` : 'R² = N/A';
        }

        // Disegno del grafico
        if (analysisCharts.correlation) analysisCharts.correlation.destroy();
        analysisCharts.correlation = new Chart(document.getElementById('correlationChart'), {
            type: 'scatter',
            data: {
                datasets: [{
                    label: 'Temp In vs Out',
                    data: pairs,
                    backgroundColor: 'rgba(56, 189, 248, 0.6)',
                    borderColor: 'rgba(56, 189, 248, 1)',
                    pointRadius: 4,
                    pointHoverRadius: 6
                }]
            },
            options: {
                ...commonOptions,
                scales: {
                    ...commonOptions.scales,
                    x: {
                        title: { display: true, text: 'Temperatura Esterna (°C)', color: '#94a3b8' },
                        grid: { color: 'rgba(255,255,255,0.05)' }
                    },
                    y: {
                        title: { display: true, text: 'Temperatura Interna (°C)', color: '#94a3b8' },
                        grid: { color: 'rgba(255,255,255,0.05)' }
                    }
                }
            }
        });
    }

    // ── 2. Derivata temperatura media (°C/h) ─────────────────────────────────
    {
        const derivSeries = allTelemetries
            .map(tel => tel ? computeDerivative(tel.tempIn) : [])
            .filter(s => s.length > 0);
        const avgDeriv = aggregateMean(derivSeries);

        const barColors = avgDeriv.data.map(v =>
            v === null ? 'rgba(255,255,255,0.1)'
                : v > 0    ? 'rgba(251,191,36,0.75)'   // riscaldamento → ambra
                    :             'rgba(96,165,250,0.75)'   // raffreddamento → blu
        );

        if (analysisCharts.tempDeriv) analysisCharts.tempDeriv.destroy();
        analysisCharts.tempDeriv = new Chart(document.getElementById('derivative1Chart'), {
            type: 'bar',
            data: {
                labels: avgDeriv.labels,
                datasets: [{label: 'ΔTemp/h (°C/h)', data: avgDeriv.data, backgroundColor: barColors, borderRadius: 3}]
            },
            options: {
                ...commonOptions,
                scales: {
                    x: {grid: {display: false}, ticks: {maxTicksLimit: 8, maxRotation: 0}},
                    y: {ticks: {callback: v => v.toFixed(2) + ' °C/h'}}
                }
            }
        });
    }

    // ── 3. Derivata peso totale (kg/h) ────────────────────────────────────────
    {
        const derivSeries = allTelemetries
            .map(tel => tel ? computeDerivative(tel.honeyWeightKg) : [])
            .filter(s => s.length > 0);
        // Somma: rateo complessivo di tutto l'apiario
        const sumDeriv = aggregateSum(derivSeries);

        const barColors = sumDeriv.data.map(v =>
            v === null ? 'rgba(255,255,255,0.1)'
                : v >= 0   ? 'rgba(16,185,129,0.75)'   // produzione → verde
                    :             'rgba(239,68,68,0.75)'    // perdita → rosso
        );

        if (analysisCharts.weightDeriv) analysisCharts.weightDeriv.destroy();
        analysisCharts.weightDeriv = new Chart(document.getElementById('weightDerivativeChart'), {
            type: 'bar',
            data: {
                labels: sumDeriv.labels,
                datasets: [{label: 'ΔPeso/h (kg/h)', data: sumDeriv.data, backgroundColor: barColors, borderRadius: 3}]
            },
            options: {
                ...commonOptions,
                scales: {
                    x: {grid: {display: false}, ticks: {maxTicksLimit: 8, maxRotation: 0}},
                    y: {ticks: {callback: v => v.toFixed(3) + ' kg/h'}}
                }
            }
        });
    }
}

function initAnalysisChartsMock() {
    const ora = Date.now(), h = 3600000;
    const mockTelemetries = hivesData.map(hive => ({
        tempIn:        Array.from({length: 12}, (_, i) => ({ts: ora - (11 - i) * h, value: parseFloat(hive.t)    + (Math.random() - 0.5) * 2})),
        tempOut:       Array.from({length: 12}, (_, i) => ({ts: ora - (11 - i) * h, value: parseFloat(hive.tOut) + (Math.random() - 0.5) * 3})),
        honeyWeightKg: Array.from({length: 12}, (_, i) => ({ts: ora - (11 - i) * h, value: parseFloat(hive.w)    + i * 0.02 + (Math.random() - 0.5) * 0.05}))
    }));
    initAnalysisCharts(mockTelemetries);
}

async function loadAnalysisCharts(interval = '24h') {
    try {
        const telemetries = await Promise.all(
            hivesData.map(hive => tbGetTelemetry(hive.id, interval).catch(() => null))
        );
        initAnalysisCharts(telemetries);
    } catch (err) {
        console.error('Errore caricamento grafici analisi avanzata:', err);
    }
}

// ─────────────────────────────────────────────
// INIT & POLLING
// ─────────────────────────────────────────────

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
        initOverviewChartsMock();
        initAnalysisChartsMock();
        lucide.createIcons();
    } else {
        hivesData.forEach(hive => {
            hive.t = 0; hive.h = 0; hive.w = 0; hive.pct = 0;
            hive.tOut = 0; hive.peakFreq = 0; hive.status = 'offline';
        });
        alertsHistory = [];
        renderHives();
        renderHistory();
        lucide.createIcons();

        loadRealData();
        loadOverviewCharts('24h');
        loadAnalysisCharts('24h');
        setInterval(loadRealData, 30000);

        try {
            alertsHistory = await tbLoadAlarms();
            renderHistory();
            renderStats();
        } catch (err) {
            console.error("Errore caricamento allarmi", err);
        }
    }
});