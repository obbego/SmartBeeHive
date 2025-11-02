SET DEVICE_ACCESS_TOKEN=tevWRCdJC7IYUPl3ha4F
SET DEVICE_ID=04de50a0-b27d-11f0-86b8-4d4474c0a366
SET THINGSBOARD_HOST_NAME=demo.thingsboard.io
SET DATA_FILE=rpc-data.json
SET JWT_TOKEN=Your_Token

REM Send RPC request
curl.exe -X POST -d @%DATA_FILE% https://%THINGSBOARD_HOST_NAME%/api/v1/%DEVICE_ACCESS_TOKEN%/rpc --header "Content-Type:application/json"

REM view the server attributes of the device
REM curl.exe -s -X GET "https://%THINGSBOARD_HOST_NAME%/api/plugins/telemetry/DEVICE/%DEVICE_ID%/values/attributes?scope=SERVER_SCOPE" -H "Content-Type: application/json" -H "X-Authorization: Bearer %JWT_TOKEN%"