Chart.defaults.color = '#94a3b8';
Chart.defaults.borderColor = 'rgba(255, 255, 255, 0.05)';
Chart.defaults.font.family = "'Inter', sans-serif";

const mockAlerts = [
    { text: "Temperatura fuori soglia", date: "10 min fa", status: "open" },
    { text: "Variazione peso anomala", date: "02/03/2026 14:32", status: "closed" },
    { text: "Batteria sensore quasi scarica", date: "03/03/2026 07:48", status: "closed" },
    { text: "Sensore temperatura non risponde", date: "04/03/2026 11:20", status: "open" }
];

// ─────────────────────────────────────────────
// LOGICA COLORI PER I 4 RIQUADRI METRICHE
// Grigio = ok, Arancio = attenzione, Rosso = allarme
// ─────────────────────────────────────────────

function applyMetricColors(tempIn, hum, weight, peakFreq) {
    setMetricColor('valTempIn',   getTempInColor(tempIn));
    setMetricColor('valHum',      getHumColor(hum));
    setMetricColor('valWeight',   getWeightColor(weight));
    setMetricColor('valPeakFreq', getFreqColor(peakFreq));
}

function setMetricColor(elementId, color) {
    const el = document.getElementById(elementId);
    if (!el) return;
    el.style.color = color;
    el.style.fontWeight = color ? '700' : '';
}

// Temperatura interna (°C)
// Ottimale api: 33–38 °C
function getTempInColor(t) {
    const v = parseFloat(t);
    if (isNaN(v) || v <= 0) return '';           // dato assente → default
    if (v >= 38.5 || v < 30) return 'var(--danger)';   // allarme
    if (v >= 37.5 || v < 32) return 'var(--warning)';  // attenzione
    return '';                                           // ok → grigio default
}

// Umidità interna (%)
// Ottimale: 40–65%
function getHumColor(h) {
    const v = parseFloat(h);
    if (isNaN(v) || v <= 0) return '';
    if (v > 75 || v < 25) return 'var(--danger)';
    if (v > 65 || v < 35) return 'var(--warning)';
    return '';
}

// Peso (kg) — segnala solo valori anomali bassi (colonia in calo)
// Soglie orientative; un'arnia sana pesa 20–60 kg
function getWeightColor(w) {
    const v = parseFloat(w);
    if (isNaN(v) || v <= 0) return '';
    if (v < 15) return 'var(--danger)';   // colonia molto debole / problemi
    if (v < 25) return 'var(--warning)';  // sotto il normale
    return '';
}

// Frequenza di picco (Hz)
// Normale: 220–270 Hz
function getFreqColor(f) {
    const v = parseFloat(f);
    if (isNaN(v) || v <= 0) return '';
    if (v >= 380) return 'var(--danger)';   // sciamatura / allarme
    if (v >= 270) return 'var(--warning)';  // stress / attenzione
    return '';
}

