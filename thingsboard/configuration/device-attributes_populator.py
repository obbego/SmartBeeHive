import subprocess
import json

# define constants for the execution
# of the program
THINGSBOARD_HOST_NAME = "demo.thingsboard.io" # change this with the real host name
CONFIG_FILENAME = "allowed_keys.json" # file where keys are saved
POST_FILENAME = "post_data.json" # file where to save data for the POST request
THINGSBOARD_ATTRIBUTE = "allowed_telemetry_keys" # name of the attribute where to save the list

def load_configuration():
    """Function to load the configuration
    from the json file given and return it to
    send the requests"""

    try:
        with open(CONFIG_FILENAME, "r") as file:
            return json.load(file)
    except json.JSONDecodeError as e:
        raise json.JSONDecodeError(f"Error while decoding JSON: {e}")
    except Exception as e:
        raise Exception(f"General error while decoding JSON file {e}")
        

def send_post_request(device:str, keys:list):
    """Function to send the POST request to the
    ThingsBoard server requiring the keys to be saved 
    with the corresponding access token of the device"""

    # write the content to be sent
    # into the JSON file
    try:
        with open(POST_FILENAME, "w") as file:
            content = {
                THINGSBOARD_ATTRIBUTE: json.dumps(keys)
            }
            file.write(json.dumps(content))
    except Exception as e:
        raise Exception(f"General error while encoding JSON file {e}")

    # define the commmand to send
    command = ["curl", "-v", "-X", "POST", "-d", f"@{POST_FILENAME}", f"https://{THINGSBOARD_HOST_NAME}/api/v1/{device}/attributes", "--header", "Content-Type:application/json"]
    subprocess.run(command, capture_output=False, text=True) # send the POST request

def main():
    """Function to handle the allowed keys written
    on a file and send the POST request to the ThingsBoard server
    where to save the allowed keys"""

    print("Start intial device attributes populator...\n")
    keys_for_device = load_configuration() # get the keys for the device
    print("Gathered data from the file...\n")

    for single_config in keys_for_device:
        # get the lists of keys and devices
        key_list = single_config['keys']
        device_list = single_config['devices']

        # cycle all the device to send
        # the attribute with the allowed keys
        # for the telemetry
        for single_device in device_list:
            send_post_request(single_device, key_list)

    print("+++ Tasks done.")

if __name__ == "__main__":
    main()