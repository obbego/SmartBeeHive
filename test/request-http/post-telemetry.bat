SET DEVICE_ACCESS_TOKEN=tevWRCdJC7IYUPl3ha4F
SET THINGSBOARD_HOST_NAME=demo.thingsboard.io
SET DATA_FILE=telemetry-data.json

curl.exe -v -X POST -d @%DATA_FILE% https://%THINGSBOARD_HOST_NAME%/api/v1/%DEVICE_ACCESS_TOKEN%/telemetry --header "Content-Type:application/json"