#include <iostream>
#include <fstream>
#include <vector>

#include <curl/curl.h>     // library to handle HTTP requests
#include <spdlog/spdlog.h> // library to handle logging
#include <spdlog/sinks/daily_file_sink.h> // daily logger sink
#include <spdlog/pattern_formatter.h> // pattern formatter
#include <memory> // for shared_ptr
#include "niagara.h"      // library to handle LoRa communication

using namespace std;

/**
 * Class to define info of the devices that will be listened
 * with the LoRa receiver and then send their data
 * to the ThingsBoard platform
 */
class DeviceInfo
{
private:
    string identifier;
    string accessToken;

public:
    /**
     * Constructor of the DeviceInfo class,
     * with the identifier used in LoRa and the corresponding token
     * used in Thingsboard communication
     * @param deviceName the identifier use in LoRa communication
     * @param token access token used in ThingsBoard communication
     */
    DeviceInfo(string deviceName, string token)
    {
        this->identifier = deviceName;
        this->accessToken = token;
    }

    /**
     * Method to get the equals between two DeviceInfo objects,
     * in order to avoid duplicates or find elements inside a list
     * @param other other DeviceInfo to make the comparison
     * @return true if the identifiers are the same, false otherwise
     */
    bool operator==(const DeviceInfo &other) const
    {
        return this->identifier == other.identifier;
    }

    /**
     * Getter for access token
     * @return the access token of the device
     */
    string getAccessToken() const
    {
        return this->accessToken;
    }
};

/* define constants for
the execution of the program */
const string DEVICES_FILE = "devices.txt";
const string POST_FILE = "post_data.json";
const string THINGSBOARD_HOST = "http://localhost:8080";
/* define global variables in order to be used all over the program */
vector<DeviceInfo> devices;
/* define logger to register device operation. 
Keep track of the last seven days of actions which will be stored. */
std::shared_ptr<spdlog::logger> logger;

void initLogger() {
    try {
        logger = spdlog::daily_logger_mt("daily_logger", "logs/lora_receiver.log");
        logger->set_level(spdlog::level::debug);
    } catch (const spdlog::spdlog_ex &ex) {
        std::cerr << "Log initialization failed: " << ex.what() << std::endl;
    }
}

/**
 * Function to recover the devices information from a specified
 * file. They're important to establish a well made communication
 *
 * @return true if there are some devices registered, false otherwise
 */
bool recoverDevices()
{
    /* open the file */
    ifstream file(DEVICES_FILE);
    if (!file){
        logger->error("Error in opening file "+DEVICES_FILE+". Please check if the file exists and has the right path and permissions");
        return false;
    }

    /* every two words add a device file */
    int counter = 0;
    string token, firstAttribute;

    while (getline(file, token, ' '))
    {
        counter++;

        /* every two words add a device file */
        if (counter % 2 != 0)
            firstAttribute = token;
        else
            devices.push_back(DeviceInfo(firstAttribute, token));
    }
    file.close(); // close file

    if (counter == 0){
        logger->error("No devices registered. Please add some to continue.");
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
 */
bool sendDataToThingsBoard(string payload, string source)
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
        headers = curl_slist_append(headers, "Content-Type: application/json");

        /* configuration of all the parts of
        the request */
        curl_easy_setopt(curl, CURLOPT_URL, THINGSBOARD_HOST + "/api/v1/" + it->getAccessToken() + "telemetry");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());

        /* get the result of the request about the connection
        and data validity */
        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK)
        {
            logger->error("Error "+ string(curl_easy_strerror(res)) + " in curl sending data to ThingsBoard using device "+source);
            check = false;
        }
        else
        {
            /* get the http code of the request */
            long httpCode = 0;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
            if (httpCode == 200){
                logger->info("Successfully sent data to ThingsBoard from device "+source);
                check = true;
            }
            else {
                logger->error("Return code error "+to_string(httpCode)+" while sending data to ThingsBoard using device " + source);
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
        return false;
    }
}

int main(int argc, char *argv[])
{
    /* initialize logger */
    initLogger();
    
    /* start of the program */
    cout << "Starting of the LoRA receiver..." << endl;
    logger->info("Starting of the LoRA receiver...");

    /* initialize variables and objects for the
    execution of the program */
    Niagara nigara = Niagara();
    niagara->set_identifier("LoRa_rec"); // set identifier

    /* setup the environment */
    if (!recoverDevices()){
        logger->error("Error in recovering devices from the file. LoRa receiver shuts down.");
        exit(1);
    }

    /* create an infinite loop to receive data and
    send them to ThingsBoard server */
    while (true)
    {
        string payload, source;

        /* receive data from LoRa receiver and control
        the return code */
        Niagara_Ret niagara_status = niagara->receive(&payload, &source);
        if (niagara_status != NIAGARA_OK)
        {
            logger->error("Error in receiving data from "+source+" with error code: "+to_string(niagara_status));
            continue;
        }
        logger->info("Data received from "+source);

        /* send the data to ThingsBoard server.
        Try two times to send the message if there are some problems */
        bool send_thingsboard_check = false;
        for (int i = 0; i < 2 && send_thingsboard_check == false; i++)
            send_thingsboard_check = sendDataToThingsBoard(payload, source);
    }

    return 0;
}