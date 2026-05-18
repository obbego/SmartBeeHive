#include "niagara_measure.h"

// --- GESTIONE CROSS-PLATFORM DEL TEMPO ---
#ifdef ARDUINO
    #include <Arduino.h>
    unsigned long get_system_millis() {
        return millis();
    }
    unsigned long long get_unix_time_ms() {
        return millis(); 
    }
    
    str float_to_str(float f) {
        return str(String(f).c_str());
    }
#else
    #include <chrono>
    unsigned long get_system_millis() {
        auto now = std::chrono::steady_clock::now();
        return std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    }
    unsigned long long get_unix_time_ms() {
        auto now = std::chrono::system_clock::now();
        return std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    }
    
    str float_to_str(float f) {
        return str(std::to_string(f).c_str());
    }
#endif

// ==========================================
// IMPLEMENTAZIONE SENDER
// ==========================================
NiagaraSender::NiagaraSender(Niagara& lora_device) 
    : device(lora_device) 
{}

void NiagaraSender::add_measure(const NiagaraMeasure& measure) {
    for(size_t i = 0; i < measures.size(); i++) {
        if(measures[i].key == measure.key) {
            measures[i] = measure; // Aggiorna interamente la struttura (incluso il timestamp)
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

int NiagaraSender::send(const char* destination) {
    unsigned long current_ts = get_system_millis();

    // Nuovo formato compatto: "current_ts;key1:val1:ts1;key2:val2:ts2;"
    str payload = str(current_ts) + str(";");

    for (size_t i = 0; i < measures.size(); i++) {
        // Se il timestamp della misura è 0, assegna il timestamp corrente di invio
        unsigned long m_ts = (measures[i].timestamp == 0) ? current_ts : measures[i].timestamp;
        
        payload += measures[i].key + str(":") + float_to_str(measures[i].value) + str(":") + str(m_ts) + str(";");
    }

    Niagara_Ret ret = device.send(destination, payload.c_str());
    return static_cast<int>(ret);
}

// ==========================================
// IMPLEMENTAZIONE RECEIVER
// ==========================================
NiagaraReceiver::NiagaraReceiver(Niagara& lora_device) 
    : device(lora_device) 
{}

int NiagaraReceiver::receive() {
    str remote_device, payload;
    Niagara_Ret ret = device.receive(&remote_device, &payload);
    
    if(ret != NIAGARA_OK) return static_cast<int>(ret);

    // Estrae il primo blocco (il timestamp di sincronizzazione del nodo)
    int first_semi = payload.indexOf(';');
    if (first_semi == -1) return -1;

    str current_ts_str = payload.substring(0, first_semi);
    unsigned long current_ts_node = static_cast<unsigned long>(current_ts_str.toInt());

    unsigned long long gateway_unix_now = get_unix_time_ms();

    // Inizializza l'array JSON per ThingsBoard: [{"ts":..., "values":{...}}, ...]
    str json = str("[");
    int index = first_semi + 1;
    bool first_item = true;

    while (index < payload.length()) {
        int next_semi = payload.indexOf(';', index);
        if (next_semi == -1) break;

        // Estrae la stringa del tipo "key:value:measure_ts"
        str pair = payload.substring(index, next_semi);
        int first_colon = pair.indexOf(':');
        
        if (first_colon != -1) {
            int second_colon = pair.indexOf(':', first_colon + 1);
            
            if (second_colon != -1) {
                str key = pair.substring(0, first_colon);
                str val = pair.substring(first_colon + 1, second_colon);
                str ts_str = pair.substring(second_colon + 1);

                unsigned long node_measure_ts = static_cast<unsigned long>(ts_str.toInt());
                
                // Calcolo differenziale del timestamp reale
                long elapsed_on_node = current_ts_node - node_measure_ts;
                if (elapsed_on_node < 0) elapsed_on_node = 0; // Protezione anti-underflow
                
                unsigned long long absolute_measure_ts = gateway_unix_now - elapsed_on_node;

                if (!first_item) {
                    json += str(", ");
                }

                // Generazione della stringa del timestamp assoluto a 64 bit
                #ifdef ARDUINO
                    str ts_abs_str = str(String((unsigned long)absolute_measure_ts).c_str());
                #else
                    str ts_abs_str = str(std::to_string(absolute_measure_ts).c_str());
                #endif

                // Costruisce l'oggetto dell'array
                json += str("{\"ts\":") + ts_abs_str + str(", \"values\":{\"") + key + str("\":") + val + str("}}");
                first_item = false;
            }
        }
        index = next_semi + 1;
    }

    json += str("]");
    last_json = json;

    return 0; 
}

str NiagaraReceiver::getJSON() const {
    return last_json;
}