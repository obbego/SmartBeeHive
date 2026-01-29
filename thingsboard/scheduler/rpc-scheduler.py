from datetime import time, datetime, timedelta
import threading
import subprocess
import json
import time as t

class DeviceInfo:
    """Class to gather information about
    the device to be connected with in thingsboard"""

    def __init__(self, deviceName, accessToken):
        """Constructor of the device"""
        self.deviceName = deviceName
        self.accessToken = accessToken

    def __eq__(self, other):
        """Define equals method for the 
        device when it has the same device name"""

        if not isinstance(other, DeviceInfo):
            return False

        return self.deviceName == other.deviceName
    
"""Define all the constants and the variables 
to interact with Thingsboard server"""
THINGSBOARD_HOST_NAME = "demo.thingsboard.io" # change this with the real host name
JSON_FILENAME = "rpc-data.json"
device_list = (
    DeviceInfo("beehive1", "tevWRCdJC7IYUPl3ha4F"),
    DeviceInfo("beehive2", "BhP1QobnPngCTa3qjo6Z")
) # change the data of the device if you need to execute it

# define the hours to do the rpc request
# change the time if you need it
timesCheckWeightBeehive = sorted({
    time(12,43)
})
timesCheckDeviceStatus = sorted({
    time(12,44)
})
# create variables to memorize last measure 
lastTimeCheckWeightBeehive = datetime.now()
lastTimeCheckDeviceStatus = datetime.now()


def send_RPC_request(device, method, params):
    """Function to send and RPC request to a server 
    related to a specific device with the name of 
    the method and possible params"""

    global THINGSBOARD_HOST_NAME, JSON_FILENAME # importing global variables

    # save the method and the params in the file
    with open(JSON_FILENAME, "w") as file:
        content = {"method":method, "params":params}
        json.dump(content, file, indent=4, ensure_ascii=False) # write the converted element into the file
  
    # define the command to send
    command = ["curl", "-v", "-X", "POST", "-d", f"@{JSON_FILENAME}", f"https://{THINGSBOARD_HOST_NAME}/api/v1/{device.accessToken}/rpc", "--header", "Content-Type:application/json"]

    result = subprocess.run(command, capture_output=True, text=True) # return the result of the rpc request
    print(result)
    return result

def isTime(time_list, last_time):
    """Function to check if the current time corresponds
    to one of the times indicated, considering last time an action was done"""
    
    currentTime = datetime.now() # get the current datetime

    for singleTime in time_list:
        # control if today the time is passed
        # or if it's a time not resolved yesterday
        if (currentTime.time() >= singleTime and datetime.combine(datetime.now().date(), singleTime)>last_time) or (last_time < datetime.combine((datetime.today() - timedelta(days=1)), singleTime)):
            return True
        
    return False

def scheduler_checkWeightBeehive():
    """Function to schedule the time to send
    RPC requests to check the weight of the beehive"""
    while True:
        try:
            global timesCheckWeightBeehive, lastTimeCheckWeightBeehive, device_list # importing global variables

            if not isTime(timesCheckWeightBeehive, lastTimeCheckWeightBeehive): # first control if it's time
                t.sleep(1)
                continue
            
            # then send request for each device
            for singleDevice in device_list:
                send_RPC_request(singleDevice, "check-weight-beehive", {})

            lastTimeCheckWeightBeehive = datetime.now() # update last time variable
        except Exception as e:
            print(f"Errore nello scheduler del peso {e}")

def format_device_averages(array, result, device):
    """Function to format the result come from a device to 
    the appropriate JSON format established
    for RPC communication"""

    # get a dictionnaire format
    if not isinstance(result, dict):
        result = json.loads(result)

    # get the keys and create an object for each one
    for singleKey in result.keys():
        array.append({
            "deviceName":device.deviceName,
            "key":singleKey,
            "value":result[singleKey]
        })


def scheduler_checkDeviceStatus():
    """Function to scheduler time to send requests to control
    the status of the device and its sensors"""
    while True:
        try:
            global timesCheckDeviceStatus, lastTimeCheckDeviceStatus, device_list # importing global variables

            if not isTime(timesCheckDeviceStatus, lastTimeCheckDeviceStatus): # first control if it's time
                t.sleep(1)
                continue
            
            # first send request to gather the
            # average for each telemetry of each device
            average_list = [] # initialise a variable to gather the averages
            for singleDevice in device_list:
                rpc_reply = send_RPC_request(singleDevice, "check-timeseries-average", {}) # get the reply
                format_device_averages(average_list, rpc_reply.stdout, singleDevice) # add the rpc reply to the average list in the proper format
            
            # then for each device send the request
            # to check the status
            for singleDevice in device_list:
                send_RPC_request(singleDevice, "check-device-status", {"timeseriesAverage":average_list})

            lastTimeCheckDeviceStatus = datetime.now() # update the last time variable
        except Exception as e:
            print(f"Errore nello scheduler del dispositivo {e}")

"""Define the thread to start"""
thread_deviceScheduler = threading.Thread(target=scheduler_checkDeviceStatus, daemon=False)
thread_weightScheduler = threading.Thread(target=scheduler_checkWeightBeehive, daemon=False)

thread_weightScheduler.start()
thread_deviceScheduler.start()

thread_weightScheduler.join()
thread_deviceScheduler.join()