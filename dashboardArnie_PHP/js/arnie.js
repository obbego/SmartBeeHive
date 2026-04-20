Chart.defaults.color = '#94a3b8';
Chart.defaults.borderColor = 'rgba(255, 255, 255, 0.05)';
Chart.defaults.font.family = "'Inter', sans-serif";
const mockAlerts = [
    { text: "Temperatura fuori soglia", date: "10 min fa", status: "open" },
    { text: "Variazione peso anomala", date: "02/03/2026 14:32", status: "closed" },
    { text: "Batteria sensore quasi scarica", date: "03/03/2026 07:48", status: "closed" },
    { text: "Sensore temperatura non risponde", date: "04/03/2026 11:20", status: "open" }
];
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
        document.getElementById('valTempIn').innerText = '0°C';
        document.getElementById('valTempOut').innerText = '0°C';
        document.getElementById('valHum').innerText = '0%';
        document.getElementById('valWeight').innerText = '0kg';
        document.getElementById('barMiele').style.height = '0%';
        document.getElementById('valMiele').innerText = '0%';
        document.getElementById('lastUpdate').innerText = 'Ultimo dato: ND';
        const semaforo = document.getElementById('statusSemaforo');
        semaforo.className = 'status-alert instabile';
        semaforo.innerHTML = '<i data-lucide="help-circle"></i> Sensori non configurati o offline';
        historyDiv.innerHTML = '<div class="text-center text-muted py-4" style="font-size: 14px;">Nessun allarme.</div>';
    };
    if (isMockMode) {
        // --- MODALITA' DEMO ---
        document.getElementById('valTempIn').innerText = hive.t + '°C';
        document.getElementById('valTempOut').innerText = hive.tOut + '°C';
        document.getElementById('valWeight').innerText = hive.w + 'kg';
        document.getElementById('valHum').innerText = hive.h + '%';
        document.getElementById('barMiele').style.height = hive.pct + '%';
        document.getElementById('valMiele').innerText = hive.pct + '%';
        document.getElementById('lastUpdate').innerText = 'Ultimo aggiornamento: ' + hive.lastUpdate;
        const semaforo = document.getElementById('statusSemaforo');
        if (hive.status === 'green') {
            semaforo.className = 'status-alert ottimale';
            semaforo.innerHTML = '<i data-lucide="check-circle"></i> Stato Ottimale: Tutto sotto controllo';
        } else if (hive.status === 'yellow') {
            semaforo.className = 'status-alert instabile';
            semaforo.innerHTML = '<i data-lucide="help-circle"></i> Stato Instabile: Monitorare variazioni';
        } else if (hive.status === 'red') {
            semaforo.className = 'status-alert allarme';
            semaforo.innerHTML = '<i data-lucide="alert-triangle"></i> Allarme: Intervento richiesto';
        }
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
            // Chiamiamo il nostro PHP passandogli l'ID dell'arnia
            telemetry = await tbGetTelemetry(hiveId);
            if (telemetry && Object.keys(telemetry).length > 0) {
                const temInVal = telemetry.tempIn ? telemetry.tempIn.slice(-1)[0].value : 0;
                const humVal = telemetry.humidity ? telemetry.humidity.slice(-1)[0].value : 0;
                const weightVal = telemetry.weight ? telemetry.weight.slice(-1)[0].value : 0;
                const tOutVal = telemetry.tempOut ? telemetry.tempOut.slice(-1)[0].value : 0;
                const pctVal = telemetry.honeyPct ? telemetry.honeyPct.slice(-1)[0].value : 0;
                document.getElementById('valTempIn').innerText = parseFloat(temInVal).toFixed(1) + '°C';
                document.getElementById('valHum').innerText = parseFloat(humVal).toFixed(1) + '%';
                document.getElementById('valWeight').innerText = parseFloat(weightVal).toFixed(1) + 'kg';
                document.getElementById('valTempOut').innerText = parseFloat(tOutVal).toFixed(1) + '°C';
                document.getElementById('barMiele').style.height = parseFloat(pctVal).toFixed(0) + '%';
                document.getElementById('valMiele').innerText = parseFloat(pctVal).toFixed(0) + '%';
                if (telemetry.tempIn && telemetry.tempIn.length > 0) {
                    const date = new Date(telemetry.tempIn.slice(-1)[0].ts);
                    document.getElementById('lastUpdate').innerText = 'Ultimo dato: ' + date.toLocaleTimeString('it-IT', {hour: '2-digit', minute:'2-digit'});
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
                //inizializzi i grafici
                initDetailCharts(telemetry);
            } else {
                impostaZeri();
            }
        } catch (error) {
            impostaZeri();
        }
        historyDiv.innerHTML = '<div class="text-center text-muted py-4" style="font-size: 14px;">Nessun allarme registrato.</div>';
    }
    lucide.createIcons();

    // --- GESTIONE SELETTORE TEMPORALE GRAFICI (STILE DASHBOARD) ---
    // Nota: cerchiamo .tab-btn invece di .tab
    const timeTabs = document.querySelectorAll('#timeRangeSelector .tab-btn');

    timeTabs.forEach(tab => {
        tab.addEventListener('click', async (e) => {
            // 1. GESTIONE ESTETICA: Questo lo facciamo SEMPRE
            timeTabs.forEach(t => t.classList.remove('active'));
            tab.classList.add('active');

            // 2. BLOCCO DEMO: Se siamo in modalità demo, fermiamo qui le chiamate
            if (isMockMode) return;

            // 3. RECUPERO DATI REALI
            const selectedInterval = tab.getAttribute('data-value');

            try {
                // Feedback visivo (opacità) sui grafici
                document.getElementById('tempInOutChart').style.opacity = '0.5';
                document.getElementById('humidityChart').style.opacity = '0.5';
                document.getElementById('weightFlowChart').style.opacity = '0.5';

                // Richiesta dati storici
                const storicTelemetry = await tbGetTelemetry(hiveId, selectedInterval);

                if (storicTelemetry && Object.keys(storicTelemetry).length > 0) {
                    initDetailCharts(storicTelemetry); // Ridisegna i grafici
                }
            } catch (error) {
                console.error("Errore nel recupero dei dati storici:", error);
            } finally {
                // Ripristina l'opacità
                document.getElementById('tempInOutChart').style.opacity = '1';
                document.getElementById('humidityChart').style.opacity = '1';
                document.getElementById('weightFlowChart').style.opacity = '1';
            }
        });
    });

});
let charts = {};
function initDetailCharts(telemetry) {
    const commonOptions = {
        responsive: true,
        maintainAspectRatio: false,
        plugins: { legend: { display: true } },
    };
    // --- TEMPERATURA ---
    const tempIn = parseTelemetrySeries(telemetry.tempIn);
    const tempOut = parseTelemetrySeries(telemetry.tempOut);
    if (charts.temp) charts.temp.destroy();
    charts.temp = new Chart(document.getElementById('tempInOutChart'), {
        type: 'line',
        data: {
            labels: tempIn.labels,
            datasets: [
                {
                    label: 'Temp In',
                    data: tempIn.data,
                    borderColor: '#fbbf24',
                    fill: true
                },
                {
                    label: 'Temp Out',
                    data: tempOut.data,
                    borderColor: '#60a5fa',
                    fill: false
                }
            ]
        },
        options: commonOptions
    });
    // --- UMIDITÀ ---
    const hum = parseTelemetrySeries(telemetry.humidity);
    if (charts.hum) charts.hum.destroy();
    charts.hum = new Chart(document.getElementById('humidityChart'), {
        type: 'line',
        data: {
            labels: hum.labels,
            datasets: [{
                label: 'Umidità',
                data: hum.data,
                borderColor: '#3b82f6',
                fill: true
            }]
        },
        options: commonOptions
    });
    // --- PESO ---
    const weight = parseTelemetrySeries(telemetry.weight);
    if (charts.weight) charts.weight.destroy();
    charts.weight = new Chart(document.getElementById('weightFlowChart'), {
        type: 'bar',
        data: {
            labels: weight.labels,
            datasets: [{
                label: 'Peso',
                data: weight.data,
                backgroundColor: '#10b981'
            }]
        },
        options: commonOptions
    });
}
function parseTelemetrySeries(series) {
    if (!series || series.length === 0) return { labels: [], data: [] };

    // ORDINA PER TIMESTAMP
    series.sort((a, b) => a.ts - b.ts);

    const labels = series.map(p => {
        const d = new Date(p.ts);
        // MODIFICA: Usiamo toLocaleString per avere "GG/MM HH:mm"
        // Invece del vecchio toLocaleTimeString che dava solo l'ora
        return d.toLocaleString('it-IT', {
            day: '2-digit',
            month: '2-digit',
            hour: '2-digit',
            minute: '2-digit'
        });
    });

    const data = series.map(p => p.value);

    return { labels, data };
}