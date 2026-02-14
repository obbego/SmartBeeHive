#!/bin/bash

# defining variables
SOURCE_FILE="lora-receiver.cpp"
OUTPUT_DIR="./out"
OUTPUT_FILE="lora-receiver.out"

NIAGARA_SOURCE="../niagara/niagara.cpp"
NIAGARA_INCLUDE="../niagara/niagara.h"
NIAGARA_BUILD="../niagara/niagara.out"

# install necessary libraries if thet
# haven't been installed yet
if ! dpkg -s libcurl4-openssl-dev >/dev/null 2>&1; then
    sudo apt install -y libcurl4-openssl-dev
fi

if ! dpkg -s libspdlog-dev >/dev/null 2>&1; then
    sudo apt install -y libspdlog-dev
fi

# compile custom libraries useful for the main script
g++ $NIAGARA_SOURCE -o $NIAGARA_BUILD -llgpio -lRadioLib

# compile the main script including the
# libraries used 
mkdir -p $OUTPUT_DIR
g++ "$SOURCE_FILE" -std=c++17 \
    -I/usr/include/spdlog \
    -I"$NIAGARA_INCLUDE" \
    -L"$NIAGARA_BUILD" -lniagara \
    -lcurl \
    -o "$OUTPUT_DIR/$OUTPUT_FILE"

# start the code in background
echo "Eseguo $OUTPUT_FILE in background..."
nohup "$OUTPUT_DIR/$OUTPUT_FILE" > "$OUTPUT_DIR/output.log" 2>&1 &
echo "Processo lanciato con PID $!"