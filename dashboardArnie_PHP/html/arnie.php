<?php
require_once '../auth.php';
require_once '../db.php';
?>
<!DOCTYPE html>
<html lang="it" data-bs-theme="dark">

<head>
    <meta charset="UTF-8" />
    <meta name="viewport" content="width=device-width, initial-scale=1.0" />
    <title>Dettaglio Arnia - Smart Hive</title>

    <link rel="preconnect" href="https://fonts.googleapis.com" />
    <link href="https://fonts.googleapis.com/css2?family=Inter:wght@400;500;600;700&display=swap" rel="stylesheet" />
    <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.2/dist/css/bootstrap.min.css" rel="stylesheet" />
    <script src="https://cdn.jsdelivr.net/npm/chart.js@4.4.0/dist/chart.umd.min.js"></script>
    <script src="https://cdn.jsdelivr.net/npm/hammerjs@2.0.8"></script>
    <script src="https://cdn.jsdelivr.net/npm/chartjs-plugin-zoom@2.0.1/dist/chartjs-plugin-zoom.min.js"></script>
    <script src="https://unpkg.com/lucide@latest"></script>
    <script src="https://cdn.jsdelivr.net/npm/bootstrap@5.3.2/dist/js/bootstrap.bundle.min.js"></script>

    <link rel="stylesheet" href="../css/style.css" />
    <link rel="stylesheet" href="../css/arnie.css" />
    <link rel="stylesheet" href="../css/navbar.css" />

    <style>
        h1, h2, h3, h4, h5, h6, p { margin-bottom: 0; }
        a { text-decoration: none; }
    </style>
</head>

<body>
<?php require_once '../includes/navbar.php'; ?>

<div class="container d-flex justify-content-between align-items-center mt-1 mb-1" style="font-size:13px; color:var(--text-muted);">
    <div class="d-flex align-items-center gap-3">
        <a href="index.php" style="color:var(--text-muted); display:flex; align-items:center; transition:0.2s;"
           onmouseover="this.style.color='white'" onmouseout="this.style.color='var(--text-muted)'">
            <i data-lucide="arrow-left" style="width:20px;height:20px;"></i>
        </a>
        <h2 id="hiveName" style="font-size:20px; font-weight:700; color:white; margin:0;">Caricamento...</h2>
    </div>
</div>

