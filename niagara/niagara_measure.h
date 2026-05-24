/**
 * @file niagara_measure.h
 * @brief Handler for the application layer of the Niagara protocol
 * 
 * This file handles the direct communication of measures between devices in the niagara protocol,
 * handling timestamping for IoT devices which don't have an internal RTC and have to rely on their
 * internal CPU timer.
 * 
 * The *Sender* is the IoT device which constructs the measure array and applies its local
 * timestamps based on its CPU timer.
 * 
 * The *Receiver* is the Gateway which receives the measures from senders and returns a JSON
 * string which can directly be sent to an IoT platform like Thingsboard through sockets.
 */

#ifndef NIAGARA_MEASURE_H
#define NIAGARA_MEASURE_H

#include "niagara.h"
#include "str.h"
#include <vector>

/**
 * @brief Single-measure structure
 * @author Zanotti Enrico
 * @struct NiagaraMeasure
 * 
 * This class manages a single key-value pair
 * identifying a single measure, alongside a timestamp.
 */
struct NiagaraMeasure {
    str key;
    float value;
    unsigned long timestamp;

    /*
     * Default timestamp initialization as zero
     * (meaning no timestamp associated - use the time
     * when the measure is sent)
    */
    NiagaraMeasure(str k, float v, unsigned long ts = 0) 
        : key(k), value(v), timestamp(ts) {}
};

/**
 * @brief Sender class for the IoT device constructing the measure to send to the gateway
 * @class NiagaraSender
 * @author Zanotti Enrico
 * 
 * This class manages an array of timestamped `NiagaraMeasure` structures and allows for them to be sent
 * to a given destination identifier.
 */
class NiagaraSender {
private:
    Niagara& device; //Reference to the Niagara object
    std::vector<NiagaraMeasure> measures;

    // Internal helper for variadic addition
    void add_measures_internal() {}

    template<typename T, typename... Args>
    void add_measures_internal(T first, Args... rest) {
        add_measure(first);
        add_measures_internal(rest...);
    }

public:
    /**
     * Default constructor, initialises an empty sender with no measures
     * @param lora_device Reference to the LoRa device to use for message transmission
     */
    NiagaraSender(Niagara& lora_device);

    /**
     * Variadic constructor which initialises a sender with an array of measures
     * @param lora_device Reference to the LoRa device to use for message transmission
     * @param initial_measures Variadic amount of measures to initialise the sender with
     */
    template<typename... Args>
    NiagaraSender(Niagara& lora_device, Args... initial_measures) 
        : NiagaraSender(lora_device) 
    {
        add_measures_internal(initial_measures...);
    }

    /**
     * Adds a new measure to the sender
     * @param measure The new measure to add to this sender
     * 
     * *NOTE:* if a measure is added through this method which is already
     * present in the sender (meaning that the measure's key matches a measure
     * object already in this sender), the value of the measure already present 
     * will be updated, the passed measure will NOT be added as a separate one,
     * given that the measure key is the primary key.
     */
    void add_measure(const NiagaraMeasure& measure);
    /**
     * Removes a measure from the sender
     * @param key The key of the measure to remove
     */
    void remove_measure(const char* key);

    /**
     * Constructs the payload and sends the message
     * @param destination The destination identifier to send the message to
     * @param timestamp An optional timestamp to be included in the message, to avoid the problem of continuous reset of the function to get millis
     * @returns Any error code given in the message transmission, `0` if the message is sent succesfully
     */
    int send(const char* destination, unsigned long timestamp = 0);
};

/**
 * @brief Receiver class for the gateway constructing the JSON to be sent to the IoT platform
 * @author Zanotti Enrico
 * @class NiagaraReceiver
 * 
 * This class receives measures from remote devices
 */
class NiagaraReceiver {
private:
    Niagara& device; //Reference to the Niagara object
    str last_json;

public:
    /**
     * Default constructor, initialises a receiver given the Niagara object
     * @param lora_device Reference to the LoRa device to use for message reception
     */
    NiagaraReceiver(Niagara& lora_device);

    /**
     * Handler for reception, dynamic JSON parsing and timestamp correction from the transmitter.
     * @param error Pointer to an integer containing an error code which might have resulted 
     *              from message reception. This value is set to `0` in case no error occurred.
     * @param remote_device The string containing the remote device identifier. This parameter
     *                      is optional and can be set to nullptr in case the remote device is not
     *                      to be retrieved.
     * @returns A string containing the parsed JSON data ready to be sent to the IoT platform.
     *          In case an error occurred an empty string `""` is returned.
     * 
     * This method starts a message reception and listens for the first device which sent
     * data directed to this node.
     */
    str receive(int* error, str* remote_device = nullptr);
};

#endif