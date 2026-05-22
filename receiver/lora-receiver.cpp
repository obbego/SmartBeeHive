#include <iostream>
#include <fstream>
#include <vector>
#include <mutex>
#include <thread>
#include <chrono> // library for time units
#include <algorithm>

#include <curl/curl.h>                    // library to handle HTTP requests
#include <spdlog/spdlog.h>                // library to handle logging
#include <spdlog/sinks/daily_file_sink.h> // daily logger sink
#include <spdlog/pattern_formatter.h>     // pattern formatter
#include <memory>                         // for shared_ptr

#include "niagara.h"                      // library to handle LoRa communication
#include "niagara_measure.h"              // library to handle measures in Niagara protocol

using namespace std;

/**
 * Class to define info of the devices that will be listened
 * with the LoRa receiver and then send their data
 * to the ThingsBoard platform
 * @author Daniele Chiarion
 */
class DeviceInfo
{
private:
    str identifier;
    str accessToken;

public:
    /**
     * Constructor of the DeviceInfo class,
     * with the identifier used in LoRa and the corresponding token
     * used in Thingsboard communication
     * @param deviceName the identifier use in LoRa communication
     * @param token access token used in ThingsBoard communication
     * @author Daniele Chiarion
     */
    DeviceInfo(str deviceName, str token)
    {
        this->identifier = deviceName;
        this->accessToken = token;
    }

    /**
     * Method to get the equals between two DeviceInfo objects,
     * in order to avoid duplicates or find elements inside a list
     * @param other other DeviceInfo to make the comparison
     * @return true if the identifiers are the same, false otherwise
     * @author Daniele Chiarion
     */
    bool operator==(const DeviceInfo &other) const
    {
        return this->identifier == other.identifier;
    }

    /**
     * Getter for access token
     * @return the access token of the device
     * @author Daniele Chiarion
     */
    str getAccessToken() const
    {
        return this->accessToken;
    }

    /**
     * Getter of the identifier of the
     * beehive device used in Niagara protocol
     * @return the identifier ot the device
     * @author Daniele Chiarion
     */
    str getDeviceIdentifier() const
    {
        return this->identifier;
    }
};

/* define constants for
the execution of the program */
const string DEVICES_FILE = "devices.txt";
const string POST_FILE = "post_data.json";
const string THINGSBOARD_HOST = "http://172.20.10.2:8080";
const int PERIODIC_REQUEST_TELEMETRIES = 5; // hours
/* define global variables in order to be used all over the program */
vector<DeviceInfo> devices;
/* define logger to register device operation.
Keep track of the last seven days of actions which will be stored. */
std::shared_ptr<spdlog::logger> logger;

/* create global niagara object and use
corresponding mutex to manage the access
between threads */
Niagara niagara;
mutex niagaraMutex;

/**
 * Function to init the logger using the name and file
 * name for different processes
 * @param loggerName name of the instance of the logger
 * @param loggerFileName name of the single rotating file of the logger
 * @return void
 * @author Daniele Chiarion
 */
void initLogger(string loggerName, string loggerFilename)
{
    try
    {
        logger = spdlog::daily_logger_mt(loggerName, loggerFilename, 0, 0, true, 7);
        logger->set_level(spdlog::level::debug);
    }
    catch (const spdlog::spdlog_ex &ex)
    {
        std::cerr << "Log initialization failed: " << ex.what() << std::endl;
    }
}

/**
 * Trims an std::string of all trailing/preceding invisible
 * characters.
 */
string trim(const string &s)
{
    auto start = s.find_first_not_of(" \n\r\t");
    auto end = s.find_last_not_of(" \n\r\t");
    return (start == string::npos) ? "" : s.substr(start, end - start + 1);
}

/**
 * Function to recover the devices information from a specified
 * file. They're important to establish a well made communication
 *
 * The file where to get the devices respect the following format:
 * DEVICE_NAME ACCESS_TOKEN
 *
 * @return true if there are some devices registered, false otherwise
 * @author Daniele Chiarion
 */
bool recoverDevices()
{
    ifstream file(DEVICES_FILE);
    if (!file)
    {
        logger->error("Error in opening file " + DEVICES_FILE);
        return false;
    }

    string identifier, token;
    /* operator reads every word and separated it from
    every space or backspace */
    while (file >> identifier >> token)
    {
        devices.push_back(DeviceInfo(trim(identifier).c_str(), trim(token).c_str()));
    }

    file.close();

    if (devices.empty())
    {
        logger->error("No devices registered.");
        return false;
    }
    return true;
}

/**
 * Function to send the data received from the LoRa receiver to
 * the ThingsBoard server using POST request
 * @param payload data received from LoRa communicatiom
 * @param source identifier for the source device
 * @return true if the data has been sent correctly, false otherwise
 * @author Daniele Chiarion
 */
