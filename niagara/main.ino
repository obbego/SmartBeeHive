#include <heltec_unofficial.h>
#include "niagara.h"

bool log_initialised = false;

void log_handler_display(const char* text) {
	if(!log_initialised) {
		//Initialise the display and heltec library
		heltec_setup();
		display.init();
		display.setFont(ArialMT_Plain_10);

		// Mark the log as initialised
		log_initialised = true;
	}

	display.print(text);
}

void log_handler_serial(const char* text) {
    if(!log_initialised) {
        Serial.begin(115200);
        log_initialised = true;
    }
    Serial.print(text);
}

Niagara *device;

void setup() {
    // Inizializza la seriale (chiamando l'handler o direttamente)
    if(!log_initialised) {
        Serial.begin(115200);
        log_initialised = true;
    }

    device = new Niagara(log_handler_serial, Niagara_LogLevel::LOG_TERMINAL);
    if(!device->set_identifier("ESP32")) {
        log_handler_serial("Errore: impossibile impostare l'identificatore.\n");
        while(true); 
    }
    
    Serial.println("\n--- ESP32 NIAGARA TEST READY ---");
}

void loop() {
    // Stampa il menu
    Serial.println("\n--- MENU PRINCIPALE ---");
    Serial.println("1: Trasmetti messaggio a RASPI");
    Serial.println("2: Mettiti in ricezione");
    Serial.print("Seleziona un'opzione: ");

    // Attendi l'input dell'utente
    while (!Serial.available()) { delay(10); }
    char choice = Serial.read();
    
    // Pulisci il buffer della seriale
    while(Serial.available()) { Serial.read(); }
    Serial.println(choice);

    if (choice == '1') {
        // --- MODALITA' TRASMISSIONE ---
        Serial.print("Inserisci la stringa da inviare: ");
        while (!Serial.available()) { delay(10); }
        String input_str = Serial.readStringUntil('\n');
        input_str.trim(); // Rimuove carriage return o newline
        Serial.println(input_str);

        Serial.println("=== AVVIO TRASMISSIONE ===");
        // Nota: Assumo che il secondo parametro accetti const char*
        Niagara_Ret error = device->send("RASPI", input_str.c_str());
        
        if (error == NIAGARA_OK) {
            Serial.println("-> Trasmissione completata con successo.");
        } else {
            Serial.print("-> Errore durante la trasmissione. Codice: ");
            Serial.println(static_cast<int>(error));
        }

    } else if (choice == '2') {
        // --- MODALITA' RICEZIONE ---
        Serial.print("Inserisci il timeout in millisecondi: ");
        while (!Serial.available()) { delay(10); }
        String timeout_str = Serial.readStringUntil('\n');
        unsigned long timeout = timeout_str.toInt();
        Serial.println(timeout);

        Serial.println("=== IN ATTESA DI DATI... ===");
        unsigned long start_time = millis();
        Niagara_Ret error;
        bool message_received = false;

        while (millis() - start_time < timeout) {
            str receive_data;
            str source;
            // Controlla l'ordine degli argomenti nella tua libreria!
            error = device->receive(&receive_data, &source); 
            
            if (error == NIAGARA_OK) {
                Serial.print("-> Messaggio Ricevuto da '");
                Serial.print(source.c_str());
                Serial.print("': ");
                Serial.println(receive_data.c_str());
                message_received = true;
                break; // Esce dal loop di ricezione e torna al menu
            }
            delay(50); // Piccolo delay per non saturare la CPU
        }

        if (!message_received) {
            Serial.println("-> Timeout scaduto. Nessun messaggio ricevuto o ultimo stato in errore.");
            Serial.print("-> Ultimo codice di ritorno: ");
            Serial.println(static_cast<int>(error));
        }
    } else {
        Serial.println("Opzione non valida.");
    }
}