from datetime import time, datetime
import threading
import subprocess
import json

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
THINGSBOARD_HOST_NAME = "localhost" # change this with the real host name
JSON_FILENAME = "rpc-data.json"
device_list = (
    DeviceInfo("device1", "dhfghsgdfhsdg"),
    DeviceInfo("device2", "dshjskldfksdfksh")
) # change the data of the device if you need to execute it

# define the hours to do the rpc request
# change the time if you need it
times_device-status = sorted({
    time(1,0)
})
times_weight-beehive = sorted({
    time(2,0)
})
last-time_device-status = datetime.now(), last-time_weight-beehive = datetime.now() # create variables to memorize last measure


def send_RPC_request(device, method, params):
    """Function to send and RPC request to a server 
    related to a specific device with the name of 
    the method and possible params"""

    global THINGSBOARD_HOST_NAME, JSON_FILENAME # importing global variables

    # save the method and the params in the file
    with open(JSON_FILENAME, "w") as file:
        content = f'{{"method":{method}, "params":{params}}}'
        json.dump(content, file, indent=4, ensure_ascii=False) # write the converted element into the file
  
    # define the command to send
    command = ["curl", "-v", "-X", "POST", f"@{JSON_FILENAME}", f"https://{THINGSBOARD_HOST_NAME}/api/v1/{device.accessToken}/rpc", "--header", "Content-Type:application/json"]

    return subprocess.run(command, capture_output=True, text=True) # return the result of the rpc request

def isTime(time_list, last_time):
    """Function to check if the current time corresponds
    to one of the times indicated, considering last time an action was done"""
    
    currentTime = datetime.now() # get the current datetime

    for singleTime in time_list:
        # CONDITION TO BE FINISHED
        if currentTime.time() >= singleTime and datetime.combine(datetime.now().date(), singleTime):
            pass

def weightBeehiveScheduler():
    pass