bool sendDataToThingsBoard(str payload, str source)
{
    /* define boolean variable */
    bool check;

    /* recover the device from the source */
    auto it = find(devices.begin(), devices.end(), DeviceInfo(source, ""));
    if (it == devices.end()) // return false if the device has not been registered
        return false;

    /* start the HTTP POST request to ThingsBoard */
    CURL *curl = curl_easy_init();
    if (curl)
    {
        /* create the header of the request */
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type:application/json");

        /* configuration of all the parts of
        the request */
        string fullURL = THINGSBOARD_HOST + "/api/v1/" + string(it->getAccessToken().c_str()) + "/telemetry";
        cout << "Full URL: '" << fullURL << "'" << endl;
        curl_easy_setopt(curl, CURLOPT_URL, fullURL.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());

        cout << "Payload: " << payload.c_str() << endl;

        /* get the result of the request about the connection
        and data validity */
        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK)
        {
            logger->error("Error " + string(curl_easy_strerror(res)) + " in curl sending data to ThingsBoard using device " + string(source.c_str()));
            cout << "Error " << string(curl_easy_strerror(res)) << " in curl sending data to ThingsBoard using device " << string(source.c_str()) << endl;
            check = false;
        }
        else
        {
            /* get the http code of the request */
            long httpCode = 0;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
            if (httpCode == 200)
            {
                logger->info("Successfully sent data to ThingsBoard from device " + string(source.c_str()));
                cout << "Successfully sent data to ThingsBoard from device " << string(source.c_str()) << endl;
                check = true;
            }
            else
            {
                logger->error("Return code error " + to_string(httpCode) + " while sending data to ThingsBoard using device " + string(source.c_str()));
                cout << "Return code error " << to_string(httpCode) << " while sending data to ThingsBoard using device " << string(source.c_str()) << endl;
                check = false;
            }
        }

        /* close and clean the resource of curl request */
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);

        return check; // return curl code
    }
    else
    {
        logger->error("Error in creating curl connection.");
        cout << "Error in creating curl connection." << endl;
        return false;
    }
}

/***
 * Function to ask for telemetries to the current device
 * from the beehives using the Niagara protocol
 * @return void
 * @author Daniele Chiarion
 */
void askTelemetriesFromDevices()
{
    /* variable declaration */
    Niagara_Ret sendStatus;
    string message = "TELEMETRY_REQUEST";

    /* cycle all the devices registered and
    send the request */
    for (int i = 0; i < devices.size(); i++)
    {
        {
            lock_guard<mutex> lock(niagaraMutex); // lock the niagara instance
            sendStatus = niagara.send(devices[i].getDeviceIdentifier().c_str(), message.c_str());
        }

        if (sendStatus != NIAGARA_OK)
        {
            logger->error("Error in sending telemetry request to device " + string(devices[i].getDeviceIdentifier().c_str()) + " with code: " + to_string(sendStatus));
            cout << "Error in sending telemetry request to device " << string(devices[i].getDeviceIdentifier().c_str()) << " with code: " << to_string(sendStatus) << endl;
        }
    }
}

/**
 * Function used in a thread to start a continuous receiver which
 * constantly checks for new updates and send them into
 * the ThingsBoard platform registered
 * @param receiver the NiagaraReceiver object used to receive data from the LoRa communication and process it
 * @return return error code
 * @author Daniele Chiarion
 */
int thread_continuousReceiver(NiagaraReceiver &receiver)
{
    /* create an infinite loop to receive data and
    send them to ThingsBoard server */
    while (true)
    {
        str payload, source;
        int error; // error code return by the receiving method

        /* receive theS JSON data from the LoRa device */
        {
            lock_guard<mutex> lock(niagaraMutex);
            payload = receiver.receive(&error, &source);
        }
        if(error != 0){
            logger->error("Error in receiving data from LoRa communication with code: " + to_string(error));
            cout << "Error in receiving data from LoRa communication with code: " << to_string(error) << endl;
            continue; // skip the rest of the loop and try to receive again 
        }

        logger->info("Data received from " + string(source.c_str()));
        cout << "Data received from " << string(source.c_str()) << endl;

        /* send the data to ThingsBoard server.
        Try two times to send the message if there are some problems */
        bool send_thingsboard_check = false;
        for (int i = 0; i < 2 && send_thingsboard_check == false; i++)
            send_thingsboard_check = sendDataToThingsBoard(payload, source);
    }

    logger->error("Generic error occured in the continuous receiver thread");

    return 1;
}

/**
 * Function to run periodic requests for telemetries
 * to all the devices registered in the receiver.
 * This helps devices to know how often the telemetries
 * are pushed, in order to select the specific time to send them
 * and reduce noise for the bees and electricity consume
 * @return return error code
 * @author Daniele Chiarion
 */
int thread_periodTelemetryRequest()
{
    while (true)
    {
        askTelemetriesFromDevices();                                         // first ask for telemetries
        this_thread::sleep_for(chrono::hours(PERIODIC_REQUEST_TELEMETRIES)); // then sleep for the amount of hours required
    }

    logger->error("Generic error occured in the periodic telemetry request thread");

    return 1;
}

int main(int argc, char *argv[])
{
    /* initialize logger */
    initLogger("daily_logger", "logs/lora_receiver.log");

    /* start of the program */
    cout << "Starting of the LoRA receiver..." << endl;
    logger->info("Starting of the LoRA receiver...");

    niagara.set_identifier("LoRaREC"); // set identifier

    /* create Niagara receiver to handle 
    connection with the DC, milliseconds and data to 
    be sent to the IOT Platform */
    NiagaraReceiver receiver(niagara);

    /* setup the environment */
    if (!recoverDevices())
    {
        logger->error("Error in recovering devices from the file. LoRa receiver shuts down.");
        exit(1);
    }

    /* create the threads and start them
    right after the instantiaton */
    thread receiving_thread(thread_continuousReceiver, ref(receiver));
    //thread request_thread(thread_periodTelemetryRequest);

    /* join the threads when they end */
    if (receiving_thread.joinable())
        receiving_thread.join();
    /* if (request_thread.joinable())
        request_thread.join(); */

    return 0;
}
