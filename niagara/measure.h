#ifndef MEASURE_H
#define MEASURE_H
#include "str.h"

class Measure {
    private:
        const str measureType;
        float value;
        unsigned long timestamp;
        bool timestamp_set;

    public:
        /**
         * Constructor for a new measure with manual timestamp
         * 
         * @param type The c-str containing the measure type (`temperature`, `humidity`, etc.) 
         * @param value The value of the measure described by `type`
         * @param timestamp The timestamp of this measure
         */
        Measure(const char* type, float value, unsigned long timestamp);

        /**
         * Constructor for a new measure with no timestamp
         * 
         * @param type The c-str containing the measure type (`temperature`, `humidity`, etc.) 
         * @param value The value of the measure described by `type`
         */
        Measure(const char *type, float value);

        /**
         * Getter for the measure type string.
         * 
         * @return A c-string constant containing the measure type
         */
        const char* getMeasureType();
        /**
         * Getter for the measure value
         * 
         * @return The value of this measure
         */
        float getValue();
        /**
         * Getter for the measure's timestamp
         * 
         * @return The timestamp of this measure
         */
        unsigned long getTimestamp();

        /**
         * Setter for the measure's value
         * 
         * @param newVal The new measure's value
         */
        void setValue(float newVal);
        /**
         * Comparator for measures
         * 
         * @param other The other measure to compare this measure with
         * @return `true` In case this measure and the other passed are equal
         */
        bool equals(Measure other);

        /**
         * Converts this measure to a JSON string.
         * 
         * @return The JSON representation of this measure
         */
        str toJSON();

        /**
         * Function to format an array of measures as a JSON string. 
         * 
         * @param array array of measures
         * @param size size of the array
         * @return JSON string with the data of the measures
         */
        static str formatMeasuresJSON(Measure measures[], int size);
};

#endif