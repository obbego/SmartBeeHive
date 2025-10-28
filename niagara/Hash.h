#ifndef HASH_H
#define HASH_H

#include <stdint.h>
#include "str.h"

// Define the CRC32 polynomial used for the hash calculation
#define CRC32_POLYNOMIAL 0xEDB88320

// Function to compute CRC32 for a given data buffer
static uint32_t crc32(const str data, unsigned int length = 32) {
    uint32_t crc = 0xFFFFFFFF;  // Initial value of CRC is all 1's (0xFFFFFFFF)

    // Process each byte in the data buffer
    for (unsigned int i = 0; i < length; ++i) {
        crc ^= data[i];  // XOR the current byte with the current CRC value
        // Process each bit of the byte
        for (int j = 0; j < 8; ++j) {
            if (crc & 1) {
                crc = (crc >> 1) ^ CRC32_POLYNOMIAL;  // If the lowest bit is 1, shift and XOR
            } else {
                crc >>= 1;  // Otherwise, just shift
            }
        }
    }

    crc = ~crc; // Final inversion of CRC (as per CRC32 standard)
    //Convert the crc 32bit value into a char array
    return crc;
}

#endif // CRC32_H
