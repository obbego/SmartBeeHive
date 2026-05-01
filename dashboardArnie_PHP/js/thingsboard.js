// Percorso del tuo nuovo backend PHP
const API_URL = "../api.php";

/*
async function tbGetTelemetry(hiveId, interval = '24h') {
    try {
        const res = await fetch(`${API_URL}?id=${hiveId}&interval=${interval}`);
        const data = await res.json();
        return data[hiveId] || {};
    } catch (error) {
        console.error("Errore chiamata API PHP:", error);
        return {};
    }
}*/

async function tbGetTelemetry(hiveId, interval = 'latest') {
    try {
        const res = await fetch(`${API_URL}?id=${hiveId}&interval=${interval}`);
        const data = await res.json();
        return data[hiveId] || {};
    } catch (error) {
        console.error("Errore chiamata API PHP:", error);
        return {};
    }
}

async function tbLoadAllHives() {
    try {
        const res = await fetch(API_URL);
        const allData = await res.json();

        for (let i = 0; i < hivesData.length; i++) {
            const hive = hivesData[i];
            const telemetry = allData[hive.id];

            if (!telemetry) continue;

            const temp = telemetry.tempIn ? telemetry.tempIn.slice(-1)[0].value : 0;
            const hum = telemetry.humidity ? telemetry.humidity.slice(-1)[0].value : 0;
            const weight = telemetry.honeyWeightKg ? telemetry.honeyWeightKg.slice(-1)[0].value : 0;
            const pct = telemetry.honeyPct ? telemetry.honeyPct.slice(-1)[0].value : 0;
            const tOut = telemetry.tempOut ? telemetry.tempOut.slice(-1)[0].value : 0;
            const peakFreq = telemetry.peakFreq ? telemetry.peakFreq.slice(-1)[0].value : 0;

            hive.t = parseFloat(temp).toFixed(1);
            hive.h = parseFloat(hum).toFixed(1);
            hive.w = parseFloat(weight).toFixed(1);
            hive.pct = parseFloat(pct).toFixed(0);
            hive.tOut = parseFloat(tOut).toFixed(1);
            hive.peakFreq = parseFloat(peakFreq).toFixed(0);

            // 1. Controllo prioritario: i dati sono più vecchi di 24 ore?
            if (telemetry.is_stale) {
                hive.status = 'yellow';
                hive.lastUpdate = "Dati non aggiornati da oltre 24 ore";
            }
            // 2. Altrimenti, se i dati sono recenti, controlliamo se sono validi
            else {
                if (temp == 0 && hum == 0 && weight == 0) {
                    hive.status = 'yellow';
                    // In questo caso il semaforo è giallo, ma il messaggio sarà diverso
                    hive.lastUpdate = "Dati mancanti";
                } else if (temp > 40 || temp < -5) {
                    hive.status = 'red';
                    hive.lastUpdate = "Allarme Temp";
                } else {
                    // Dati recenti e corretti
                    hive.status = 'green';
                    hive.lastUpdate = "Aggiornato";
                }
            }
        }
    } catch (err) {
        console.error("Errore caricamento dati dal PHP", err);
        hivesData.forEach(hive => {
            hive.t = 0; hive.h = 0; hive.w = 0; hive.pct = 0; hive.tOut = 0; hive.peakFreq = 0; hive.status = 'offline';
        });
    }
}

async function tbLoadAlarms() {
    try {
        const res = await fetch(API_URL);
        const allData = await res.json();
        const alarms = allData.alarms || [];

        const alarmTypeLabels = {
            'HoneyReady':                   'Miele pronto da raccogliere',
            'ErrorDeviceTimeseries':         'Errore lettura telemetria',
            'TelemetryInvalidKey':           'Chiave telemetria non valida',
            'FailedAssetAttributes':         'Errore attributi asset',
            'DeviceOldTemperature':          'Temperatura non aggiornata',
            'DeviceOldHumidity':             'Umidità non aggiornata',
            'DeviceOldWeight':               'Peso non aggiornato',
            'DeviceOldNoiseFrequency':       'Freq. rumore non aggiornata',
            'DeviceOldNoiseIntensity':       'Intensità rumore non aggiornata',
            'DeviceDifferentTemperature':    'Temperatura anomala rispetto alle altre arnie',
            'DeviceDifferentHumidity':       'Umidità anomala rispetto alle altre arnie',
            'DeviceDifferentWeight':         'Peso anomalo rispetto alle altre arnie',
            'DeviceDifferentNoiseFrequency': 'Frequenza rumore anomala',
            'DeviceDifferentNoiseIntensity': 'Intensità rumore anomala',
            'ErrorTimeSeriesWeightDevice':   'Errore lettura peso'
        };

        return alarms.map(alarm => {
            const deviceName = alarm.originator?.entityType === 'DEVICE'
                ? (alarm.originatorName || 'Dispositivo sconosciuto')
                : 'Sistema';

            const ts = alarm.createdTime
                ? new Date(alarm.createdTime).toLocaleString('it-IT', {
                    day: '2-digit', month: '2-digit', year: '2-digit',
                    hour: '2-digit', minute: '2-digit'
                })
                : '--';

            return {
                id: alarm.id?.id || alarm.id || (deviceName + '_' + ts),
                hive: deviceName,
                msg: alarmTypeLabels[alarm.type] || alarm.type,
                time: ts,
                status: alarm.status === 'CLEARED_ACK' || alarm.status === 'CLEARED_UNACK' ? 'closed' : 'open',
                severity: alarm.severity || 'WARNING', tbStatus: alarm.status || ''
            };

        });
    } catch (err) {
        console.error("Errore caricamento allarmi", err);
        return [];
    }
}
// ─── AZIONI ALLARMI SU THINGSBOARD ────────────────────────────────
const ALARM_ACTION_URL = "../alarm_action.php";

async function tbAckAlarm(alarmId) {
    const res  = await fetch(ALARM_ACTION_URL, {
        method:  'POST',
        headers: { 'Content-Type': 'application/json' },
        body:    JSON.stringify({ action: 'ack', alarmId }),
    });
    const data = await res.json().catch(() => ({}));
    if (!res.ok) throw new Error(data.error || `HTTP ${res.status}`);
    return data;
}

async function tbClearAlarm(alarmId) {
    const res  = await fetch(ALARM_ACTION_URL, {
        method:  'POST',
        headers: { 'Content-Type': 'application/json' },
        body:    JSON.stringify({ action: 'clear', alarmId }),
    });
    const data = await res.json().catch(() => ({}));
    if (!res.ok) throw new Error(data.error || `HTTP ${res.status}`);
    return data;
}