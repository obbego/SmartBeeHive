#include <iostream>
#include <queue>
#include <string>
#include <chrono>
#include <thread>

//Timer gia' presente
#include "Timer.h"
//Funzioni di hashing CRC32
#include "Hash.h"

// ---- Minimal stubs & helpers to run on PC ----
enum Niagara_Ret {
  NIAGARA_OK,
  NIAGARA_TIMEOUT,
  NIAGARA_NO_IDENTIFIER,
  NIAGARA_RECEIVE_ERROR,
  NIAGARA_SEND_ERROR,
  NIAGARA_RETRANSMISSION_ERROR,
  NIAGARA_INVALID_DATA,
  NIAGARA_NOT_DESTINATION,
  RADIOLIB_ERROR
};

enum Niagara_Control {
  SYN = 1,
  ACK = 2,
  RETRANSMISSION_TIMEOUT = 3
};

static constexpr int MAX_RECV_WAIT = 2000; // ms
static constexpr int NIAGARA_RETRANSMISSIONS = 3;

// ---- Fake radio layer ----

struct Frame {
  str src;
  Niagara_Control ctrl;
  str payload;
};

static std::queue<Frame> fake_air;

// ---- Niagara mock ----
struct Niagara {
    static str identifier;

    // Stato interno per simulare handshake in receive_raw
    static bool simulate_next_syn;

    static str format_message(const str &dst, Niagara_Control c, const str &msg) {
        return identifier + ";" + std::to_string(c) + ";" + msg;
    }

    static void process_message(str out[3], const str &in) {
        size_t p1 = in.indexOf(';');
        size_t p2 = in.substring(p1 + 1).indexOf(';') + p1 + 1;
        out[0] = in.substring(0, p1);
        out[1] = in.substring(p1 + 1, p2 - p1 - 1);
        out[2] = in.substring(p2 + 1);
    }

    // ---- Fake send_raw ----
    static Niagara_Ret send_raw(str destination, Niagara_Control control, str message) {
        std::cout << "[TX] to=" << destination.c_str()
                  << " ctrl=" << control
                  << " payload='" << message.c_str() << "'" << std::endl;

        // Inserisce il pacchetto nella coda della "radio"
        fake_air.push({identifier, control, message});

        // Simula handshake 3-way: SYN → remote risponde con ACK+CRC
        if(control == SYN) {
            str crc = crc32_to_str(crc32(message));
            fake_air.push({destination, ACK, crc});
        }

        return NIAGARA_OK;
    }

    // ---- Fake receive_raw ----
    static Niagara_Ret receive_raw(str* source, Niagara_Control* control, str* message) {
        // If the fake queue has something, pop it first (simulate normal receive)
        if (!fake_air.empty()) {
            Frame f = fake_air.front();
            fake_air.pop();

            *source = f.src;
            *control = f.ctrl;
            *message = f.payload;

            std::cout << "[RX] from=" << (*source).c_str()
                      << " ctrl=" << *control
                      << " payload='" << (*message).c_str() << "'" << std::endl;
            return NIAGARA_OK;
        }

        // Otherwise simulate the handshake
        if (simulate_next_syn) {
            // Send a SYN from remote device
            *source = "NODE_B";
            *control = SYN;
            *message = "Hello Niagara";

            std::cout << "[RX SIM] from=" << (*source).c_str()
                      << " ctrl=" << *control
                      << " payload='" << (*message).c_str() << "'" << std::endl;

            // Next time we should simulate the ACK
            simulate_next_syn = false;
            return NIAGARA_OK;
        } else {
            // Simulate ACK with CRC of the payload
            *source = "NODE_B";
            *control = ACK;
            *message = "";

            std::cout << "[RX SIM] from=" << (*source).c_str()
                      << " ctrl=" << *control
                      << " payload='" << (*message).c_str() << "'" << std::endl;

            // Reset for next handshake
            simulate_next_syn = true;
            return NIAGARA_OK;
        }
    }

    static Niagara_Ret send(str destination, str message);
    static Niagara_Ret receive(str* output, str* source);
};
bool Niagara::simulate_next_syn = true;

str Niagara::identifier;

