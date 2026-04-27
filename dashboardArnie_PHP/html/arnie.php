<?php require_once '../auth.php'; ?>
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
  <script src="https://unpkg.com/lucide@latest"></script>
  <script src="https://cdn.jsdelivr.net/npm/bootstrap@5.3.2/dist/js/bootstrap.bundle.min.js"></script>

  <link rel="stylesheet" href="../css/style.css" />
  <link rel="stylesheet" href="../css/arnie.css" />

  <style>
    h1, h2, h3, h4, h5, h6, p { margin-bottom: 0; }
    a { text-decoration: none; }
  </style>
</head>

<body>
<?php require_once '../includes/navbar.php'; ?>

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
        <div class="col-12">
          <div class="glass-panel p-4">
            <div class="chart-title mb-3">Stato Sensori</div>
            <div id="statusSemaforo" class="status-alert mb-3"></div>
            <div class="d-flex justify-content-between align-items-center pt-3">
              <span style="font-size: 13px; color: var(--text-muted);">Affidabilità R² (In/Out):</span>
              <strong id="valR2" class="text-white">0.82</strong>
            </div>
          </div>
        </div>

      </div>
    </div>

    <!-- Storico allarmi -->
    <div class="col-lg-6 col-right-fix">
      <div class="glass-panel p-0 flex-grow-1 d-flex flex-column margin-top-align">
        <div class="p-3 border-bottom border-white border-opacity-10">
          <div class="chart-title mb-0" style="color: var(--danger);">Storico Allarmi</div>
        </div>
        <div id="localHistory" class="history-list p-3"></div>
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

<div class="container d-flex justify-content-between align-items-center py-3">
  <h2 id="hiveName" style="font-size:20px; font-weight:700;">Caricamento...</h2>
  <span id="lastUpdate" style="font-size:13px; color:var(--text-muted);">Ultimo dato: --:--</span>
</div>

<script src="../js/dati.js"></script>
<script src="../js/thingsboard.js"></script>
<script src="../js/arnie.js"></script>
<script src="../js/navbar.js"></script>
</body>

</html>