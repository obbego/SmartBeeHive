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
// SENDER IMPLEMENATATION
// ==========================================
NiagaraSender::NiagaraSender(Niagara& lora_device) 
    : device(lora_device) 
{}

void NiagaraSender::add_measure(const NiagaraMeasure& measure) {
    // Search if the current measure is already in the array
    for(size_t i = 0; i < measures.size(); i++) {
        if(measures[i].key == measure.key) {
            measures[i] = measure; // Update the measure with the new parameter
            return;
        }
    }
    // If the measure wasn't already present, add it as new
    measures.push_back(measure);
}

void NiagaraSender::remove_measure(const char* key) {
    str targetKey(key); //Convert the key to look for in string format
    // Iterate through all measures
    for (auto it = measures.begin(); it != measures.end(); ++it) {
        //Found the correct one
        if (it->key == targetKey) {
            //Erase this measure and exit
            measures.erase(it);
            break;
        }
    }
}

int NiagaraSender::send(const char* destination, unsigned long timestamp) {
    /* if the timestamp is not provided, use the system's current timestamp. 
    Some IoT device might use a calculated timestamp instead of the
    function to get millis because possible telemetries could encounter problems
    in calculating the corresponding epoch timestamp due to continuous reset of the millis() function
    after a deep sleep time. */
    unsigned long current_ts = (timestamp == 0) ? get_system_millis() : timestamp;

    // Message format: "current_ts;key1:val1:ts1;key2:val2:ts2;..."
    str payload = str(current_ts) + str(";"); //Add the current timestamp as the first element

    for (size_t i = 0; i < measures.size(); i++) {
        //If the measure's timestamp is zero, assign current timestamp
        unsigned long m_ts = (measures[i].timestamp == 0) ? current_ts : measures[i].timestamp;
        //Include all structure elements to the message payload
        payload += measures[i].key + str(":") + float_to_str(measures[i].value) + str(":") + str(m_ts) + str(";");
    }

    //Send the message through niagara
    Niagara_Ret ret = device.send(destination, payload.c_str());
    //Return the error code
    return static_cast<int>(ret);
}

// ==========================================
// RECEIVER IMPLEMENTATION
// ==========================================
NiagaraReceiver::NiagaraReceiver(Niagara& lora_device) 
    : device(lora_device) 
{}

str NiagaraReceiver::receive(int* error, str* ext_remote_device) {
    //Initialise the error as 0
    *error = 0;

    //Try to receive data through niagara
    str remote_device, payload;
    Niagara_Ret ret = device.receive(&payload, &remote_device);
    
    if(ret != NIAGARA_OK) {
        *error = static_cast<int>(ret);
        return "";
    }

    if(ext_remote_device != nullptr) *ext_remote_device = remote_device;

    //Extract the measure's timestamp
    int first_semi = payload.indexOf(';');
    if (first_semi == -1) { //If not present, error out
        *error = -1; 
        return "";
    }
    //Convert the measure's timestamp to a long value
    str current_ts_str = payload.substring(0, first_semi);
    long long current_ts_str_converted = current_ts_str.toLong();
    if(current_ts_str_converted < 0) {
        *error = -2;
        return "";
    }
    unsigned long current_ts_node = (unsigned long)current_ts_str_converted;

    //Get the actual current time
    unsigned long long gateway_unix_now = get_unix_time_ms();

    // Initialise JSON array for the IoT platform: [{"ts":..., "values":{...}}, ...]
    str json = str("[");
    int index = first_semi + 1;
    bool first_item = true;
    //Iterate through all measures
    while (index < payload.length()) {
        int next_semi = payload.indexOf(';', index);
        if (next_semi == -1) break;

        //Extract the string of type "key:value:timestamp"
        str pair = payload.substring(index, next_semi);
        int first_colon = pair.indexOf(':');
        
        if (first_colon != -1) { //If a first colon is present
            int second_colon = pair.indexOf(':', first_colon + 1);
            
            if (second_colon != -1) { //If a second colon is also present
                //Extract all values
                str key = pair.substring(0, first_colon);
                str val = pair.substring(first_colon + 1, second_colon);
                str ts_str = pair.substring(second_colon + 1);

                //Convert the timestamp
                long long ts_str_converted = ts_str.toLong();
                if(ts_str_converted < 0) {
                    *error = -3;
                    return "";
                }
                unsigned long node_measure_ts = (unsigned long)ts_str_converted;
                
                //Compute the real timestamp avoiding underflow
                unsigned long elapsed_on_node;
                if(node_measure_ts > current_ts_node)
                    elapsed_on_node = 0;
                else elapsed_on_node = current_ts_node - node_measure_ts;
                //Compute the corrected timestamp
                unsigned long long absolute_measure_ts = gateway_unix_now - elapsed_on_node;

                if (!first_item) {
                    json += str(", ");
                }

                //Generate the absolute timestamped string
                #ifdef ARDUINO
                    str ts_abs_str = str(String((unsigned long)absolute_measure_ts).c_str());
                #else
                    str ts_abs_str = str(std::to_string(absolute_measure_ts).c_str());
                #endif

                //Construct the JSON object
                json += str("{\"ts\":") + ts_abs_str + str(", \"values\":{\"") + key + str("\":") + val + str("}}");
                first_item = false;
            }
        }
        index = next_semi + 1;
    }

    //Close the JSON array
    json += str("]");
    return json;
}