// ---- Include your already-reviewed send/receive implementations ----
Niagara_Ret Niagara::receive(str* output, str* source) {
  //Check if the identifier hasn't yet been initialised
  if(identifier.length() == 0)
      return NIAGARA_NO_IDENTIFIER;

  //Saves the current receive state of the handshake  
  enum class RxState {
    WAIT_SYN,
    WAIT_FINAL_ACK
  };

  //Variable which keeps track of the handshake's state
  RxState state = RxState::WAIT_SYN;
  //Timer used to keep track of timeouts
  Timer session_timer;

  //The remote device's identifier to communicate with
  str device = "";
  //Buffer for the payload
  str payload;
  //Buffer for the output message
  str output_message;
  //Buffer for the control message
  Niagara_Control control;
  //Return status of send and receive methods
  Niagara_Ret status;

  //Start the timer to count for timeouts
  session_timer.start();

  //Keep reiterating until the amount of retransmissions reaches the maximum amount of retransmissions
  while(true) {
    //Check if the session time has exceeded the timeout in which case stop the communication
    if(session_timer.elapsed() > MAX_RECV_WAIT)
      return NIAGARA_TIMEOUT;

    //Receive raw data from the radio module and process it at the lower layer
    status = Niagara::receive_raw(source, &control, &payload);

    //If the receive went into timeout state then try receiving the data again
    if(status == NIAGARA_TIMEOUT)
      continue;

    //If the receive returned an error then propagate it to the whole method
    if(status != NIAGARA_OK)
      return NIAGARA_RECEIVE_ERROR;

    /*
     * Check if the remote device has been set and it's the source of this message,
     * in which case, if the control message is a SYNCHRONIZE request, reset the handshake
     * status to restart. This situation might happen when the CRC32 check failed on the other
     * side and the other side sent the message again with the control message set as a SYN.
    */
    if(device == *source && control == SYN)
      state = RxState::WAIT_SYN;

    //Check based on the current handshake's state
    switch(state) {
      //If the handshake is waiting for a SYN message and it got received
      case RxState::WAIT_SYN:
        //If the maximum retransmission amount was reached on the remote side then close
        if(control == RETRANSMISSION_TIMEOUT)
          return NIAGARA_RETRANSMISSION_ERROR;
        //Check if the control message is SYN, otherwise ignore this message
        if(control != SYN)
          break;

        //The remote device we're communicating with is the source of this message
        device = *source;
        //Save the payload of the last message
        output_message = payload;

        //Reply with the acknowledgement 
        {
          //Compute the CRC to send back to the remote device
          str crc = crc32_to_str(crc32(payload));
          //Send the crc back as the payload of the acknowledgement
          if(Niagara::send_raw(device, ACK, crc) != NIAGARA_OK)
            return NIAGARA_SEND_ERROR; //In case of any send error then propagate it to the whole method
        }

        //Set the state to wait for the final acknowledgement
        state = RxState::WAIT_FINAL_ACK;
        //Reset the session timer
        session_timer.start();
        //Go back and wait for data to receive
        break;

      //If the handshake was pending for final acknowledgement
      case RxState::WAIT_FINAL_ACK:
        //Check if the source is the remote device we're communicating with
        if(*source != device)
          break;

        //If the control message matches with the requested acknowledgement
        if(control == ACK) {
          //Return the output and return the OK status
          *output = output_message;
          return NIAGARA_OK;
        }

        //If the maximum retransmission amount was reached on the remote side then close
        if(control == RETRANSMISSION_TIMEOUT)
          return NIAGARA_RETRANSMISSION_ERROR;

        break;
      }
  }
}

Niagara_Ret Niagara::send(str destination, str message) {
  //Check if the identifier hasn't yet been initialised
  if(identifier.length() == 0)
    return NIAGARA_NO_IDENTIFIER;

  //Saves the current send state of the handshake  
  enum class TxState {
    SEND_SYN,
    WAIT_ACK
  };

  //Variable to keep track of the current handshake's state
  TxState state = TxState::SEND_SYN;
  //Timer to check for timeouts
  Timer timer;
  //Counter for the amount of retransmissions
  int retransmissions = 0;

  //Remote device's callsign
  str remote;
  //Buffer for the received crc
  str received_crc;
  //Buffer for the control message received
  Niagara_Control control;
  //Buffer for the status of send and receive functions
  Niagara_Ret status;

  //Compute the crc of the message being sent so it can be checked
  str expected_crc = crc32_to_str(crc32(message));

  //Start the timer counting for retransmissions
  timer.start();
  //Loop until the maximum amount of retransmissions is reached
  while(retransmissions < NIAGARA_RETRANSMISSIONS) {
    switch(state) { //Check the current state of the handshake
      case TxState::SEND_SYN: //If the SYN request should be sent:
        //Send the SYN request with the message as payload
        if(Niagara::send_raw(destination, SYN, message) != NIAGARA_OK)
          return NIAGARA_SEND_ERROR; //In case of any error propagate it to the whole method

        //Start the timer counting for retransmission timeouts
        timer.start();
        //Wait for the acknowledgement
        state = TxState::WAIT_ACK;
        break;

      //If the ACK should be received
      case TxState::WAIT_ACK:
        //Wait for it to be actually received
        status = Niagara::receive_raw(&remote, &control, &received_crc);

        //In case the acknowledgement timed out then try to retransmit the message
        if(status == NIAGARA_TIMEOUT && timer.elapsed() > MAX_RECV_WAIT) {
          //Increment the retransmission counter
          retransmissions++;
          //Go back to the SYN
          state = TxState::SEND_SYN;
          //Exit from the switch condition
          break;
        }

        //If the receive method returned an error then propagate it to the whole method
        if(status != NIAGARA_OK)
          return NIAGARA_RECEIVE_ERROR;

        //If the remote device isn't correct or the control message isn't an acknowledgement then ignore this message
        if(remote != destination || control != ACK)
          break;

        //If the CRC check doesn't work correctly then go back to the SYN and retransmit
        if(received_crc != expected_crc) {
          //Increase the retransmission counter
          retransmissions++;
          //Go back to sending the SYN request
          state = TxState::SEND_SYN;
          //Exit from the switch condition
          break;
        }

        //Send the final acknowledgement with no payload and exit
        status = Niagara::send_raw(destination, ACK, "");
        //If there has been a problem sending the message then propagate that problem back to the whole method
        if(status != NIAGARA_OK)
          return NIAGARA_SEND_ERROR;
        //If the final acknowledgement had no internal error then the message got sent correctly
        return NIAGARA_OK;
    }
  }

  //In case of no retransmissions left then send a retransmission timeout message and exit
  Niagara::send_raw(destination, RETRANSMISSION_TIMEOUT, "");
  return NIAGARA_TIMEOUT;
}


// ---- Test main ----

int main() {
  Niagara::identifier = "NODE_A";

  std::cout << "--- TEST SEND ---" << std::endl;
  Niagara::send("NODE_B", "Hello Niagara");

  std::cout << "\n--- TEST RECEIVE ---" << std::endl;
  str out, src;
  Niagara::receive(&out, &src);

  std::cout << "\nFinal result:" << std::endl;
  std::cout << "source=" << src.c_str() << " message='" << out.c_str() << "'" << std::endl;
}
