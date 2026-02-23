#!/bin/bash

# Script to stop the LoRa receiver process
OUTPUT_FILE="lora-receiver.out"

# Kill the process by name
pkill -9 -f "$OUTPUT_FILE"

echo "LoRa receiver process stopped."
