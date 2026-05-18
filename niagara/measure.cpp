#include "measure.h"

#include <stdlib.h>

Measure::Measure(const char *type, float v, unsigned long ts) : measureType(type), value(v), timestamp(ts), timestamp_set(true) {}
Measure::Measure(const char *type, float v) : measureType(type), value(v), timestamp(0), timestamp_set(false) {}

const char* Measure::getMeasureType(){
	return measureType.c_str();
}

float Measure::getValue(){
	return value;
}

unsigned long Measure::getTimestamp(){
	return timestamp;
}

void Measure::setValue(float v){
	value = v;
}

bool Measure::equals(Measure otherMeasure)
{
	return this->timestamp == otherMeasure.timestamp && measureType == otherMeasure.measureType;
}

str Measure::toJSON()
{
	if(timestamp_set) {
		// Retrieve the size of the string
		int bufSize = snprintf(NULL, 0, "{\"ts\":%lu, \"values\":{\"%s\":%f}}", timestamp, measureType.c_str(), value);
		if(bufSize < 0) return ""; // Return an empty string in case of error while handling JSON format
		bufSize++; //Include the null-termination
		char buffer[bufSize]; 
		snprintf(buffer, bufSize, "{\"ts\":%lu, \"values\":{\"%s\":%f}}", timestamp, measureType.c_str(), value);
		// Create a safe object string which contains the buffer contents
		return str(buffer);
	} else {
		// Retrieve the size of the string
		int bufSize = snprintf(NULL, 0, "{\"%s\":%f}", measureType.c_str(), value);
		if(bufSize < 0) return ""; // Return an empty string in case of error while handling JSON format
		bufSize++; //Include the null-termination
		char buffer[bufSize]; 
		snprintf(buffer, bufSize, "{\"%s\":%f}", measureType.c_str(), value);
		// Create a safe object string which contains the buffer contents
		return str(buffer);
	}
}

str Measure::formatMeasuresJSON(Measure array[], int size)
{
	str output; // Make use of strings to avoid memory allocation
	/* if the array is null or empty return empty brackets
	for the JSON format */
	if(array==nullptr || size == 0){
		output = "[]";
		return output.c_str();
	}

	/* start to concatenate strings */
	output = "[";
	for(int i=0;i<size;i++){
		output += array[i].toJSON(); //add the JSON format for the data
		if(i < size-1) // if it's not the last element, add a comma
			output += ",";
	}
	output += "]"; // end with the closing bracket

	return output; // return the c-string output of the used string
}