/**
 * @file niagara_test.cpp
 * @brief Interactive test for the Niagara LoRa communication library.
 *
 * Compatible with Arduino and Raspberry Pi (Linux).
 * On Arduino:  flash this sketch, then use the Serial Monitor at 115200 baud.
 * On RPi/Linux: compile with g++ and run from the terminal.
 *
 *   g++ -std=c++17 -I<path/to/niagara> niagara_test.cpp -o niagara_test
 *   ./niagara_test
 */

#include "niagara.h"

// ─────────────────────────────────────────────────────────────────────────────
//  Platform abstraction
// ─────────────────────────────────────────────────────────────────────────────

#ifdef ARDUINO
    #define SERIAL_BEGIN(baud)  Serial.begin(baud); while (!Serial) {}
    #define PRINT(x)            Serial.print(x)
    #define PRINTLN(x)          Serial.println(x)
    #define PLATFORM_DELAY(ms)  delay(ms)

    // Read a full line from Serial (blocking)
    static str platform_readline() {
        while (Serial.available() == 0) { /* busy wait */ }
        String line = Serial.readStringUntil('\n');
        line.trim();
        return str(line.c_str());
    }

#else  // Raspberry Pi / Linux
    #include <cstdio>
    #include <cstring>
    #include <unistd.h>   // usleep

    #define SERIAL_BEGIN(baud)  /* nothing needed on Linux */
    #define PRINT(x)            printf("%s", x)
    #define PRINTLN(x)          printf("%s\n", x)
    #define PLATFORM_DELAY(ms)  usleep((ms) * 1000UL)

    static str platform_readline() {
        char buf[512] = {};
        if (fgets(buf, sizeof(buf), stdin)) {
            // strip trailing newline
            size_t len = strlen(buf);
            if (len && buf[len - 1] == '\n') buf[len - 1] = '\0';
        }
        return str(buf);
    }
#endif

// ─────────────────────────────────────────────────────────────────────────────
//  Log handler  (void(const char*) signature required by Niagara)
// ─────────────────────────────────────────────────────────────────────────────

void niagara_log(const char* msg) {
#ifdef ARDUINO
    Serial.print("[NIAGARA] ");
    Serial.println(msg);
#else
    printf("[NIAGARA] %s\n", msg);
#endif
}

// ─────────────────────────────────────────────────────────────────────────────
//  Helper: human-readable Niagara_Ret
// ─────────────────────────────────────────────────────────────────────────────

