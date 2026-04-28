#include "niagara.h"
#include "Timer.h"

#include <cstdio>
#include <cstring>
#include <string>
#include <chrono>
#include <iomanip>
#include <sstream>

// Timestamped logger
void timestamp_logger(const char* text) {
    static std::string buffer;
    static bool at_line_start = true;

    auto get_timestamp = []() {
        using namespace std::chrono;

        auto now = system_clock::now();
        auto s = time_point_cast<std::chrono::seconds>(now);
        auto ms = duration_cast<milliseconds>(now - s).count();

        std::time_t tt = system_clock::to_time_t(s);
        std::tm tm{};
#if defined(_WIN32)
        localtime_s(&tm, &tt);
#else
        localtime_r(&tt, &tm);
#endif

        std::ostringstream oss;
        oss << std::put_time(&tm, "%H:%M:%S")
            << "." << std::setw(3) << std::setfill('0') << ms
            << " -> ";
        return oss.str();
    };

    buffer += text;

    size_t pos = 0;
    while ((pos = buffer.find('\n')) != std::string::npos) {
        std::string line = buffer.substr(0, pos);
        buffer.erase(0, pos + 1);

        if (at_line_start) {
            std::string ts = get_timestamp();
            std::fwrite(ts.c_str(), 1, ts.size(), stdout);
        }

        std::fwrite(line.c_str(), 1, line.size(), stdout);
        std::fwrite("\n", 1, 1, stdout);

        at_line_start = true;
    }

    // Remaining partial line (no newline yet)
    if (!buffer.empty()) {
        if (at_line_start) {
            std::string ts = get_timestamp();
            std::fwrite(ts.c_str(), 1, ts.size(), stdout);
            at_line_start = false;
        }

        std::fwrite(buffer.c_str(), 1, buffer.size(), stdout);
        buffer.clear();
    }

    std::fflush(stdout);
}

// Define the callback function for logging
void logger(const char* message) {
	printf(message);
}

int main(void) {
	//Initialise the device
	Niagara device(timestamp_logger, Niagara_LogLevel::LOG_TERMINAL);
	if(!device.set_identifier("RASPI")) {
		fprintf(stderr, "Error while setting identifier.\n");
		return 1;
	}
	
	//Variable to save error output
	Niagara_Ret error;
	//Send timer
	Timer send_timer;
	const unsigned int send_time = 5000;
	
	send_timer.start();
	
	//Start infinite loop
	for(;;) {
		/*
		//If the send timer has elapsed then send a new packet
		if(send_timer.elapsed() > send_time) {
			send_timer.start();
		
			error = device.send("ESP32", "Hello!");
			if(error != NIAGARA_OK) {
				fprintf(stderr, "Error while sending data: %d\n", static_cast<int>(error));
				continue;
			}
		}
		*/

		//Try to receive
		str receive;
		str source;
		error = device.receive(&receive, &source);
		if(error != NIAGARA_OK) {
			fprintf(stderr, "Error while receiving data: %d\n", static_cast<int>(error));
			continue;
		}

		printf("Received: %s\n", receive.c_str());
		break;
	}
}