// ─────────────────────────────────────────────

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

    const params = new URLSearchParams(window.location.search);
    const hiveId = parseInt(params.get('id')) || 1;
    const hive = typeof hivesData !== 'undefined' ? (hivesData.find(h => h.id === hiveId) || hivesData[0]) : null;
    document.getElementById('hiveName').innerText = hive ? hive.name : `Arnia 0${hiveId}`;

    const historyDiv = document.getElementById('localHistory');

    const impostaZeri = () => {
        document.getElementById('valTempIn').innerText   = '0°C';
        document.getElementById('valPeakFreq').innerText = '0 Hz';
        document.getElementById('valHum').innerText      = '0%';
        document.getElementById('valWeight').innerText   = '0kg';
        document.getElementById('barMiele').style.height = '0%';
        document.getElementById('valMiele').innerText    = '0%';
        document.getElementById('valR2').innerText       = 'N/D';

        //document.getElementById('lastUpdate').innerText  = 'Ultimo dato: ND';
        const semaforo = document.getElementById('statusSemaforo');
        semaforo.className = 'status-alert allarme';
        semaforo.innerHTML = '<i data-lucide="help-circle"></i> Sensori non configurati o offline';
        historyDiv.innerHTML = '<div class="text-center text-muted py-4" style="font-size: 14px;">Nessun allarme.</div>';
    };

    if (isMockMode) {
        document.getElementById('valTempIn').innerText   = hive.t + '°C';
        document.getElementById('valPeakFreq').innerText = hive.peakFreq + ' Hz';
        document.getElementById('valWeight').innerText   = hive.w + 'kg';
        document.getElementById('valHum').innerText      = hive.h + '%';
        document.getElementById('barMiele').style.height = hive.pct + '%';
        document.getElementById('valMiele').innerText    = hive.pct + '%';
        //document.getElementById('lastUpdate').innerText  = 'Ultimo aggiornamento: ' + hive.lastUpdate;

        // Colora tutti e 4 i riquadri
        applyMetricColors(hive.t, hive.h, hive.w, hive.peakFreq);

        const semaforo = document.getElementById('statusSemaforo');
        if (hive.status === 'green') {
            semaforo.className = 'status-alert ottimale';
            semaforo.innerHTML = '<i data-lucide="check-circle"></i> Dati ricevuti: tutto regolare (Ultimo aggiornamento dati: ' + hive.lastUpdate + ')';
        } else if (hive.status === 'yellow') {
            semaforo.className = 'status-alert instabile';
            semaforo.innerHTML = '<i data-lucide="help-circle"></i> Attenzione: ultimo aggiornamento superiore a 24 ore (' + hive.lastUpdate + ')';
        } else if (hive.status === 'red' || hive.status === 'offline') {
            semaforo.className = 'status-alert allarme';
            semaforo.innerHTML = '<i data-lucide="alert-triangle"></i> Arnia non disponibile: uno o più dispositivi sono offline';
        }

        const ora = Date.now();
        const unOra = 3600000;
        const mockTelemetry = {
            tempIn:   [
                { ts: ora - 4*unOra, value: hive.t - 0.5 },
                { ts: ora - 3*unOra, value: hive.t - 0.2 },
                { ts: ora - 2*unOra, value: hive.t + 0.3 },
                { ts: ora - unOra,   value: hive.t + 0.1 },
                { ts: ora,           value: hive.t }
            ],
            tempOut:  [
                { ts: ora - 4*unOra, value: hive.tOut - 2 },
                { ts: ora - 3*unOra, value: hive.tOut - 1 },
                { ts: ora - 2*unOra, value: hive.tOut + 1 },
                { ts: ora - unOra,   value: hive.tOut + 0.5 },
                { ts: ora,           value: hive.tOut }
            ],
            humidity: [
                { ts: ora - 4*unOra, value: hive.h + 2 },
                { ts: ora - 2*unOra, value: hive.h - 1 },
                { ts: ora,           value: hive.h }
            ],
            honeyWeightKg:   [
                { ts: ora - 4*unOra, value: hive.w - 0.3 },
                { ts: ora - 3*unOra, value: hive.w - 0.2 },
                { ts: ora - 2*unOra, value: hive.w - 0.1 },
                { ts: ora,           value: hive.w }
            ],
            honeyPct: [{ ts: ora, value: hive.pct }],
            peakFreq: [
                { ts: ora - 4*unOra, value: hive.peakFreq - 10 },
                { ts: ora - 3*unOra, value: hive.peakFreq - 5  },
                { ts: ora - 2*unOra, value: hive.peakFreq + 8  },
                { ts: ora - unOra,   value: hive.peakFreq + 3  },
                { ts: ora,           value: hive.peakFreq }
            ]
        };

        initDetailCharts(mockTelemetry);
        updateR2(mockTelemetry);

        historyDiv.innerHTML = "";
        mockAlerts.forEach(alert => {
            historyDiv.innerHTML += `
            <div class="history-item px-0 mb-3">
                <div>
                    <div style="font-weight:600; color:white;">${alert.text}</div>
                    <div style="font-size:12px; color:var(--text-muted);">${alert.date}</div>
                </div>
                <span class="tag ${alert.status}">${alert.status === "open" ? "APERTO" : "RISOLTO"}</span>
            </div>`;
        });

    } else {
        // --- MODALITA' REALE ---
        let telemetry = null;
        try {
            telemetry = await tbGetTelemetry(hiveId, '24h');

            // Verifica se esistono dati utili nelle ultime 24h
            const hasData24h = telemetry && telemetry.tempIn && telemetry.tempIn.length > 0;

            if (!hasData24h) {
                // Nessun dato nelle ultime 24h, recuperiamo l'ultimo dato assoluto ('latest')
                try {
                    const latestTelemetry = await tbGetTelemetry(hiveId, 'latest');
                    if (latestTelemetry && latestTelemetry.tempIn && latestTelemetry.tempIn.length > 0) {
                        // Sostituiamo per far disegnare l'interfaccia (ed il puntatore nel grafico) usando quest'ultimo dato
                        telemetry = latestTelemetry;
                        telemetry.is_stale = true; // Triggera l'allarme giallo "dati vecchi"
                    }
                } catch (e) {
                    console.error("Impossibile recuperare i dati assoluti (latest)", e);
                }
            }

            if (telemetry && Object.keys(telemetry).length > 0 && telemetry.tempIn && telemetry.tempIn.length > 0) {
                ['tempIn', 'tempOut', 'humidity', 'honeyWeightKg', 'honeyPct', 'peakFreq'].forEach(key => {
                    if (telemetry[key] && telemetry[key].length > 0) {
                        telemetry[key].sort((a, b) => a.ts - b.ts);
                    }
                });

                const temInVal  = telemetry.tempIn   ? telemetry.tempIn.slice(-1)[0].value   : 0;
                const humVal    = telemetry.humidity  ? telemetry.humidity.slice(-1)[0].value  : 0;
                const weightVal = telemetry.honeyWeightKg ? telemetry.honeyWeightKg.slice(-1)[0].value : 0;
                const pctVal    = telemetry.honeyPct  ? telemetry.honeyPct.slice(-1)[0].value  : 0;
                const freqVal   = telemetry.peakFreq  ? telemetry.peakFreq.slice(-1)[0].value  : 0;

                document.getElementById('valTempIn').innerText   = parseFloat(temInVal).toFixed(1)  + '°C';
                document.getElementById('valHum').innerText      = parseFloat(humVal).toFixed(1)    + '%';
                document.getElementById('valWeight').innerText   = parseFloat(weightVal).toFixed(1) + 'kg';
                document.getElementById('valPeakFreq').innerText = parseFloat(freqVal).toFixed(0)   + ' Hz';
                document.getElementById('barMiele').style.height = parseFloat(pctVal).toFixed(0)    + '%';
                document.getElementById('valMiele').innerText    = parseFloat(pctVal).toFixed(0)    + '%';

                applyMetricColors(temInVal, humVal, weightVal, freqVal);

                /*
                if (telemetry.tempIn && telemetry.tempIn.length > 0) {
                    const date = new Date(telemetry.tempIn.slice(-1)[0].ts);
                    document.getElementById('lastUpdate').innerText = 'Ultimo dato: ' + date.toLocaleTimeString('it-IT', { hour: '2-digit', minute: '2-digit' });
                } else {
                    document.getElementById('lastUpdate').innerText = 'Ultimo dato: Non disponibile';
                }

                const semaforo = document.getElementById('statusSemaforo');
                if (temInVal == 0 && weightVal == 0 && humVal == 0) {
                    semaforo.className = 'status-alert instabile';
                    semaforo.innerHTML = '<i data-lucide="help-circle"></i> Valori a zero - Verificare sensori';
                } else if (temInVal > 40 || temInVal < -5) {
                    semaforo.className = 'status-alert allarme';
                    semaforo.innerHTML = '<i data-lucide="alert-triangle"></i> Allarme: Temperatura fuori soglia';
                } else {
                    semaforo.className = 'status-alert ottimale';
                    semaforo.innerHTML = '<i data-lucide="check-circle"></i> Dati Ricevuti: Tutto regolare';
                }

                initDetailCharts(telemetry);
                */

                // --- AGGIORNAMENTO TESTO DATA ---
                let dataFormattata = telemetry.last_ts_human || 'Data non disponibile';
                if (dataFormattata === 'Errore' || dataFormattata === 'Mai') {
                    dataFormattata = 'Nessun dato registrato';
                }

                // --- GESTIONE SEMAFORO E WARNING ---
                const semaforo = document.getElementById('statusSemaforo');

                if (telemetry.is_stale) {
                    semaforo.className = 'status-alert instabile';
                    semaforo.innerHTML = `<i data-lucide="help-circle"></i> Attenzione: <br> Ultimo aggiornamento superiore a 24 ore (${dataFormattata})`;
                }
                else if (temInVal == 0 && weightVal == 0 && humVal == 0) {
                    semaforo.className = 'status-alert allarme';
                    semaforo.innerHTML = `<i data-lucide="alert-triangle"></i> Arnia non disponibile: <br> uno o più dispositivi sono offline`;
                }
                else {
                    semaforo.className = 'status-alert ottimale';
                    semaforo.innerHTML = `<i data-lucide="check-circle"></i> Dati ricevuti: <br> Tutto regolare (Ultimo aggiornamento dati: ${dataFormattata})`;
                }

                // --- AGGIORNAMENTO R2 E GRAFICI ---
                updateR2(telemetry);

                if (window.lucide) lucide.createIcons();
                initDetailCharts(telemetry);
            } else {
                impostaZeri();
            }
        } catch (error) {
            impostaZeri();
        }
        try {
            const allAlarms = await tbLoadAlarms();
            const hiveName = hive ? hive.name : `Arnia 0${hiveId}`;
            const hiveAlarms = allAlarms.filter(a => a.hive === hiveName);

            if (hiveAlarms.length === 0) {
                historyDiv.innerHTML = '<div class="text-center text-muted py-4" style="font-size: 14px;">Nessun allarme registrato.</div>';
            } else {
                historyDiv.innerHTML = hiveAlarms.map(alarm => {
                    const statusMap = {
                        system: { cls: 'tag-system', label: '⚙ DA GESTIRE' },
                        open:   { cls: 'tag open',   label: '● APERTO'     },
                        closed: { cls: 'tag closed', label: '✓ RISOLTO'    },
                    };
                    const s = statusMap[alarm.status] || statusMap.system;
                    return `
            <div class="history-item px-0">
                <div>
                    <div style="font-weight:600; color:white;">${alarm.msg}</div>
                    <div style="font-size:12px; color:var(--text-muted);">${alarm.time}</div>
                </div>
                <span class="${s.cls}">${s.label}</span>
            </div>`;
                }).join('');
            }
        } catch (err) {
            historyDiv.innerHTML = '<div class="text-center text-muted py-4" style="font-size: 14px;">Errore caricamento allarmi.</div>';
        }
    }

    lucide.createIcons();

    // --- SELETTORE TEMPORALE ---
    const timeTabs = document.querySelectorAll('#timeRangeSelector .tab-btn');
    timeTabs.forEach(tab => {
        tab.addEventListener('click', async (e) => {
            timeTabs.forEach(t => t.classList.remove('active'));
            tab.classList.add('active');
            if (isMockMode) return;

            let selectedInterval = tab.getAttribute('data-value');

            // FIX: Se il pulsante è impostato su 'latest' o è vuoto, forziamo '24h'
            // per garantire che i grafici ricevano lo storico temporale e non un solo punto.
            if (!selectedInterval || selectedInterval === 'latest') {
                selectedInterval = '24h';
            }

            try {
                ['tempInOutChart', 'humidityChart', 'weightFlowChart', 'peakFreqChart'].forEach(id => {
                    document.getElementById(id).style.opacity = '0.5';
                });
                const storicTelemetry = await tbGetTelemetry(hiveId, selectedInterval);
                if (storicTelemetry && Object.keys(storicTelemetry).length > 0) {
                    initDetailCharts(storicTelemetry);
                }
            } catch (error) {
                console.error("Errore nel recupero dei dati storici:", error);
            } finally {
                ['tempInOutChart', 'humidityChart', 'weightFlowChart', 'peakFreqChart'].forEach(id => {
                    document.getElementById(id).style.opacity = '1';
                });
            }
        });
    });
});

