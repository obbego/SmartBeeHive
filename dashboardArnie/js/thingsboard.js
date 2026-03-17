const TB_HOST = "https://eu.thingsboard.cloud";

const TB_USERNAME = "francesco.bego@iisviolamarchesini.edu.it";
const TB_PASSWORD = "ApiApi1234!";

// DEVICE ID delle arnie
const TB_DEVICES = {
    1: "83ada8d0-171e-11f1-acb1-ebc343e93a59",
    2: "0c2d9880-1c67-11f1-a469-05ae34b6a511",
    3: "19898c50-1c67-11f1-a469-05ae34b6a511",
    4: "24507d60-1c67-11f1-b0fa-13069a1cfc9d",
    5: "387ab0d0-1c67-11f1-b0fa-13069a1cfc9d"
};

let TB_TOKEN = null;

// login
async function tbLogin() {
    if (TB_TOKEN) return TB_TOKEN;

    const res = await fetch(`${TB_HOST}/api/auth/login`, {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({
            username: TB_USERNAME,
            password: TB_PASSWORD
        })
    });

    const data = await res.json();
    TB_TOKEN = data.token;

    return TB_TOKEN;
}

async function tbGetTelemetry(deviceId) {
    const token = await tbLogin();

    const res = await fetch(
        `${TB_HOST}/api/plugins/telemetry/DEVICE/${deviceId}/values/timeseries?keys=temperature,humidity,weight,battery,honeyPct`,
        {
            headers: {
                "X-Authorization": `Bearer ${token}`
            }
        }
    );

    return await res.json();
}

// aggiorna tutte le arnie
async function tbLoadAllHives() {
    for (const hiveId in TB_DEVICES) {
        const deviceId = TB_DEVICES[hiveId];
        const hive = hivesData.find(h => h.id == hiveId);

        if (!hive) continue;

        try {
            const telemetry = await tbGetTelemetry(deviceId);

            // Prendi l'ultimo valore, se non c'è imposta a 0
            const temp = telemetry.temperature ? telemetry.temperature.slice(-1)[0].value : 0;
            const hum = telemetry.humidity ? telemetry.humidity.slice(-1)[0].value : 0;
            const weight = telemetry.weight ? telemetry.weight.slice(-1)[0].value : 0;
            const pct = telemetry.honeyPct ? telemetry.honeyPct.slice(-1)[0].value : 0;

            // Aggiorna l'oggetto hive con i dati a 1 decimale
            hive.t = parseFloat(temp).toFixed(1);
            hive.h = parseFloat(hum).toFixed(1);
            hive.w = parseFloat(weight).toFixed(1);
            hive.pct = parseFloat(pct).toFixed(0);

            // Calcola lo stato del "semaforo" della card in base ai dati veri
            if (temp == 0 && hum == 0 && weight == 0) {
                hive.status = 'yellow'; // Nessun dato ricevuto
            } else if (temp > 40 || temp < -5) {
                hive.status = 'red'; // Allarme temperatura
            } else {
                hive.status = 'green'; // Tutto ok
            }

        } catch (err) {
            console.error("Errore arnia", hiveId, err);
            // In caso di errore o disconnessione forza tutto a 0
            hive.t = 0;
            hive.h = 0;
            hive.w = 0;
            hive.pct = 0;
            hive.status = 'offline';
        }
    }
}
