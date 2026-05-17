#ifndef NIAGARA_MEASURE_H
#define NIAGARA_MEASURE_H

#include "niagara.h"
#include <vector>

// Funzioni cross-platform per la gestione del tempo
unsigned long get_system_millis();
unsigned long long get_unix_time_ms();

// Struttura della singola misura
struct NiagaraMeasure {
    str key;
    float value;
};

// ==========================================
// SENDER (Dispositivo IoT)
// ==========================================
class NiagaraSender {
private:
    Niagara device;
    std::vector<NiagaraMeasure> measures;
    unsigned long custom_ts;
    bool has_custom_ts;

    // Helper interno per l'aggiunta variadica
    void add_measures_internal() {}

    template<typename T, typename... Args>
    void add_measures_internal(T first, Args... rest) {
        add_measure(first);
        add_measures_internal(rest...);
    }

public:
    // Costruttore base
    NiagaraSender(const char* identifier, int* error_code);

    // Costruttore Variadico per accettare N misure in inizializzazione
    template<typename... Args>
    NiagaraSender(const char* identifier, int* error_code, Args... initial_measures) 
        : NiagaraSender(identifier, error_code) 
    {
        add_measures_internal(initial_measures...);
    }

    void add_measure(const NiagaraMeasure& measure);
    void remove_measure(const char* key);
    
    // Imposta un timestamp custom (se non chiamato, usa il tempo di quando si fa send())
    void set_timestamp(unsigned long ts_millis);

    // Costruisce il payload compatto e lo invia via LoRa
    int send(const char* destination);
};

// ==========================================
// RECEIVER (Gateway)
// ==========================================
class NiagaraReceiver {
private:
    Niagara device;
    str last_json;

public:
    NiagaraReceiver(const char* identifier, int* error_code);

    // Mettiti in ascolto. Ritorna NIAGARA_OK (0) se riceve, o codice di timeout/errore
    int receive();

    // Ritorna l'ultima stringa JSON generata
    str getJSON() const;
};

#endif