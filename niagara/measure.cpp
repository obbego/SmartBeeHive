#include <string.h>
#include <stdio.h>

/**
 * Class to collect a measure with a specific
 * timestamp
 */
class Measure
{
private:
    char measureType[50];
    float value;
    unsigned long timestamp;

public:
    static const int MAX_LENGTH_JSON_STRING = 100;

    /**
     * Constructor for a measure
     * @param type Type of measure (temperature, humidity, etc.)
     * @param v Value of the measure
     * @param ts Timestamp of the measure
     */
    Measure(char *type, float v, unsigned long ts)
    {
        strncpy(this->measureType, type, sizeof(measureType) - 1);
        this->measureType[sizeof(this->measureType) - 1] = '\0'; // Ensure null-termination
        this->value = v;
        this->timestamp = ts;
    }

    /**
     * Equals to compare two measures.
     * Two measures are equal if they have the same timestamp and measure type.
     * @param otherMeasure Measure to compare with
     * @return true if the measures are equal, false otherwise
     */
    bool equals(Measure otherMeasure)
    {
        return this->timestamp == otherMeasure.timestamp && strcmp(this->measureType, otherMeasure.measureType) == 0;
    }

    /**
     * function to convert the measure to a JSON string
     * @return JSON string with the data of the measure
     */
    const char *toJSON()
    {
        static char buffer[MAX_LENGTH_JSON_STRING];
        sprintf(buffer, "{%s: %f, \"timestamp\": %lu}", measureType, value, timestamp);
        return buffer;
    }
};

const char *formatMeasures(Measure array[], int size)
{
    //static char buffer[Measure::MAX_LENGTH_JSON_STRING * size + 2];
}