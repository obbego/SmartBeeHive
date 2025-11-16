DEVICE_ACCESS_TOKEN=BhP1QobnPngCTa3qjo6Z
THINGSBOARD_HOST_NAME=demo.thingsboard.io
DATA_FILE=rpc-data.json

curl -X POST -d @$DATA_FILE https://$THINGSBOARD_HOST_NAME/api/v1/$DEVICE_ACCESS_TOKEN/rpc --header "Content-Type:application/json"