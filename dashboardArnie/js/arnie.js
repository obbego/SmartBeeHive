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
        try {
            const deviceId = TB_DEVICES[hiveId];
            if (deviceId) {
                const telemetry = await tbGetTelemetry(deviceId);

                const tempVal = telemetry.temperature ? telemetry.temperature.slice(-1)[0].value : 0;
                const humVal = telemetry.humidity ? telemetry.humidity.slice(-1)[0].value : 0;
                const weightVal = telemetry.weight ? telemetry.weight.slice(-1)[0].value : 0;
                const tOutVal = telemetry.tempOut ? telemetry.tempOut.slice(-1)[0].value : 0;
                const pctVal = telemetry.honeyPct ? telemetry.honeyPct.slice(-1)[0].value : 0;

                document.getElementById('valTempIn').innerText = parseFloat(tempVal).toFixed(1) + '°C';
                document.getElementById('valHum').innerText = parseFloat(humVal).toFixed(1) + '%';
                document.getElementById('valWeight').innerText = parseFloat(weightVal).toFixed(1) + 'kg';
                document.getElementById('valTempOut').innerText = parseFloat(tOutVal).toFixed(1) + '°C';
                document.getElementById('barMiele').style.height = parseFloat(pctVal).toFixed(0) + '%';
                document.getElementById('valMiele').innerText = parseFloat(pctVal).toFixed(0) + '%';

                if (telemetry.temperature && telemetry.temperature.length > 0) {
                    const date = new Date(telemetry.temperature.slice(-1)[0].ts);
                    document.getElementById('lastUpdate').innerText = 'Ultimo dato: ' + date.toLocaleTimeString('it-IT', {hour: '2-digit', minute:'2-digit'});
                } else {
                    document.getElementById('lastUpdate').innerText = 'Ultimo dato: Non disponibile';
                }

                const semaforo = document.getElementById('statusSemaforo');
                if (tempVal == 0 && weightVal == 0 && humVal == 0) {
                    semaforo.className = 'status-alert instabile';
                    semaforo.innerHTML = '<i data-lucide="help-circle"></i> Valori a zero - Verificare sensori';
                } else if (tempVal > 40 || tempVal < -5) {
                    semaforo.className = 'status-alert allarme';
                    semaforo.innerHTML = '<i data-lucide="alert-triangle"></i> Allarme: Temperatura fuori soglia';
                } else {
                    semaforo.className = 'status-alert ottimale';
                    semaforo.innerHTML = '<i data-lucide="check-circle"></i> Dati Ricevuti: Tutto regolare';
                }
            } else {
                impostaZeri();
            }
        } catch (error) {
            impostaZeri();
        }
        historyDiv.innerHTML = '<div class="text-center text-muted py-4" style="font-size: 14px;">Nessun allarme registrato.</div>';
    }

    initDetailCharts();
    lucide.createIcons();
});

function initDetailCharts() {
    const commonOptions = {
        responsive: true, maintainAspectRatio: false, plugins: { legend: { display: false } },
        scales: { x: { grid: { display: false }, ticks: { font: { size: 11 } } }, y: { border: { dash: [4, 4] }, ticks: { font: { size: 11 } } } }
    };

    new Chart(document.getElementById('tempInOutChart').getContext('2d'), { type: 'line', data: { labels: ['00', '04', '08', '12', '16', '20', '24'], datasets: [{ label: 'Temp In', data: [34.5, 35, 35.8, 36.2, 35.9, 35.2, 34.8], borderColor: '#fbbf24', backgroundColor: 'rgba(251,191,36,0.1)', fill: true, tension: 0.4 }, { label: 'Temp Out', data: [18, 19, 23, 27, 26, 22, 20], borderColor: '#60a5fa', fill: false, tension: 0.4 }] }, options: { ...commonOptions, plugins: { legend: { display: true, labels: { color: '#94a3b8' } } } } });
    new Chart(document.getElementById('fftChart').getContext('2d'), { type: 'line', data: { labels: ['100', '200', '250', '300', '400', '500'], datasets: [{ data: [20, 45, 90, 120, 60, 30], borderColor: '#fbbf24', backgroundColor: 'rgba(251, 191, 36, 0.1)', fill: true, tension: 0.4 }] }, options: commonOptions });
    new Chart(document.getElementById('weightFlowChart').getContext('2d'), { type: 'bar', data: { labels: ['08:00', '10:00', '12:00', '14:00', '16:00'], datasets: [{ data: [0.1, 0.4, 0.8, -0.2, -0.5], backgroundColor: '#10b981', borderRadius: 4 }] }, options: commonOptions });
    new Chart(document.getElementById('humidityChart').getContext('2d'), { type: 'line', data: { labels: ['08:00', '10:00', '12:00', '14:00', '16:00', '18:00', '20:00'], datasets: [{ data: [55, 58, 60, 62, 59, 57, 55], borderColor: '#3b82f6', backgroundColor: 'rgba(59,130,246,0.1)', fill: true, tension: 0.4 }] }, options: { ...commonOptions, scales: { ...commonOptions.scales, y: { min: 0, max: 100 } } } });
}