<div class="container">
    <div class="row g-4 mb-4 align-items-stretch">
        <div class="col-lg-6">
            <div class="row g-3">

                <!-- Colonnina miele -->
                <div class="col-3">
                    <div class="glass-panel stat-card h-100 d-flex flex-column align-items-center justify-content-between py-3"
                         style="border-color: var(--honey-glow);">
                        <div class="stat-label text-center" style="color: var(--honey-glow); font-size: 14px; line-height: 1.2;">
                            <i data-lucide="archive"></i> <b>Livello miele</b>
                        </div>
                        <div class="honey-tank-vertical-wrapper">
                            <div class="honey-tank-vertical">
                                <div id="barMiele" class="honey-liquid-vertical" style="height: 0%"></div>
                            </div>
                        </div>
                        <span id="valMiele" class="honey-pct-vertical" style="color: var(--honey-glow); font-weight: 700;">--%</span>
                    </div>
                </div>

                <div class="col-9">
                    <div class="row g-3">
                        <!-- Peso -->
                        <div class="col-6">
                            <div class="glass-panel stat-card h-100 text-center">
                                <div class="stat-label mb-2 justify-content-center"><i data-lucide="scale"></i> Peso</div>
                                <div class="stat-value" id="valWeight">--</div>
                            </div>
                        </div>
                        <!-- Umidità -->
                        <div class="col-6">
                            <div class="glass-panel stat-card h-100 text-center">
                                <div class="stat-label mb-2 justify-content-center"><i data-lucide="droplets"></i> Umidità</div>
                                <div class="stat-value" id="valHum">--</div>
                            </div>
                        </div>
                        <!-- Temperatura Interna -->
                        <div class="col-6">
                            <div class="glass-panel stat-card h-100 text-center">
                                <div class="stat-label mb-2 justify-content-center"><i data-lucide="thermometer"></i> Temp In</div>
                                <div class="stat-value" id="valTempIn">--</div>
                            </div>
                        </div>
                        <!-- Frequenza Picco — identico agli altri 3, nessun bordo speciale -->
                        <div class="col-6">
                            <div class="glass-panel stat-card h-100 text-center">
                                <div class="stat-label mb-2 justify-content-center">
                                    <i data-lucide="activity"></i> Freq. Picco
                                </div>
                                <div class="stat-value" id="valPeakFreq">--</div>
                            </div>
                        </div>
                    </div>
                </div>

                <!-- Stato sensori -->
                <div class="col-3">
                    <div class="glass-panel stat-card h-100 text-center d-flex flex-column">
                        <div class="stat-label justify-content-center mt-2">
                            R² Pearson
                        </div>

                        <div class="flex-grow-1 d-flex align-items-center justify-content-center">
                            <div class="stat-value" id="valR2" style="font-size: 1.2rem;">
                                --
                            </div>
                        </div>
                    </div>
                </div>

                <div class="col-9">
                    <div class="glass-panel p-4 h-100 d-flex flex-column justify-content-center">
                        <div class="chart-title mb-3">Attività Sensori</div>
                        <div id="statusSemaforo" class="status-alert mb-0"></div>
                    </div>
                </div>

            </div>
        </div>

        <!-- Storico allarmi -->
        <div class="col-lg-6 col-right-fix">
            <div class="glass-panel p-0 d-flex flex-column alarm-history-container-arnie">
                <div class="p-3 border-bottom border-white border-opacity-10">
                    <div class="chart-title mb-0" style="color: var(--danger);">Storico Allarmi</div>
                </div>
                <div id="localHistory" class="history-list p-3 custom-scrollbar">
                </div>
            </div>
        </div>
    </div>

    <!-- Selettore temporale grafici -->
    <div class="d-flex justify-content-between align-items-center mb-4 mt-4">
        <h4 class="text-white mb-0" style="font-size: 1.1rem;">Analisi Storica Grafici</h4>
        <nav class="tabs-nav d-inline-flex gap-1" id="timeRangeSelector">
            <button class="tab-btn active" data-value="24h">24 Ore</button>
            <button class="tab-btn" data-value="7d">7 Giorni</button>
            <button class="tab-btn" data-value="30d">1 Mese</button>
            <button class="tab-btn" data-value="1a">1 Anno</button>
        </nav>
    </div>

    <div class="row g-4 mb-5">
        <div class="col-lg-6">
            <div class="glass-panel chart-box">
                <div class="chart-title">Temperatura Interna vs Esterna</div>
                <div class="chart-area"><canvas id="tempInOutChart"></canvas></div>
            </div>
        </div>

        <div class="col-lg-6">
            <div class="glass-panel chart-box">
                <div class="chart-title">Frequenza Picco Attività Api (Hz)</div>
                <div class="chart-area"><canvas id="peakFreqChart"></canvas></div>
            </div>
        </div>

        <div class="col-lg-6">
            <div class="glass-panel chart-box">
                <div class="chart-title">Variazione Peso (kg/h)</div>
                <div class="chart-area"><canvas id="weightFlowChart"></canvas></div>
            </div>
        </div>

        <div class="col-lg-6">
            <div class="glass-panel chart-box">
                <div class="chart-title">Umidità Interna</div>
                <div class="chart-area"><canvas id="humidityChart"></canvas></div>
            </div>
        </div>
    </div>
</div>

<script src="../js/dati.js"></script>
<script src="../js/thingsboard.js"></script>
<script src="../js/alarm_state.js"></script>
<script src="../js/arnie.js"></script>
<script src="../js/navbar.js"></script>
</body>

</html>