let charts = {};

function initDetailCharts(telemetry) {
    /**
     * Funzione interna per gestire lo stato visivo "Nessun Dato".
     * Aggiunge o rimuove la classe CSS .no-data al contenitore del grafico.
     */

    const warningVisible = document.body.innerText.includes("Ultimo aggiornamento superiore a 24 ore");
    const currentRange = document.querySelector('.tab-btn.active').dataset.value;

    const checkEmpty = (dataArray, canvasId) => {
        const canvas = document.getElementById(canvasId);
        if (!canvas) return true;
        const container = canvas.closest('.chart-box');

        // Controlliamo se l'array esiste e ha elementi
        const isEmpty = !dataArray || dataArray.length === 0;

        if (isEmpty) {
            container.classList.add('no-data');
        } else {
            container.classList.remove('no-data');
        }
        return isEmpty;
    };

    // Configurazione comune per i grafici
    const optionsWithLegend = {
        responsive: true,
        maintainAspectRatio: false,
        plugins: {
            legend: { display: true }
        }
    };

    const optionsNoLegend = {
        responsive: true,
        maintainAspectRatio: false,
        plugins: {
            legend: { display: false }
        }
    };

    // 1. GRAFICO: Temperatura Interna vs Esterna
    const tempIn = parseTelemetrySeries(telemetry.tempIn);
    const tempOut = parseTelemetrySeries(telemetry.tempOut);
    checkEmpty(tempIn.data, 'tempInOutChart');

    if (charts.temp) charts.temp.destroy();
    charts.temp = new Chart(document.getElementById('tempInOutChart'), {
        type: 'line',
        data: {
            labels: tempIn.labels,
            datasets: [
                { label: 'Temp In',  data: tempIn.data,  borderColor: '#fbbf24', backgroundColor: 'rgba(251, 191, 36, 0.1)', fill: true },
                { label: 'Temp Out', data: tempOut.data, borderColor: '#60a5fa', fill: false }
            ]
        },
        options: optionsWithLegend
    });

    // 2. GRAFICO: Umidità Interna
    const hum = parseTelemetrySeries(telemetry.humidity);
    checkEmpty(hum.data, 'humidityChart');

    if (charts.hum) charts.hum.destroy();
    charts.hum = new Chart(document.getElementById('humidityChart'), {
        type: 'line',
        data: {
            labels: hum.labels,
            datasets: [{ label: 'Umidità', data: hum.data, borderColor: '#3b82f6', backgroundColor: 'rgba(59, 130, 246, 0.1)', fill: true }]
        },
        options: optionsNoLegend
    });

    // 3. GRAFICO: Variazione Peso (Barre)
    const weight = parseTelemetrySeries(telemetry.honeyWeightKg);
    checkEmpty(weight.data, 'weightFlowChart');

    if (charts.weight) charts.weight.destroy();
    charts.weight = new Chart(document.getElementById('weightFlowChart'), {
        type: 'bar',
        data: {
            labels: weight.labels,
            datasets: [{ label: 'Peso', data: weight.data, backgroundColor: '#10b981' }]
        },
        options: optionsNoLegend
    });

    // 4. GRAFICO: Frequenza Picco
    const freq = parseTelemetrySeries(telemetry.peakFreq);
    checkEmpty(freq.data, 'peakFreqChart');

    if (charts.freq) charts.freq.destroy();
    charts.freq = new Chart(document.getElementById('peakFreqChart'), {
        type: 'line',
        data: {
            labels: freq.labels,
            datasets: [{
                label: 'Frequenza Picco (Hz)',
                data: freq.data,
                borderColor: '#a78bfa',
                backgroundColor: 'rgba(167, 139, 250, 0.1)',
                fill: true,
                tension: 0.4
            }]
        },
        options: {
            ...optionsNoLegend,
            scales: {
                y: {
                    suggestedMin: 150,
                    suggestedMax: 600,
                    ticks: { callback: (val) => val + ' Hz' }
                }
            }
        }
    });
}