static const char* ret_to_str(Niagara_Ret r) {
    switch (r) {
        case NIAGARA_OK:                  return "OK";
        case NIAGARA_NO_IDENTIFIER:       return "NO_IDENTIFIER";
        case NIAGARA_INVALID_DATA:        return "INVALID_DATA";
        case NIAGARA_NO_DATA:             return "NO_DATA";
        case NIAGARA_TOO_LARGE:           return "TOO_LARGE";
        case NIAGARA_NOT_DESTINATION:     return "NOT_DESTINATION";
        case NIAGARA_TIMEOUT:             return "TIMEOUT";
        case RADIOLIB_ERROR:              return "RADIOLIB_ERROR";
        case NIAGARA_RECEIVE_ERROR:       return "RECEIVE_ERROR";
        case NIAGARA_RETRANSMISSION_ERROR:return "RETRANSMISSION_ERROR";
        case NIAGARA_SEND_ERROR:          return "SEND_ERROR";
        default:                          return "UNKNOWN";
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  Test helpers
// ─────────────────────────────────────────────────────────────────────────────

static int tests_run    = 0;
static int tests_passed = 0;

static void test_assert(const char* label, bool condition) {
    ++tests_run;
    if (condition) {
        ++tests_passed;
        PRINT("[PASS] "); PRINTLN(label);
    } else {
        PRINT("[FAIL] "); PRINTLN(label);
    }
}

static void print_separator() {
    PRINTLN("----------------------------------------");
}

// ─────────────────────────────────────────────────────────────────────────────
//  Test suites
// ─────────────────────────────────────────────────────────────────────────────

/**
 * @brief Tests set_identifier() with valid and invalid inputs.
 *        Runs without needing a remote device.
 */
static void test_identifier(Niagara& n) {
    PRINTLN("=== [TEST] set_identifier ===");

    // Valid callsigns
    test_assert("set_identifier: 4-char minimum (ABCD)",     n.set_identifier(str("ABCD")));
    test_assert("set_identifier: 12-char maximum (ABCDEF123456)", n.set_identifier(str("ABCDEF123456")));
    test_assert("set_identifier: alphanumeric mixed (TEST01)", n.set_identifier(str("TEST01")));

    // Invalid callsigns
    test_assert("set_identifier: too short (ABC) → false",   !n.set_identifier(str("ABC")));
    test_assert("set_identifier: too long (ABCDEF1234567) → false",
                                                              !n.set_identifier(str("ABCDEF1234567")));
    test_assert("set_identifier: reserved BROADCAST → false",!n.set_identifier(str("BROADCAST")));
    test_assert("set_identifier: special chars (AB-CD) → false",
                                                              !n.set_identifier(str("AB-CD")));
    test_assert("set_identifier: empty string → false",      !n.set_identifier(str("")));

    print_separator();
}

/**
 * @brief Tests that send/receive fail with NIAGARA_NO_IDENTIFIER
 *        when no identifier has been set yet.
 */
static void test_no_identifier_guard() {
    PRINTLN("=== [TEST] No-identifier guard ===");

    Niagara n;   // fresh instance, no identifier set

    str out;
    Niagara_Ret rx = n.receive(&out);
    test_assert("receive() without identifier → NO_IDENTIFIER",
                rx == NIAGARA_NO_IDENTIFIER);

    Niagara_Ret tx = n.send(str("DEST1"), str("hello"));
    test_assert("send() without identifier → NO_IDENTIFIER",
                tx == NIAGARA_NO_IDENTIFIER);

    print_separator();
}

/**
 * @brief Sends a single short message and reports the result.
 *        Requires a remote device ready to receive.
 */
static void test_send_single(Niagara& n, const str& destination) {
    PRINTLN("=== [TEST] Single-message send ===");
    PRINT("  Destination : "); PRINTLN(destination.c_str());

    str msg("Hello from Niagara test!");
    PRINT("  Payload     : "); PRINTLN(msg.c_str());

    Niagara_Ret ret = n.send(destination, msg);
    PRINT("  Result      : "); PRINTLN(ret_to_str(ret));
    test_assert("send single message → OK", ret == NIAGARA_OK);

    print_separator();
}

/**
 * @brief Sends a long message that should trigger fragmentation.
 */
static void test_send_fragmented(Niagara& n, const str& destination) {
    PRINTLN("=== [TEST] Fragmented-message send ===");

    // 300-character payload — well above a typical LoRa MTU
    const char long_payload[] =
        "FRAGMENT_TEST: Lorem ipsum dolor sit amet, consectetur adipiscing elit. "
        "Sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. "
        "Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi "
        "ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit.";

    PRINT("  Payload length: ");
    // print length without sprintf for Arduino compatibility
    int len = 0; while (long_payload[len]) ++len;
#ifdef ARDUINO
    Serial.println(len);
#else
    printf("%d\n", len);
#endif

    Niagara_Ret ret = n.send(destination, str(long_payload));
    PRINT("  Result        : "); PRINTLN(ret_to_str(ret));
    test_assert("send fragmented message → OK", ret == NIAGARA_OK);

    print_separator();
}

/**
 * @brief Sends a broadcast message (destination = "BROADCAST").
 */
static void test_send_broadcast(Niagara& n) {
    PRINTLN("=== [TEST] Broadcast send ===");

    Niagara_Ret ret = n.send(str("BROADCAST"), str("Broadcast test from Niagara!"));
    PRINT("  Result: "); PRINTLN(ret_to_str(ret));
    test_assert("send broadcast → OK", ret == NIAGARA_OK);

    print_separator();
}

/**
 * @brief Blocking receive loop: waits for up to `timeout_ms` milliseconds
 *        for an incoming message, then reports content and source.
 */
static void test_receive(Niagara& n, unsigned long timeout_ms) {
    PRINTLN("=== [TEST] Receive ===");
    PRINT("  Waiting up to ");
#ifdef ARDUINO
    Serial.print(timeout_ms / 1000);
#else
    printf("%lu", timeout_ms / 1000);
#endif
    PRINTLN(" seconds...");

    str output;
    str source;
    Niagara_Ret ret = NIAGARA_NO_DATA;

    unsigned long deadline = timeout_ms;
    const unsigned long poll_interval = 100; // ms

    while (deadline > 0) {
        ret = n.receive(&output, &source);
        if (ret != NIAGARA_NO_DATA) break;
        PLATFORM_DELAY(poll_interval);
        deadline = (deadline > poll_interval) ? (deadline - poll_interval) : 0;
    }

    PRINT("  Result : "); PRINTLN(ret_to_str(ret));

    if (ret == NIAGARA_OK) {
        PRINT("  Source : "); PRINTLN(source.c_str());
        PRINT("  Message: "); PRINTLN(output.c_str());
    }

    test_assert("receive message → OK", ret == NIAGARA_OK);
    print_separator();
}

/**
 * @brief Receive loop: keeps listening until the user interrupts (Arduino) 
 *        or sends an empty line (RPi).
 */
static void test_receive_loop(Niagara& n) {
    PRINTLN("=== [TEST] Continuous receive loop ===");
#ifdef ARDUINO
    PRINTLN("  Listening... Reset to exit.");
#else
    PRINTLN("  Listening... press Ctrl-C to exit.");
#endif

    while (true) {
        str output;
        str source;
        Niagara_Ret ret = n.receive(&output, &source);

        if (ret == NIAGARA_OK) {
            PRINT("  [RX] From: "); PRINT(source.c_str());
            PRINT(" | Msg: ");     PRINTLN(output.c_str());
        } else if (ret != NIAGARA_NO_DATA) {
            PRINT("  [RX] Error: "); PRINTLN(ret_to_str(ret));
        }

        PLATFORM_DELAY(50);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  Interactive menu
// ─────────────────────────────────────────────────────────────────────────────

static void print_menu() {
    PRINTLN("");
    PRINTLN("╔══════════════════════════════════════╗");
    PRINTLN("║       NIAGARA  TEST  TERMINAL        ║");
    PRINTLN("╠══════════════════════════════════════╣");
    PRINTLN("║ 1) Run identifier unit tests         ║");
    PRINTLN("║ 2) Test: send single message         ║");
    PRINTLN("║ 3) Test: send fragmented message     ║");
    PRINTLN("║ 4) Test: send broadcast              ║");
    PRINTLN("║ 5) Test: receive (30 s timeout)      ║");
    PRINTLN("║ 6) Continuous receive loop           ║");
    PRINTLN("║ 7) Print test summary                ║");
    PRINTLN("╚══════════════════════════════════════╝");
    PRINT("Choice: ");
}

// ─────────────────────────────────────────────────────────────────────────────
//  Arduino entry points
// ─────────────────────────────────────────────────────────────────────────────

#ifdef ARDUINO

Niagara niagara(niagara_log, NIAGARA_LOG_DEBUG);   // adjust log level as needed
static bool identifier_set = false;

void setup() {
    SERIAL_BEGIN(115200);
    PRINTLN("Niagara test started.");

    // Ask for callsign
    PRINT("Enter your callsign (4-12 alphanumeric chars): ");
    str callsign = platform_readline();
    PRINTLN(callsign.c_str());

    if (niagara.set_identifier(callsign)) {
        PRINT("Callsign set: "); PRINTLN(callsign.c_str());
        identifier_set = true;
    } else {
        PRINTLN("Invalid callsign! Defaulting to TESTDEV.");
        niagara.set_identifier(str("TESTDEV"));
        identifier_set = true;
    }

    test_no_identifier_guard();
    print_menu();
}

void loop() {
    if (Serial.available() == 0) return;

    char choice = Serial.read();
    // flush rest of line
    while (Serial.available()) Serial.read();

    str dest;

    switch (choice) {
        case '1':
            test_identifier(niagara);
            break;
        case '2':
            PRINT("Destination callsign: ");
            dest = platform_readline();
            PRINTLN(dest.c_str());
            test_send_single(niagara, dest);
            break;
        case '3':
            PRINT("Destination callsign: ");
            dest = platform_readline();
            PRINTLN(dest.c_str());
            test_send_fragmented(niagara, dest);
            break;
        case '4':
            test_send_broadcast(niagara);
            break;
        case '5':
            test_receive(niagara, 30000);
            break;
        case '6':
            test_receive_loop(niagara);   // blocking
            break;
        case '7': {
            PRINTLN("=== Summary ===");
            PRINT("Passed: "); Serial.print(tests_passed);
            PRINT(" / ");      Serial.println(tests_run);
            break;
        }
        default:
            PRINTLN("Unknown option.");
    }

    print_menu();
}

// ─────────────────────────────────────────────────────────────────────────────
//  Linux / Raspberry Pi entry point
// ─────────────────────────────────────────────────────────────────────────────

#else  // Linux

int main() {
    Niagara niagara(niagara_log, NIAGARA_LOG_DEBUG);   // adjust log level as needed

    PRINTLN("=== Niagara LoRa test program ===");

    // ── Set callsign ──────────────────────────────────────────────────────────
    PRINT("Enter your callsign (4-12 alphanumeric chars): ");
    str callsign = platform_readline();

    if (!niagara.set_identifier(callsign)) {
        PRINTLN("Invalid callsign! Defaulting to TESTDEV.");
        niagara.set_identifier(str("TESTDEV"));
    } else {
        PRINT("Callsign set: "); PRINTLN(callsign.c_str());
    }

    // ── No-identifier guard (uses a separate instance) ────────────────────────
    test_no_identifier_guard();

    // ── Interactive loop ──────────────────────────────────────────────────────
    while (true) {
        print_menu();

        str choice_str = platform_readline();
        if (choice_str.c_str()[0] == '\0') continue;
        char choice = choice_str.c_str()[0];

        str dest;

        switch (choice) {
            case '1':
                test_identifier(niagara);
                break;
            case '2':
                PRINT("Destination callsign: ");
                dest = platform_readline();
                test_send_single(niagara, dest);
                break;
            case '3':
                PRINT("Destination callsign: ");
                dest = platform_readline();
                test_send_fragmented(niagara, dest);
                break;
            case '4':
                test_send_broadcast(niagara);
                break;
            case '5':
                test_receive(niagara, 30000);
                break;
            case '6':
                test_receive_loop(niagara);   // blocking — Ctrl-C to exit
                break;
            case '7':
                printf("=== Summary ===\nPassed: %d / %d\n", tests_passed, tests_run);
                break;
            default:
                PRINTLN("Unknown option, try again.");
        }
    }

    return 0;
}

#endif // ARDUINO