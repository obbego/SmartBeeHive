#include "niagara.h"
#include "Timer.h"

#include <cstdio>
#include <cstring>
#include <string>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <thread>

using namespace std;

// Timestamped logger
void timestamp_logger(const char* text) {
    static std::string buffer;
    static bool at_line_start = true;

    auto get_timestamp = []() {
        using namespace std::chrono;

        auto now = system_clock::now();
        auto s = time_point_cast<std::chrono::seconds>(now);
        auto ms = duration_cast<milliseconds>(now - s).count();

        std::time_t tt = system_clock::to_time_t(s);
        std::tm tm{};
#if defined(_WIN32)
        localtime_s(&tm, &tt);
#else
        localtime_r(&tt, &tm);
#endif

        std::ostringstream oss;
        oss << std::put_time(&tm, "%H:%M:%S")
            << "." << std::setw(3) << std::setfill('0') << ms
            << " -> ";
        return oss.str();
    };

    buffer += text;

    size_t pos = 0;
    while ((pos = buffer.find('\n')) != std::string::npos) {
        std::string line = buffer.substr(0, pos);
        buffer.erase(0, pos + 1);

        if (at_line_start) {
            std::string ts = get_timestamp();
            std::fwrite(ts.c_str(), 1, ts.size(), stdout);
        }

        std::fwrite(line.c_str(), 1, line.size(), stdout);
        std::fwrite("\n", 1, 1, stdout);

        at_line_start = true;
    }

    // Remaining partial line (no newline yet)
    if (!buffer.empty()) {
        if (at_line_start) {
            std::string ts = get_timestamp();
            std::fwrite(ts.c_str(), 1, ts.size(), stdout);
            at_line_start = false;
        }

        std::fwrite(buffer.c_str(), 1, buffer.size(), stdout);
        buffer.clear();
    }

    std::fflush(stdout);
}

int main(void) {
    // Inizializza il dispositivo
    Niagara device(timestamp_logger, Niagara_LogLevel::LOG_TERMINAL);
    if(!device.set_identifier("RASPI")) {
        fprintf(stderr, "Errore durante l'impostazione dell'identificatore.\n");
        return 1;
    }
    
    cout << "\n--- RASPI NIAGARA TEST READY ---" << endl;

    // Loop infinito per il menu
    for(;;) {
        cout << "\n--- MENU PRINCIPALE ---" << endl;
        cout << "1: Trasmetti messaggio a ESP32" << endl;
        cout << "2: Mettiti in ricezione" << endl;
        cout << "Seleziona un'opzione: ";
        
        string choice;
        getline(cin, choice);

        if (choice == "1") {
            // --- MODALITA' TRASMISSIONE ---
            cout << "Inserisci la stringa da inviare: ";
            string input_str;
            getline(cin, input_str);

            cout << "=== AVVIO TRASMISSIONE ===" << endl;
            Niagara_Ret error = device.send("ESP32", input_str.c_str());
            
            if(error == NIAGARA_OK) {
                cout << "-> Trasmissione completata con successo." << endl;
            } else {
                cout << "-> Errore durante la trasmissione. Codice: " << static_cast<int>(error) << endl;
            }

        } else if (choice == "2") {
            // --- MODALITA' RICEZIONE ---
            cout << "Inserisci il timeout in millisecondi: ";
            string timeout_str;
            getline(cin, timeout_str);
            
            long timeout_ms;
            try {
                timeout_ms = stol(timeout_str);
            } catch (...) {
                cout << "Input non valido. Uso 5000ms di default." << endl;
                timeout_ms = 5000;
            }

            cout << "=== IN ATTESA DI DATI... ===" << endl;
            
            auto start_time = chrono::steady_clock::now();
            auto timeout_duration = chrono::milliseconds(timeout_ms);
            
            Niagara_Ret error = NIAGARA_OK;
            bool message_received = false;

            while (chrono::steady_clock::now() - start_time < timeout_duration) {
                str receive_data;
                str source;
                // Controlla l'ordine degli argomenti nella tua libreria!
                error = device.receive(&receive_data, &source);
                
                if (error == NIAGARA_OK) {
                    cout << "-> Messaggio Ricevuto da '" << source.c_str() << "': " 
                         << receive_data.c_str() << endl;
                    message_received = true;
                    break;
                }
                
                // Piccolo sleep per non mandare il core della CPU al 100%
                this_thread::sleep_for(chrono::milliseconds(10)); 
            }

            if (!message_received) {
                cout << "-> Timeout scaduto. Nessun messaggio ricevuto." << endl;
                cout << "-> Ultimo codice di ritorno (Errore/Status): " << static_cast<int>(error) << endl;
            }

        } else {
            cout << "Opzione non valida." << endl;
        }
    }
    
    return 0;
}