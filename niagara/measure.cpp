#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * Class to collect a measure with a specific
 * timestamp
 */
class Measure
{
private:
	char measureType[20];
	float value;
	unsigned long timestamp;
public:
	/**
	 * Constant value for the maximum length of the string
	 * that contains the JSON format of the measure
	 */
	static const int MAX_LENGTH_JSON_STRING = 60;

	/**
	 * Constructor for a measure
	 * @param type Type of measure (temperature, humidity, etc.)
	 * @param v Value of the measure
	 * @param ts Timestamp of the measure
	 */
	Measure(const char *type, float v, unsigned long ts)
	{
		strncpy(this->measureType, type, sizeof(measureType) - 1);
		this->measureType[sizeof(this->measureType) - 1] = '\0'; // Ensure null-termination
		this->value = v;
		this->timestamp = ts;
	}

	/**
	 * Function to return the physical type of measure
	 * @return string with the type of measure
	 */
	char* getMeasureType(){
		return this->measureType;
	}

	/**
	 * Return the value of measure
	 * @return value of the measure
	 */
	float getValue(){
		return this->value;
	}

	/**
	 * Return the timestamp of the measure
	 * @return timestamp of the measure
	 */
	unsigned long getTimestamp(){
		return this->timestamp;
	}

	/**
	 * Set a new value for the measure
	 * @param v new value for the measure
	 */
	void setValue(float v){
		this->value = v;
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
		sprintf(buffer, "{\"ts\":%lu, \"values\":{\"%s\":%f}}", timestamp, measureType, value);
		return buffer;
	}
};

/**
 * Function to format an array of measures as a JSON string. 
 * 
 * REMEMBER TO FREE THE RETURNED STRING AFTER THE USE. 
 * @param array array of measures
 * @param size size of the array
 * @return JSON string with the data of the measures
 */
const char *formatMeasuresJSON(Measure array[], int size)
{
	char *output = (char *)malloc(Measure::MAX_LENGTH_JSON_STRING * size + 2); // +2 for brackets
	/* if the array is null or empty return empty brackets
	for the JSON format */
	if(array==nullptr || size == 0){
		strcpy(output, "[]");
		return output;
	}

	/* start to concatenate strings */
	strcpy(output, "[");
	for(int i=0;i<size;i++){
		strcat(output, array[i].toJSON()); //add the JSON format for the data
		if(i<size-1) // if it's not the last element, add a comma
			strcat(output, ",");
	}
	strcat(output, "]"); // end with the closing bracket

	return output; // return the string
}