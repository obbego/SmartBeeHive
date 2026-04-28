<?php
// ============================================================
// ACCOUNT DISABILITATI — decommentare quando si lavora a scuola
// ============================================================
require_once '../auth.php'; // fornisce $utente_nome = 'Dev' in modalità locale
?>
<!DOCTYPE html>
<html lang="it" data-bs-theme="dark">

<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Dashboard - Smart Hive</title>

    <link rel="preconnect" href="https://fonts.googleapis.com">
    <link rel="preconnect" href="https://fonts.gstatic.com" crossorigin>
    <link href="https://fonts.googleapis.com/css2?family=Inter:wght@400;500;600;700&display=swap" rel="stylesheet">
    <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.2/dist/css/bootstrap.min.css" rel="stylesheet">

    <script src="https://cdn.jsdelivr.net/npm/chart.js@4.4.0/dist/chart.umd.min.js"></script>
    <script src="https://unpkg.com/lucide@latest"></script>

    <link rel="stylesheet" href="../css/style.css">

    <style>
        /* Rimuovo i margini di default di bootstrap per evitare interferenze con il layout*/
        h1,
        h2,
        h3,
        h4,
        h5,
        h6,
        p {
            margin-bottom: 0;
        }

        a {
            text-decoration: none;
        }
    </style>
</head>

<body>

<header class="mb-4 py-3 border-bottom border-secondary border-opacity-25" style="background: rgba(15, 23, 42, 0.3);">
    <div class="container d-flex justify-content-between align-items-center">
        <h1><i data-lucide="layout-grid" class="brand-icon me-2"></i> FrontEnd Managment Arnie</h1>

        <div class="d-flex align-items-center gap-4">
            <div class="form-check form-switch d-flex align-items-center mb-0">
                <input class="form-check-input me-2" type="checkbox" role="switch" id="mockDataSwitch" style="cursor: pointer; width: 2.5em; height: 1.25em; border-color: rgba(255,255,255,0.5);">
                <label class="form-check-label text-white" for="mockDataSwitch" style="font-size: 13px; cursor: pointer;">Modalità Demo</label>
            </div>

            <div style="font-size: 13px; color: var(--text-muted);">
                Benvenuto, <strong class="text-white"><?= htmlspecialchars($utente_nome) ?></strong>
                &nbsp;·&nbsp;
                <a href="../logout.php" style="color: var(--text-muted); font-size: 13px; text-decoration: underline;">Esci</a>
            </div>
        </div>
    </div>
</header>

