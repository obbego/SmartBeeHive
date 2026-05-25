#include <Arduino.h>
#include "niagara.h"
#include "niagara_measure.h"

Niagara *lora_device;

void setup() {
    Serial.begin(115200);
    while (!Serial) { delay(10); }
    Serial.println("--- IoT Device Niagara Sender Initialized ---");
    // Creazione dell'oggetto niagara all'interno del setup() per evitare crash di FreeRTOS
    // Questo oggetto non puo' essere creato nella fase di inizializzazione del programma.
    lora_device = new Niagara();
    lora_device->set_identifier("IoTDevice0");
}

void loop() {
    Serial.println("\n[INFO] Preparazione nuove misurazioni...");

    NiagaraMeasure temp("temperature", 24.8);
    NiagaraMeasure hum("humidity", 58.3);

    // Misura con timestamp CUSTOM
    unsigned long tempo_misura_passata = millis() - 5000;
    NiagaraMeasure press("pressure", 1012.4, tempo_misura_passata);

    // Inizializziamo il Sender passando il riferimento all'unica istanza 'lora_device'
    // Usiamo il costruttore variadico per inserire subito le prime due misure
    NiagaraSender sender(*lora_device, temp, hum);
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
