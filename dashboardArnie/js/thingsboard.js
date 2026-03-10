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
        `${TB_HOST}/api/plugins/telemetry/DEVICE/${deviceId}/values/timeseries?keys=temperature,humidity,weight,battery`,
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

        try {

            const telemetry = await tbGetTelemetry(deviceId);

            const temp = telemetry.temperature?.slice(-1)[0]?.value;
            const hum = telemetry.humidity?.slice(-1)[0]?.value;
            const weight = telemetry.weight?.slice(-1)[0]?.value;

            const hive = hivesData.find(h => h.id == hiveId);

            if (!hive) continue;

            hive.t = parseFloat(temp || 0);
            hive.h = parseFloat(hum || 0);
            hive.w = parseFloat(weight || 0);

        } catch (err) {

            console.error("Errore arnia", hiveId, err);

        }

    }

}