#include <iostream>
#include <unistd.h> // Per usleep
#include "niagara/niagara.h"
#include "niagara/niagara_measure.h"

void log_handler_terminal(const char* text) {
    printf(text);
}

// Istanza UNICA del protocollo hardware low-level su Linux
//Niagara lora_device(log_handler_terminal, Niagara_LogLevel::LOG_TERMINAL);
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

    // Inizializziamo il Receiver passando il riferimento all'unica istanza 'lora_device'
    lora_device.set_identifier("Gateway0");
    NiagaraReceiver receiver(lora_device);

    while (true) {
        // Tenta la ricezione (il metodo chiama internamente device.recv())
        int ret;
        str remote;
        str out = receiver.receive(&ret, &remote);

        if (ret == 0) { // Ricezione avvenuta con successo (NIAGARA_OK)
            std::cout << "[LORA] Ricevuto un nuovo pacchetto valido!" << std::endl;

            // Mostriamo il JSON a terminale e simuliamo l'invio alla piattaforma
            platform_send(out.c_str());

        } else {
            std::cout << "Codice di errore ritornato dalla libreria: " << ret << std::endl;

            // Piccolo sleep di cortesia (100 millisecondi) per non saturare la CPU se la recv non è bloccante
            usleep(100000);
        }
    }

    return 0;
}