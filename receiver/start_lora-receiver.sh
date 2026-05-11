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

# install cmake and make if not found
if ! command -v cmake &> /dev/null; then
    sudo apt install -y cmake
fi

if ! command -v make &> /dev/null; then
    sudo apt install -y make
fi

# install lgpio if not found
if [ ! -f "/usr/local/lib/liblgpio.so" ]; then
    echo "lgpio non trovata, procedo con l'installazione..."
    
    # Save current directory
    CURRENT_DIR=$(pwd)
    
    # Clone lg2 repository
    TMP_DIR=$(mktemp -d)
    cd "$TMP_DIR"
    git clone https://github.com/joan2937/lg.git
    cd lg
    
    # Compile and install
    make
    sudo make install
    
    # Cleanup and return to original directory
    cd "$CURRENT_DIR"
    rm -rf "$TMP_DIR"
    echo "lgpio installata con successo"
else
    echo "lgpio trovata nel sistema"
fi


# install RadioLib if not found
if [ ! -d "/usr/local/include/RadioLib" ]; then
    echo "RadioLib non trovata, procedo con l'installazione..."
    
    # Save current directory
    CURRENT_DIR=$(pwd)
    
    # Clone RadioLib repository
    TMP_DIR=$(mktemp -d)
    cd "$TMP_DIR"
    git clone https://github.com/jgromes/RadioLib.git
    cd RadioLib
    
    # Create build directory and compile
    mkdir build
    cd build
    cmake ..
    make
    sudo make install
    
    # Cleanup and return to original directory
    cd "$CURRENT_DIR"
    rm -rf "$TMP_DIR"
    echo "RadioLib installata con successo"
else
    echo "RadioLib trovata nel sistema"
fi


# compile the main script including the
# libraries used 
mkdir -p $OUTPUT_DIR
g++ "$SOURCE_FILE" "$NIAGARA_SOURCE" \
    ../niagara/AsyncDevice.cpp \
    ../niagara/fragmenter.cpp \
    ../niagara/measure.cpp \
    -std=c++17 \
    -I../niagara \
    -I/usr/local/include \
    -L/usr/local/lib \
    -o "$OUTPUT_DIR/$OUTPUT_FILE" \
    -lRadioLib -llgpio -lrt -lpthread -lspdlog -lcurl -lfmt

# start the code in background
echo "Eseguo $OUTPUT_FILE in background..."
nohup "$OUTPUT_DIR/$OUTPUT_FILE" > "$OUTPUT_DIR/output.log" 2>&1 &
echo "Processo lanciato con PID $!"