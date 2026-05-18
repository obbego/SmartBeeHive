#include <iostream>
#include <unistd.h> // Per usleep
#include "niagara.h"
#include "niagara_message.h"

// Istanza UNICA del protocollo hardware low-level su Linux
Niagara lora_device;

// Funzione fittizia che simula l'inoltro a ThingsBoard richiesto nella tua architettura
void platform_send(const char* json_string) {
    std::cout << "\n========================================================" << std::endl;
    std::cout << "[SOCKET -> THINGSBOARD] Inoltro del seguente JSON:" << std::endl;
    std::cout << json_string << std::endl;
    std::cout << "========================================================\n" << std::endl;
}

int main() {
    std::cout << "--- Gateway Niagara Receiver Initialized ---" << std::endl;
    std::cout << "In ascolto di messaggi dai dispositivi IoT..." << std::endl;

    int error_code = 0;
    
    // Inizializziamo il Receiver passando il riferimento all'unica istanza 'lora_device'
    lora_device.set_initializer("Gateway0");
    NiagaraReceiver receiver(lora_device, &error_code);

    if (error_code != 0) {
        std::cerr << "[CRITICAL] Errore inizializzazione hardware/protocollo: " << error_code << std::endl;
        return 1;
    }

    while (true) {
        // Tenta la ricezione (il metodo chiama internamente device.recv())
        int ret = receiver.receive();

        if (ret == 0) { // Ricezione avvenuta con successo (NIAGARA_OK)
            std::cout << "[LORA] Ricevuto un nuovo pacchetto valido!" << std::endl;
            
            // Estraiamo il JSON generato con i timestamp sincronizzati su Epoch Unix del Gateway
            str json_payload = receiver.getJSON();
            
            // Mostriamo il JSON a terminale e simuliamo l'invio alla piattaforma
            platform_send(json_payload.c_str());

        } else {
            // Qui gestisci i codici di ritorno. 
            // Se 'ret' corrisponde al tuo codice di Timeout (es. se la receive è non-blocking o ha un timeout interno),
            // evitiamo di intasare la CPU rallentando leggermente il ciclo.
            // Assumiamo ad esempio che il timeout sia un codice specifico (es. 1 o codice di libreria).
            
            // Piccolo sleep di cortesia (100 millisecondi) per non saturare la CPU se la recv non è bloccante
            usleep(100000); 
        }
    }

    return 0;
}