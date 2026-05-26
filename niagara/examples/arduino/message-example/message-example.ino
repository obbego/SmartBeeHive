#include <Arduino.h>
#include "niagara.h"
#include "niagara_measure.h"

Niagara *lora_device;

double randomDouble(double minVal, double maxVal) {
  // random() restituisce un long intero
  // Dividendo per 1.000.000 otteniamo una frazione tra 0.0 e 1.0
  double r = (double)random(0, 1000000) / 1000000.0;

  // Scala il valore nel range desiderato
  return minVal + r * (maxVal - minVal);
}

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

    NiagaraMeasure temp("tempIn", randomDouble(15, 25));
    NiagaraMeasure hum("humidity", randomDouble(20, 80));
    NiagaraMeasure weight("weight", randomDouble(35, 60));
    NiagaraMeasure freq("peakFreq", randomDouble(5000, 15000));

    // Inizializziamo il Sender passando il riferimento all'unica istanza 'lora_device'
    // Usiamo il costruttore variadico per inserire subito le prime due misure
    NiagaraSender sender(*lora_device, temp, hum, weight, freq);

    Serial.println("[LORA] Invio dati al Gateway...");
    
    int send_res = sender.send("LoRaREC");

    if (send_res == 0) { // Assumiamo 0 = NIAGARA_OK
        Serial.println("[SUCCESS] Pacchetto LoRa inviato correttamente!");
    } else {
        Serial.print("[ERRORE] Invio fallito con codice: ");
        Serial.println(send_res);
    }

    // Attendi 8 secondi prima del prossimo invio ciclico
    delay(8000);
}
