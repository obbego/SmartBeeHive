// Configurazione globale Chart.js (uniformata all'index)
Chart.defaults.color = '#94a3b8';
Chart.defaults.borderColor = 'rgba(255, 255, 255, 0.05)';
Chart.defaults.font.family = "'Inter', sans-serif";

document.addEventListener('DOMContentLoaded', () => {
    const params = new URLSearchParams(window.location.search);
    const hiveId = parseInt(params.get('id'));
    const hive = hivesData.find(h => h.id === hiveId) || hivesData[0];

    // Popolamento testi
    document.getElementById('hiveName').innerText = hive.name;
    document.getElementById('valTempIn').innerText = hive.t + '°C';
    document.getElementById('valTempOut').innerText = hive.tOut + '°C';
    document.getElementById('valWeight').innerText = hive.w + 'kg';
    document.getElementById('valHum').innerText = hive.h + '%';
    // ... sotto la riga di valHum
    document.getElementById('barMiele').style.height = hive.pct + '%';
    document.getElementById('valMiele').innerText = hive.pct + '%';
    document.getElementById('lastUpdate').innerText = 'Ultimo aggiornamento: ' + hive.lastUpdate;


    // Gestione Semaforo con classi CSS custom
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

    /// Popolamento Allarmi multipli
    const historyDiv = document.getElementById('localHistory');

    // Simulazione di più allarmi
    const alerts = [
        {
            text: "Temperatura fuori soglia",
            date: hive.lastUpdate,
            status: "open"
        },
        {
            text: "Variazione peso anomala",
            date: "02/03/2026 14:32",
            status: "closed"
        },
        {
            text: "Batteria sensore quasi scarica",
            date: "03/03/2026 07:48",
            status: "closed"
        },
        {
            text: "Sensore temperatura non risponde",
            date: "04/03/2026 11:20",
            status: "open"
        },
        {
            text: "Umidità troppo alta",
            date: "05/03/2026 16:05",
            status: "closed"
        },
        {
            text: "Possibile apertura arnia rilevata",
            date: "06/03/2026 08:54",
            status: "closed"
        },
        {
            text: "Calo rapido del peso",
            date: "06/03/2026 19:12",
            status: "open"
        },
        {
            text: "Connessione sensori instabile",
            date: "07/03/2026 10:41",
            status: "closed"
        },
        {
            text: "Temperatura interna troppo bassa",
            date: "07/03/2026 23:18",
            status: "open"
        },
        {
            text: "Livello miele critico",
            date: "08/03/2026 09:07",
            status: "open"
        }
    ];

    historyDiv.innerHTML = "";

    alerts.forEach(alert => {
        historyDiv.innerHTML += `
        <div class="history-item px-0 mb-3">
            <div>
                <div style="font-weight:600; color:white;">${alert.text}</div>
                <div style="font-size:12px; color:var(--text-muted);">${alert.date}</div>
            </div>
            <span class="tag ${alert.status}">
                ${alert.status === "open" ? "APERTO" : "RISOLTO"}
            </span>
        </div>
    `;
    });

    initDetailCharts();

    // Inizializzo le icone una sola volta alla fine
    lucide.createIcons();
});

function initDetailCharts() {
    // Opzioni condivise per i grafici per mantenere coerenza
    const commonOptions = {
        responsive: true,
        maintainAspectRatio: false,
        plugins: {
            legend: { display: false }
        },
        scales: {
            x: {
                grid: { display: false },
                ticks: { font: { size: 11 } }
            },
            y: {
                border: { dash: [4, 4] },
                ticks: { font: { size: 11 } }
            }
        }
    };

    // 1. Temperatura Interna vs Esterna
    new Chart(document.getElementById('tempInOutChart').getContext('2d'), {
        type: 'line',
        data: {
            labels: ['00', '04', '08', '12', '16', '20', '24'],
            datasets: [
                {
                    label: 'Temp In',
                    data: [34.5, 35, 35.8, 36.2, 35.9, 35.2, 34.8],
                    borderColor: '#fbbf24', // --honey-glow
                    backgroundColor: 'rgba(251,191,36,0.1)',
                    fill: true, tension: 0.4
                },
                {
                    label: 'Temp Out',
                    data: [18, 19, 23, 27, 26, 22, 20],
                    borderColor: '#60a5fa', // blue
                    fill: false, tension: 0.4
                }
            ]
        },
        options: { ...commonOptions, plugins: { legend: { display: true, labels: { color: '#94a3b8' } } } }
    });

    // 2. Grafico FFT
    new Chart(document.getElementById('fftChart').getContext('2d'), {
        type: 'line',
        data: {
            labels: ['100', '200', '250', '300', '400', '500'],
            datasets: [{
                data: [20, 45, 90, 120, 60, 30],
                borderColor: '#fbbf24',
                backgroundColor: 'rgba(251, 191, 36, 0.1)',
                fill: true, tension: 0.4
            }]
        },
        options: commonOptions
    });

    // 3. Variazione Peso
    new Chart(document.getElementById('weightFlowChart').getContext('2d'), {
        type: 'bar',
        data: {
            labels: ['08:00', '10:00', '12:00', '14:00', '16:00'],
            datasets: [{
                data: [0.1, 0.4, 0.8, -0.2, -0.5],
                backgroundColor: '#10b981', // --success
                borderRadius: 4
            }]
        },
        options: commonOptions
    });

    // 4. Umidità
    new Chart(document.getElementById('humidityChart').getContext('2d'), {
        type: 'line',
        data: {
            labels: ['08:00', '10:00', '12:00', '14:00', '16:00', '18:00', '20:00'],
            datasets: [{
                data: [55, 58, 60, 62, 59, 57, 55],
                borderColor: '#3b82f6',
                backgroundColor: 'rgba(59,130,246,0.1)',
                fill: true, tension: 0.4
            }]
        },
        options: { ...commonOptions, scales: { ...commonOptions.scales, y: { min: 0, max: 100 } } }
    });
}