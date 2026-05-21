#ifndef CROSS_STRING_H
#define CROSS_STRING_H

#ifdef ARDUINO
  #include <Arduino.h>
  #include <cstdlib> // Used for atoll(const char*) in conversion from string to long number
  typedef String BaseString;
#else
  #include <string>
  typedef std::string BaseString;
#endif

#if defined(ARDUINO)
inline bool isValidInteger(String input) {
  boolean isNum=false;
  for(byte i=0;i<input.length();i++) {
    isNum = isDigit(input[i]) || input[i] == '+' || input[i] == '.' || input[i] == '-';
    if(!isNum) return false;
  }
  return isNum;
}
#endif

/**
 * @brief String class cross-compatible on Arduino and gcc compilers.
 * @file str.h
 * @class str
 * @author Zanotti Enrico
 * 
 * This class is used for all operations in the Niagara protocol
 * since it's cross-compatible in the platforms where the protocol operates.
 */
class str {
private:
    BaseString internalString;

public:
    /**
     * Default constructor, it creates an empty string.
     */
    str() {}
    /**
     * @param str The c-string used to create this string object
     * Istantiates this object given a c-string.
     */
    str(const char* str) : internalString(str ? str : "") {}
    /**
     * @param other Another platform-dependent string to create this string with
     * Istantiates this object with a platform-dependent string object
     */
    str(const BaseString& other) : internalString(other) {}
    /**
     * @param other Another `str` object
     * Istantiates this object with another `str` cross-compatible object
     */
    str(const str& other) : internalString(other.internalString) {}

    /**
     * @param input An long integer value to create the string with
     * Given an integer value, it creates an `str` object with the string representation of it.
     */
	#ifdef ARDUINO
	str(long long input) : internalString(input) {}
	#else
	str(long long input) : internalString(std::to_string(input)) {}
	#endif

    str& operator=(const str& other) {
        internalString = other.internalString;
        return *this;
    }

    str& operator=(const char* str) {
        internalString = str ? str : "";
        return *this;
    }

    // Concatenation
    str operator+(const str& other) const {
        return str(internalString + other.internalString);
    }

    str& operator+=(const str& other) {
        internalString += other.internalString;
        return *this;
    }
    str& operator+=(char newChar) {
        internalString += newChar;
        return *this;
    }

    char operator[](size_t index) const {
        #if defined(ARDUINO)
            if(index < internalString.length()) return internalString[index];
            return '\0';
        #else
            if(index < internalString.size()) return internalString[index];
            return '\0';
        #endif
    }

    /**
     * Gives the string's length
     * @returns The amount of characters contained in the string
     */
    size_t length() const {
    #ifdef ARDUINO
        return internalString.length();
    #else
        return internalString.size();
    #endif
    }

    /**
     * Computes a substring
     * @returns The substring of this string, trimmed at the given start position
     * @param start The index of the character where the substring returned should start
     */
    str substring(size_t start) const {
    #ifdef ARDUINO
        return str(internalString.substring(start));
    #else
        if (start >= internalString.size()) return str("");
        return str(internalString.substr(start));
    #endif
    }

    /**
     * Computes a substring
     * @param start The start index of the substring
     * @param end Index where the substring should stop
     * @returns A substring of this string, consisting of characters from index `start` to `stop - 1`
     */
    str substring(size_t start, size_t end) const {
    #ifdef ARDUINO
        return str(internalString.substring(start, end));
    #else
        if (start >= internalString.size() || end <= start) return str("");
        if (end > internalString.size()) end = internalString.size();
        return str(internalString.substr(start, end - start));
    #endif
    }

    /**
     * Finds the index of the passed character
     * @param search Character to search
     * @returns The first found index of the passed character
     */
    int indexOf(char search) const {
    #ifdef ARDUINO
        return internalString.indexOf(search);
    #else
        size_t pos = internalString.find(search);
        return (pos == std::string::npos) ? -1 : static_cast<int>(pos);
    #endif
    }

    /**
     * Converts this string to an integer
     * @returns The integer representing the contents of this string, 
     *          or `-1` if this string doesn't contain a valid integer.
     * 
     * This method is ideally used with positive integers because, although
     * it supports negative numbers, there is no difference in the output resulted
     * from the string `"-1"` and invalid string contents.
     */
    int toInt() {
	#ifdef ARDUINO
        if(!isValidInteger(internalString)) return -1;
		return internalString.toInt();
	#else
		int output;
		try {
			output = std::stoi(internalString);
		} catch(...) {
			return -1;	
		}
		return output;
	#endif
	}

    /**
     * @brief Converts this string to a long value
     * @returns The long long value representing the contents of this string, or
     *          `-1` if the string doesn't contain a valid long value.
     * 
     * This method is ideally used with positive numbers because, although negatives are supported, 
     * there is no difference in the output resulted from the string `"-1"` and invalid string contents.
     */
    long long toLong() {
        #ifdef ARDUINO
            if(!isValidInteger(internalString))
                return -1;

            return atoll(internalString.c_str());
        #else
            long long output;

            try {
                size_t pos;
                output = std::stoll(internalString, &pos);
                if(pos != internalString.length())
                    return -1;
            } catch(...) {
                return -1;
            }

            return output;
        #endif
    }

    /**
     * Finds the index of the passed string
     * @param other Character to search inside this string
     * @param fromIndex Index to start the search at, by default set to `0` (string start)
     * @returns The index of the first found occurrence of the passed string
     *          inside this string, starting from the start index
     */
    int indexOf(char search, size_t fromIndex) const {
    #ifdef ARDUINO
        return internalString.indexOf(search, fromIndex);
    #else
        size_t pos = internalString.find(search, fromIndex);
        return (pos == std::string::npos) ? -1 : static_cast<int>(pos);
    #endif
    }

    /**
     * Converts this string to a c-string (character array)
     */
    const char* c_str() const {
        return internalString.c_str();
    }

    bool operator==(const str& other) const {
        return internalString == other.internalString;
    }

    bool operator!=(const str& other) const {
        return internalString != other.internalString;
    }
};

#endif
