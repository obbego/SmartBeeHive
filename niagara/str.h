#ifndef CROSS_STRING_H
#define CROSS_STRING_H

#ifdef ARDUINO
  #include <Arduino.h>
  typedef String BaseString;
#else
  #include <string>
  typedef std::string BaseString;
#endif

class str {
private:
    BaseString s;

public:
    str() {}
    str(const char* str) : s(str ? str : "") {}
    str(const BaseString& other) : s(other) {}
    str(const str& other) : s(other.s) {}
	#ifdef ARDUINO
	str(int input) : s(input) {}
	#else
	str(int input) : s(std::to_string(input)) {}
	#endif

    str& operator=(const str& other) {
        s = other.s;
        return *this;
    }

    str& operator=(const char* str) {
        s = str ? str : "";
        return *this;
    }

    // Concatenation
    str operator+(const str& other) const {
        return str(s + other.s);
    }

    str& operator+=(const str& other) {
        s += other.s;
        return *this;
    }
    
    char operator[](size_t index) const {
        #if defined(ARDUINO)
            if(index < s.length()) return s[index];
            return '\0';
        #else
            if(index < s.size()) return s[index];
            return '\0';
        #endif
    }

    // Length
    size_t length() const {
    #ifdef ARDUINO
        return s.length();
    #else
        return s.size();
    #endif
    }

    // substring(start)
    str substring(size_t start) const {
    #ifdef ARDUINO
        return str(s.substring(start));
    #else
        if (start >= s.size()) return str("");
        return str(s.substr(start));
    #endif
    }

    // substring(start, end)
    str substring(size_t start, size_t end) const {
    #ifdef ARDUINO
        return str(s.substring(start, end));
    #else
        if (start >= s.size() || end <= start) return str("");
        if (end > s.size()) end = s.size();
        return str(s.substr(start, end - start));
    #endif
    }

    // indexOf(char)
    int indexOf(char c) const {
    #ifdef ARDUINO
        return s.indexOf(c);
    #else
        size_t pos = s.find(c);
        return (pos == std::string::npos) ? -1 : static_cast<int>(pos);
    #endif
    }

    int toInt() {
	#ifdef ARDUINO
		return s.toInt();
	#else
		int output;
		try {
			output = std::stoi(s);
		} catch(...) {
			return -1;	
		}
		return output;
	#endif
	}

    // indexOf(string, fromIndex)
    int indexOf(const str& other, size_t fromIndex = 0) const {
    #ifdef ARDUINO
        return s.indexOf(other.s, fromIndex);
    #else
        if (fromIndex >= s.size()) return -1;
        size_t pos = s.find(other.s, fromIndex);
        return (pos == std::string::npos) ? -1 : static_cast<int>(pos);
    #endif
    }

    // c_str()
    const char* c_str() const {
        return s.c_str();
    }

    // Comparison
    bool operator==(const str& other) const {
        return s == other.s;
    }

    bool operator!=(const str& other) const {
        return s != other.s;
    }
};

#endif