function parseTelemetrySeries(series) {
    if (!series || series.length === 0) return { labels: [], data: [] };
    series.sort((a, b) => a.ts - b.ts);
    const labels = series.map(p => {
        const d = new Date(p.ts);
        return d.toLocaleString('it-IT', { day: '2-digit', month: '2-digit', hour: '2-digit', minute: '2-digit' });
    });
    const data = series.map(p => p.value);
    return { labels, data };
}

// Funzione matematica per calcolare l'R² tra due array di dati appaiati
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

function updateR2(telemetry) {
    const r2El = document.getElementById('valR2');
    if (!r2El) return;

    // Se non ci sono dati o c'è solo l'ultimo dato (arnia offline)
    if (!telemetry || !telemetry.tempIn || telemetry.tempIn.length < 3) {
        r2El.innerText = 'Dati insuff.'; // Invece di N/D, più chiaro
        return;
    }

    const pairs = [];
    const MAX_TIME_DIFF = 60 * 60 * 1000; // Alziamo a 1 ora per sicurezza

    telemetry.tempIn.forEach(pin => {
        let closestOut = null;
        let minDiff = Infinity;

        telemetry.tempOut.forEach(pout => {
            const diff = Math.abs(pout.ts - pin.ts);
            if (diff < minDiff && diff <= MAX_TIME_DIFF) {
                minDiff = diff;
                closestOut = pout;
            }
        });

        if (closestOut) {
            pairs.push({ x: parseFloat(closestOut.value), y: parseFloat(pin.value) });
        }
    });

    if (pairs.length >= 3) {
        const r2 = computeR2(pairs.map(p => p.x), pairs.map(p => p.y));
        r2El.innerText = r2 !== null ? r2.toFixed(2) : 'N/D';
    } else {
        r2El.innerText = 'Sync in corso...';
    }
}