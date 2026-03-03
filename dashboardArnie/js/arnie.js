// Database Mock (In produzione sarà una chiamata API a ThingsBoard)
document.addEventListener('DOMContentLoaded', () => {
    // 1. Recupero ID dall'URL
    const params = new URLSearchParams(window.location.search);
    const hiveId = parseInt(params.get('id'));

    // 2. Trovo i dati dell'arnia
    const hive = hivesData.find(h => h.id === hiveId) || hivesData[0];

    // 3. Popolo i testi
    document.getElementById('hiveName').innerText = hive.name;
    document.getElementById('valTempIn').innerText = hive.t + '°C';    // Era tIn
    document.getElementById('valTempOut').innerText = hive.tOut + '°C';
    document.getElementById('valWeight').innerText = hive.w + 'kg';
    document.getElementById('valHum').innerText = hive.h + '%';
    document.getElementById('lastUpdate').innerText = 'Ultimo aggiornamento: ' + hive.lastUpdate;

    // 4. Gestione Semaforo (Cap 3.2 Analisi Funzionale)
    // ... dentro il DOMContentLoaded dopo aver trovato l'oggetto 'hive'
    // ... dentro il find dell'arnia e il popolamento dei testi ...
    const semaforo = document.getElementById('statusSemaforo');

    if (hive.status === 'green') {
        semaforo.className = 'alert alert-success d-flex align-items-center gap-2 border-0';
        semaforo.style.color = '#10b981'; // Verde smeraldo
        semaforo.innerHTML = '<i data-lucide="check-circle"></i> Stato Ottimale: Tutto sotto controllo';
    }
    else if (hive.status === 'yellow') {
        semaforo.className = 'alert alert-warning d-flex align-items-center gap-2 border-0';
        semaforo.style.backgroundColor = 'rgba(251, 191, 36, 0.1)';
        semaforo.style.color = '#fbbf24';
        semaforo.innerHTML = '<i data-lucide="help-circle"></i> Stato Instabile: Monitorare variazioni';
    }
    else if (hive.status === 'red') {
        semaforo.className = 'alert alert-danger d-flex align-items-center gap-2 border-0';
        semaforo.style.color = '#ef4444'; // Rosso alert
        semaforo.innerHTML = '<i data-lucide="alert-triangle"></i> Allarme: Intervento richiesto';
    }

    // Re-inizializza le icone per mostrare quella corretta nel semaforo
    lucide.createIcons();

    // Ricorda di chiamare lucide dopo aver iniettato l'HTML
    lucide.createIcons();

    // 5. Inizializzo icone e grafici
    lucide.createIcons();
    initDetailCharts();
});

function initDetailCharts() {
    //Grafico Temperatura Interna vs Esterna
    const ctxTemp = document.getElementById('tempInOutChart').getContext('2d');

    new Chart(ctxTemp, {
        type: 'line',
        data: {
            labels: ['00', '04', '08', '12', '16', '20', '24'],
            datasets: [
                {
                    label: 'Temp Interna',
                    data: [34.5, 35, 35.8, 36.2, 35.9, 35.2, 34.8],
                    borderColor: '#fbbf24',
                    backgroundColor: 'rgba(251,191,36,0.15)',
                    fill: true,
                    tension: 0.4
                },
                {
                    label: 'Temp Esterna',
                    data: [18, 19, 23, 27, 26, 22, 20],
                    borderColor: '#60a5fa',
                    backgroundColor: 'rgba(96,165,250,0.1)',
                    fill: true,
                    tension: 0.4
                }
            ]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false,
            plugins: {
                legend: {
                    display: true
                }
            },
            scales: {
                y: {
                    title: {
                        display: true,
                        text: 'Gradi [°C]',
                        color: '#94a3b8',
                        font: { size: 12 }
                    }
                },
                x: {
                    title: {
                        display: true,
                        text: 'Ora',
                        color: '#94a3b8',
                        font: { size: 12 }
                    }
                }
            }
        }
    });

    // Grafico FFT (Frequenza vs Ampiezza)
    const ctxFft = document.getElementById('fftChart').getContext('2d');
    new Chart(ctxFft, {
        type: 'line',
        data: {
            labels: ['100Hz', '200Hz', '250Hz', '300Hz', '400Hz', '500Hz'],
            datasets: [{
                label: 'Ampiezza (dB)',
                data: [20, 45, 90, 120, 60, 30],
                borderColor: '#fbbf24',
                fill: true,
                backgroundColor: 'rgba(251, 191, 36, 0.1)',
                tension: 0.4
            }]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false,
            scales: {
                x: {
                    title: {
                        display: true,
                        text: 'Frequenza [Hz]',
                        color: '#94a3b8',
                        font: { size: 12 }
                    }
                },
                y: {
                    title: {
                        display: true,
                        text: 'Ampiezza [dB]',
                        color: '#94a3b8',
                        font: { size: 12 }
                    }
                }
            },
            plugins: {
                legend: { display: false }
            }
        }
    });

    // Grafico Variazione Peso (Derivata Prima)
    const ctxWeight = document.getElementById('weightFlowChart').getContext('2d');
    new Chart(ctxWeight, {
        type: 'bar',
        data: {
            labels: ['08:00', '10:00', '12:00', '14:00', '16:00'],
            datasets: [{
                label: 'Variazione Peso (kg/h)',
                data: [0.1, 0.4, 0.8, -0.2, -0.5],
                backgroundColor: '#10b981'
            }]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false,
            scales: {
                x: {
                    title: {
                        display: true,
                        text: 'Ora',
                        color: '#94a3b8',
                        font: { size: 12 }
                    }
                },
                y: {
                    title: {
                        display: true,
                        text: 'Variazione Peso [kg/h]',
                        color: '#94a3b8',
                        font: { size: 12 }
                    }
                }
            },
            plugins: {
                legend: { display: false }
            }
        }
    });

    // Grafico Umidità
    const ctxHum = document.getElementById('humidityChart').getContext('2d');
    new Chart(ctxHum, {
        type: 'line',
        data: {
            labels: ['08:00', '10:00', '12:00', '14:00', '16:00', '18:00', '20:00'],
            datasets: [{
                label: 'Umidità (%)',
                data: [55, 58, 60, 62, 59, 57, 55], // valori di esempio, sostituire con dati reali
                borderColor: '#3b82f6', // blu
                backgroundColor: 'rgba(59,130,246,0.1)',
                fill: true,
                tension: 0.4
            }]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false,
            plugins: {
                legend: { display: true }
            },
            scales: {
                y: {
                    min: 0,
                    max: 100,
                    title: {
                        display: true,
                        text: 'Umidità (%)',
                        color: '#94a3b8',
                        font: { size: 12 }
                    }
                },
                x: {
                    title: {
                        display: true,
                        text: 'Ora',
                        color: '#94a3b8',
                        font: { size: 12 }
                    }
                }
            }
        }
    });
}