#include "niagara_measure.h"

// --- GESTIONE CROSS-PLATFORM DEL TEMPO ---
#ifdef ARDUINO
    #include <Arduino.h>
    unsigned long get_system_millis() {
        return millis();
    }
    unsigned long long get_unix_time_ms() {
        // Su Arduino senza RTC, ritorniamo i millis. 
        // Questo non verrà usato per calcolare l'epoca, ma serve per compatibilità di compilazione.
        return millis(); 
    }
    
    // Helper per convertire float in str cross-platform
    str float_to_str(float f) {
        return str(String(f).c_str());
    }
#else
    #include <chrono>
    unsigned long get_system_millis() {
        // Tempo di uptime relativo (simile a millis()) per calcoli differenziali
        auto now = std::chrono::steady_clock::now();
        return std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    }
    unsigned long long get_unix_time_ms() {
        // Epoch Unix reale in millisecondi (per Thingsboard)
        auto now = std::chrono::system_clock::now();
        return std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    }
    
    // Helper per convertire float in str cross-platform
    str float_to_str(float f) {
        return str(std::to_string(f).c_str());
    }
#endif

// ==========================================
// IMPLEMENTAZIONE SENDER
// ==========================================
NiagaraSender::NiagaraSender(const char* identifier, int* error_code) {
    has_custom_ts = false;
    custom_ts = 0;
    Niagara_Ret ret = device.set_identifier(identifier);
    if(error_code != nullptr) *error_code = static_cast<int>(ret);
}

void NiagaraSender::add_measure(const NiagaraMeasure& measure) {
    for(size_t i = 0; i < measures.size(); i++) {
        if(measures[i].key == measure.key) {
            measures[i].value = measure.value; // Aggiorna se esiste già
            return;
        }
    }
    measures.push_back(measure);
}

void NiagaraSender::remove_measure(const char* key) {
    str targetKey(key);
    for (auto it = measures.begin(); it != measures.end(); ++it) {
        if (it->key == targetKey) {
            measures.erase(it);
            break;
        }
    }
}

void NiagaraSender::set_timestamp(unsigned long ts_millis) {
    custom_ts = ts_millis;
    has_custom_ts = true;
}

int NiagaraSender::send(const char* destination) {
    unsigned long current_ts = get_system_millis();
    unsigned long measure_ts = has_custom_ts ? custom_ts : current_ts;

    // Costruzione payload compatto: "current_ts;measure_ts;key1:val1;key2:val2;"
    str payload = str(current_ts) + str(";") + str(measure_ts) + str(";");

    for (size_t i = 0; i < measures.size(); i++) {
        payload += measures[i].key + str(":") + float_to_str(measures[i].value) + str(";");
    }

    Niagara_Ret ret = device.send(destination, payload.c_str());
    
    // Reset dello stato temporale per la prossima misurazione
    has_custom_ts = false; 
    
    return static_cast<int>(ret);
}

// ==========================================
// IMPLEMENTAZIONE RECEIVER
// ==========================================
NiagaraReceiver::NiagaraReceiver(const char* identifier, int* error_code) {
    Niagara_Ret ret = device.set_identifier(identifier);
    if(error_code != nullptr) *error_code = static_cast<int>(ret);
}

int NiagaraReceiver::receive() {
    str remote_device, payload;
    Niagara_Ret ret = device.recv(&remote_device, &payload);
    
    if(ret != NIAGARA_OK) return static_cast<int>(ret);

    // Parsing della stringa "current_ts;measure_ts;key1:val1;key2:val2;"
    int first_semi = payload.indexOf(';');
    if (first_semi == -1) return -1; // Errore di formato

    int second_semi = payload.indexOf(';', first_semi + 1);
    if (second_semi == -1) return -1; // Errore di formato

    str current_ts_str = payload.substring(0, first_semi);
    str measure_ts_str = payload.substring(first_semi + 1, second_semi);

    long current_ts_node = current_ts_str.toInt();
    long measure_ts_node = measure_ts_str.toInt();

    // Calcolo del timestamp assoluto!
    // unix_ora - (tempo_trascorso_dalla_misura_sul_nodo)
    unsigned long long gateway_unix_now = get_unix_time_ms();
    long elapsed_on_node = current_ts_node - measure_ts_node;
    
    // Protezione per evitare underflow se i pacchetti si accavallano stranamente
    if (elapsed_on_node < 0) elapsed_on_node = 0; 
    
    unsigned long long absolute_measure_ts = gateway_unix_now - elapsed_on_node;

    // Costruzione del JSON di ThingsBoard
    // Formato: {"ts":1234567890000, "values":{"temp":25.0, "hum":55}}
    
    // Usa un helper string standard per l'unsigned long long (il tuo str.h gestisce solo int)
    #ifdef ARDUINO
        str json = str("{\"ts\":") + str(String((unsigned long)absolute_measure_ts).c_str()) + str(", \"values\":{");
    #else
        str json = str("{\"ts\":") + str(std::to_string(absolute_measure_ts).c_str()) + str(", \"values\":{");
    #endif

    int index = second_semi + 1;
    bool first_value = true;

    while (index < payload.length()) {
        int next_semi = payload.indexOf(';', index);
        if (next_semi == -1) break;

        str pair = payload.substring(index, next_semi);
        int colon = pair.indexOf(':');
        
        if (colon != -1) {
            str key = pair.substring(0, colon);
            str val = pair.substring(colon + 1);

            if (!first_value) {
                json += str(", ");
            }
            json += str("\"") + key + str("\":") + val;
            first_value = false;
        }
        index = next_semi + 1;
    }

    json += str("}}");
    last_json = json;

    return 0; // Successo
}

str NiagaraReceiver::getJSON() const {
    return last_json;
}