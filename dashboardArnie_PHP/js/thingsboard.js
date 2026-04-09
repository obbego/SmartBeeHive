// Percorso del tuo nuovo backend PHP
const API_URL = "../api.php"; 

async function tbGetTelemetry(hiveId) {
    try {
        const res = await fetch(`${API_URL}?id=${hiveId}`);
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
            const weight = telemetry.weight ? telemetry.weight.slice(-1)[0].value : 0;
            const pct = telemetry.honeyPct ? telemetry.honeyPct.slice(-1)[0].value : 0;
            const tOut = telemetry.tempOut ? telemetry.tempOut.slice(-1)[0].value : 0;

            hive.t = parseFloat(temp).toFixed(1);
            hive.h = parseFloat(hum).toFixed(1);
            hive.w = parseFloat(weight).toFixed(1);
            hive.pct = parseFloat(pct).toFixed(0);
            hive.tOut = parseFloat(tOut).toFixed(1);

            if (temp == 0 && hum == 0 && weight == 0) {
                hive.status = 'yellow'; 
            } else if (temp > 40 || temp < -5) {
                hive.status = 'red'; 
            } else {
                hive.status = 'green'; 
            }
        }
    } catch (err) {
        console.error("Errore caricamento dati dal PHP", err);
        hivesData.forEach(hive => {
            hive.t = 0; hive.h = 0; hive.w = 0; hive.pct = 0; hive.status = 'offline';
        });
    }
}