<div class="container pb-5">
    <div class="row g-3 mb-4">
        <!-- Miele Totale -->
        <div class="col-12 col-sm-6 col-lg">
            <div class="glass-panel stat-card h-100 text-center" style="border-color: var(--honey-glow);">
                <div class="stat-label mb-2 justify-content-center" style="color: var(--honey-glow);"><i data-lucide="archive"></i> Miele Totale</div>
                <div class="stat-value" id="statHoney" style="color: var(--honey-glow);">--</div>
                <div class="stat-detail">Somma da tutte le arnie</div>
            </div>
        </div>

        <!-- Allarmi Attivi -->
        <div class="col-12 col-sm-6 col-lg">
            <div class="glass-panel stat-card h-100 text-center" style="border-color: var(--danger);">
                <div class="stat-label mb-2 justify-content-center" style="color: var(--danger);"><i data-lucide="alert-octagon" color="var(--danger)"></i> Allarmi</div>
                <div class="stat-value" id="statAlarms" style="color: var(--danger);">--</div>
                <div class="stat-detail" id="statAlarmsDetail">Attenzione richiesta</div>
            </div>
        </div>

        <!-- Temperatura Interna Media -->
        <div class="col-12 col-sm-6 col-lg">
            <div class="glass-panel stat-card h-100 text-center">
                <div class="stat-label mb-2 justify-content-center"><i data-lucide="thermometer"></i> Temp. Media Int.</div>
                <div class="stat-value" id="statTemp">--</div>
                <div class="stat-detail">Media arnie attive</div>
            </div>
        </div>

        <!-- Umidità Media -->
        <div class="col-12 col-sm-6 col-lg">
            <div class="glass-panel stat-card h-100 text-center">
                <div class="stat-label mb-2 justify-content-center"><i data-lucide="droplets"></i> Umidità Media</div>
                <div class="stat-value" id="statHum">--</div>
                <div class="stat-detail">Media arnie attive</div>
            </div>
        </div>

        <!-- Temperatura Esterna Media -->
        <div class="col-12 col-sm-6 col-lg">
            <div class="glass-panel stat-card h-100 text-center" style="border-color: rgba(96, 165, 250, 0.5);">
                <div class="stat-label mb-2 justify-content-center" style="color: #60a5fa;"><i data-lucide="cloud-sun"></i> Temp. Esterna</div>
                <div class="stat-value" id="statTempOut" style="color: #60a5fa;">--</div>
                <div class="stat-detail">Temperatura Esterna</div>
            </div>
        </div>
    </div>

    <div class="columns">
        <!-- div arnie -->
        <div class="left">
            <div class="tabs-container mb-4 d-flex align-items-center">
                <h3 class="fs-5 fw-bold mb-0 me-2">Stato Arnie</h3>
            </div>
            <div class="row g-4 mb-5 justify-content-center" id="hivesGrid">
                <div class="col-12 text-center text-muted">Caricamento dati in corso...</div>
            </div>
        </div>

        <!-- div storico, panoramica e analisi avanzata -->
        <div class="right">
            <div class="tabs-container mb-4 d-flex justify-content-between align-items-center flex-wrap gap-3">

                <!-- TAB PRINCIPALI -->
                <nav class="tabs-nav d-inline-flex gap-1">
                    <button class="tab-btn active" onclick="switchTab('history', this)">Storico Allarmi</button>
                    <button class="tab-btn" onclick="switchTab('overview', this)">Panoramica</button>
                    <button class="tab-btn" onclick="switchTab('analysis', this)">Analisi Avanzata</button>
                </nav>

                <!-- TIME SELECTOR (DINAMICO) -->
                <div id="globalTimeSelector" class="tabs-nav d-inline-flex gap-1 d-none"></div>

            </div>

            <div id="tab-history" class="tab-content active">
                <div class="row g-4 d-flex justify-content-center">
                    <div class="col-12 col-lg-11">
                        <div class="glass-panel chart-box" style="height: auto; min-height: 400px;">
                            <div class="p-3 border-bottom border-white border-opacity-10 d-flex justify-content-between align-items-center">
                                <div class="chart-title mb-0">Log Eventi e Allarmi</div>
                                <a href="allarmi.php" class="btn-manage-alarms">
                                    <i data-lucide="bell-ring" style="width:14px;height:14px;"></i>
                                    Gestisci Allarmi
                                </a>
                            </div>
                            <div id="historyList" class="history-list p-3">
                            </div>
                        </div>
                    </div>
                </div>
            </div>

            <div id="tab-overview" class="tab-content ">
                <div class="row g-4 d-flex justify-content-center">
                    <div class="col-12 col-lg-11 d-flex justify-content-between align-items-center mt-2 mb-2">
                    </div>
                    <div class="col-12 col-lg-11">
                        <div class="glass-panel chart-box">
                            <div class="chart-title">Andamento Temperature</div>
                            <div class="chart-area"><canvas id="tempChart"></canvas></div>
                        </div>
                    </div>
                    <div class="col-12 col-lg-11">
                        <div class="glass-panel chart-box">
                            <div class="chart-title">Livelli Umidità</div>
                            <div class="chart-area"><canvas id="humidityChart"></canvas></div>
                        </div>
                    </div>
                    <div class="col-12 col-lg-11">
                        <div class="glass-panel chart-box">
                            <div class="chart-title">Produzione Giornaliera</div>
                            <div class="chart-area"><canvas id="honeyChart"></canvas></div>
                        </div>
                    </div>
                    <div class="col-12 col-lg-11">
                        <div class="glass-panel chart-box">
                            <div class="chart-title">Frequenza Sonora (Hz)</div>
                            <div class="chart-area"><canvas id="soundChart"></canvas></div>
                        </div>
                    </div>
                </div>
            </div>

            <div id="tab-analysis" class="tab-content">
                <div class="row g-4 d-flex justify-content-center">
                    <div class="col-12 col-lg-11 d-flex justify-content-between align-items-center mt-2 mb-2"> </div>

                    <!-- Scatter correlazione -->
                    <div class="col-12 col-lg-11">
                        <div class="glass-panel chart-box">
                            <div class="chart-title">Correlazione Temp. Interna / Esterna</div>
                            <div id="analysisR2"
                                 style="font-size: 28px; font-weight: 700; color: var(--honey-glow); text-align:center; margin-bottom: 10px;">
                                R² = ...
                            </div>
                            <div class="chart-area"><canvas id="correlationChart"></canvas></div>
                        </div>
                    </div>

                    <!-- Derivata temperatura -->
                    <div class="col-12 col-lg-11">
                        <div class="glass-panel chart-box">
                            <div class="chart-title">Velocità Variazione Temperatura (°C/h)</div>
                            <div style="font-size: 12px; color: var(--text-muted); margin-bottom: 8px;">
                                Media aggregata tra tutte le arnie &mdash;
                                <span style="color: rgba(251,191,36,0.85);">▌</span> riscaldamento &nbsp;
                                <span style="color: rgba(96,165,250,0.85);">▌</span> raffreddamento
                            </div>
                            <div class="chart-area"><canvas id="derivative1Chart"></canvas></div>
                        </div>
                    </div>

                    <!-- Derivata peso -->
                    <div class="col-12 col-lg-11">
                        <div class="glass-panel chart-box">
                            <div class="chart-title">Rateo Variazione Peso Totale (kg/h)</div>
                            <div style="font-size: 12px; color: var(--text-muted); margin-bottom: 8px;">
                                Somma di tutte le arnie &mdash;
                                <span style="color: rgba(16,185,129,0.85);">▌</span> produzione &nbsp;
                                <span style="color: rgba(239,68,68,0.85);">▌</span> perdita
                            </div>
                            <div class="chart-area"><canvas id="weightDerivativeChart"></canvas></div>
                        </div>
                    </div>
                </div>
            </div>
        </div>
    </div>
</div>

<script src="../js/dati.js"></script>
<script src="https://cdn.jsdelivr.net/npm/bootstrap@5.3.2/dist/js/bootstrap.bundle.min.js"></script>
<script src="../js/index.js"></script>
<script src="../js/thingsboard.js"></script>
</body>
</html>