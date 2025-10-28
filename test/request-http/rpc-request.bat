SET DEVICE_ACCESS_TOKEN=tevWRCdJC7IYUPl3ha4F
SET THINGSBOARD_HOST_NAME=demo.thingsboard.io
SET DATA_FILE=rpc-data.json

curl.exe -X POST -d @%DATA_FILE% https://%THINGSBOARD_HOST_NAME%/api/v1/%DEVICE_ACCESS_TOKEN%/rpc --header "Content-Type:application/json"