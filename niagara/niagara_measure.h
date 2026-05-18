#ifndef NIAGARA_MEASURE_H
#define NIAGARA_MEASURE_H

#include "niagara.h"
#include "str.h"
#include <vector>

// Funzioni cross-platform per la gestione del tempo
unsigned long get_system_millis();
unsigned long long get_unix_time_ms();

// Struttura della singola misura aggiornata con timestamp predefinito a 0
struct NiagaraMeasure {
    str key;
    float value;
    unsigned long timestamp;

    // Costruttore per permettere l'inizializzazione implicita ed esplicita con default a 0
    NiagaraMeasure(str k, float v, unsigned long ts = 0) 
        : key(k), value(v), timestamp(ts) {}
};

// ==========================================
// SENDER (Dispositivo IoT)
// ==========================================
class NiagaraSender {
private:
    Niagara& device; // Riferimento all'istanza Niagara esterna
    std::vector<NiagaraMeasure> measures;

    // Helper interno per l'aggiunta variadica
    void add_measures_internal() {}

    template<typename T, typename... Args>
    void add_measures_internal(T first, Args... rest) {
        add_measure(first);
        add_measures_internal(rest...);
    }

public:
    // Costruttore base che accetta il riferimento all'oggetto Niagara
    NiagaraSender(Niagara& lora_device);

    // Costruttore Variadico per accettare N misure in inizializzazione
    template<typename... Args>
    NiagaraSender(Niagara& lora_device, Args... initial_measures) 
        : NiagaraSender(lora_device) 
    {
        add_measures_internal(initial_measures...);
    }

    void add_measure(const NiagaraMeasure& measure);
    void remove_measure(const char* key);

    // Costruisce il payload inserendo i timestamp per ogni misura e invia
    int send(const char* destination);
};

// ==========================================
// RECEIVER (Gateway)
// ==========================================
class NiagaraReceiver {
private:
    Niagara& device; // Riferimento all'istanza Niagara esterna
    str last_json;

public:
    NiagaraReceiver(Niagara& lora_device);

    // Ricezione e parsing dinamico dell'array JSON
    int receive();

    // Ritorna l'array JSON generato
    str getJSON() const;
};

#endif