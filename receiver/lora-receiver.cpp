#include <iostream>
#include <fstream>
#include <vector>

#include <curl/curl.h>

using namespace std;

/**
 * Class to define info of the devices that will be listened
 * with the LoRa receiver and then send their data
 * to the ThingsBoard platform
 */
class DeviceInfo {
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
        DeviceInfo(string deviceName, string token){
            this->identifier = deviceName;
            this->accessToken = token;
        }

        /**
         * Method to get the equals between two DeviceInfo objects, 
         * in order to avoid duplicates or find elements inside a list
         * @param other other DeviceInfo to make the comparison
         * @return true if the identifiers are the same, false otherwise
         */
        bool operator==(const DeviceInfo& other) const {
            return this->identifier == other.identifier;
        }
};

/* define constants for 
the execution of the program */
#define DEVICES_FILE "devices.txt"
/* define global variables in order to be used all over the program */
vector<DeviceInfo> devices;

/**
 * Function to recover the devices information from a specified
 * file. They're important to establish a well made communication
 * 
 * @return true if there are some devices registered, false otherwise
 */
bool recoverDevices(){
    /* open the file */
    ifstream file(DEVICES_FILE);
    if (!file)
        return false;

    /* every two words add a device file */
    int counter = 0;
    string token, firstAttribute;

    while(getline(file, token, ' ')){
        counter++;

        /* every two words add a device file */
        if(counter % 2 != 0)
            firstAttribute = token;
        else
            devices.push_back(DeviceInfo(firstAttribute, token));
    }

    if(counter == 0)
        return false;

    return true;
}

