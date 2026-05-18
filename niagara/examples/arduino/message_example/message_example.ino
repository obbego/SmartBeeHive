#include <Arduino.h>
#include "niagara.h"
#include "niagara_message.h"

// Istanza UNICA del protocollo hardware/low-level LoRa
Niagara lora_device;

void setup() {
    Serial.begin(115200);
    while (!Serial) { delay(10); }
    Serial.println("--- IoT Device Niagara Sender Initialized ---");
    lora_device.set_initializer("IoTDevice0");
}

void loop() {
    Serial.println("\n[INFO] Preparazione nuove misurazioni...");

    NiagaraMeasure temp("temperature", 24.8);
    NiagaraMeasure hum("humidity", 58.3);

    // 2. Misura con timestamp CUSTOM
    unsigned long tempo_misura_passata = millis() - 5000;
    NiagaraMeasure press("pressure", 1012.4, tempo_misura_passata);

    int error_code = 0;

    // Inizializziamo il Sender passando il riferimento all'unica istanza 'lora_device'
    // Usiamo il costruttore variadico per inserire subito le prime due misure
    NiagaraSender sender(lora_device, &error_code, temp, hum);
    
    if (error_code != 0) {
        Serial.print("[ERRORE] Errore inizializzazione Sender: ");
        Serial.println(error_code);
        delay(5000);
        return;
    }
    sender.add_measure(press);

    Serial.println("[LORA] Invio dati al Gateway...");
    
    int send_res = sender.send("Gateway0");

    if (send_res == 0) { // Assumiamo 0 = NIAGARA_OK
        Serial.println("[SUCCESS] Pacchetto LoRa inviato correttamente!");
    } else {
        Serial.print("[ERRORE] Invio fallito con codice: ");
        Serial.println(send_res);
    }

    // Attendi 8 secondi prima del prossimo invio ciclico
    delay(8000);
}