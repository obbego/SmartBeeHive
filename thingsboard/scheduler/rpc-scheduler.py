from datetime import time, datetime
import threading
import subprocess

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
device_list = (
    DeviceInfo("device1", "dhfghsgdfhsdg"),
    DeviceInfo("device2", "dshjskldfksdfksh")
) # change the data of the device if you need to execute it

# define the hours to do the rpc request
# change the time if you need it
times_device-status = {
    time(1,0)
}
times_weight-beehive = {
    time(2,0)
}
last-time_device-status = datetime.now(), last-time_weight-beehive = datetime.now() # create variables to memorize last measure


def send_RPC_request(device, method, params):
    """Function to send and RPC request to a server 
    related to a specific device with the name of 
    the method and possible params"""

    global THINGSBOARD_HOST_NAME # importing global variables
