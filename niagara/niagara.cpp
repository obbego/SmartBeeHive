#include "niagara.h"
#include "Timer.h"
#include "Hash.h"
#include "fragmenter.h"

// Initialization of the static variable referencing the object
// Practice done to avoid "undefined reference" errors in the linker
Niagara* Niagara::this_object = nullptr;
															
void Niagara::vlog_printf(const char* format, va_list args) {
	// Ignore logging if log handler isn't set
	if(Niagara::log_handler == nullptr || Niagara::log_level == NO_LOG) return;

	// Copy arguments because they're consumed by first vsnprintf call
	va_list args_copy;
	va_copy(args_copy, args);

	// Calculate the length needed, ignoring the terminator character
	int len = vsnprintf(nullptr, 0, format, args_copy);
	va_end(args_copy);

	// If the formatting is invalid, the return value is negative
	if (len < 0) return;

	// Allocate needed memory
	char* buffer = new char[len + 1];

	// Write the text in the newly allocated buffer
	vsnprintf(buffer, len + 1, format, args);

	// Send the retrieved text to the log handler
	Niagara::log_handler(buffer);

	// Free used memory
	delete[] buffer;
}

void Niagara::log_printf(const char* format, ...) {
	// Ignore logging if log handler isn't set
	if(Niagara::log_handler == nullptr || Niagara::log_level == NO_LOG) return;

	//Call the internal variadic printf method
	va_list args;
	va_start(args, format);
	vlog_printf(format, args);
	va_end(args);
}

void Niagara::log_printf(Niagara_LogLevel level, const char* format, ...) {
	// Ignore logging if log handler isn't set
	if(Niagara::log_handler == nullptr || Niagara::log_level == NO_LOG) return;
	//Check if the current log level is the same as the one specified
	if(Niagara::log_level != level) return;

	//Call the printf method
	va_list args;
	va_start(args, format);
	vlog_printf(format, args);
	va_end(args);
}

void Niagara::log_print(str text) {
	//Ignore logging if log handler isn't set
	if(Niagara::log_handler == nullptr || Niagara::log_level == NO_LOG) return;

	//Send the string to the handler
	Niagara::log_handler(text.c_str());
}

/*
 * This is a convenience method which bundles the check
 * for the log level, deciding whether to use the concise
 * or the extended string based on the set log level.
 * If the log level is set to NO_LOG then the log handler isn't called.
 */
void Niagara::log_print(str extended, str concise) {
	//Ignore logging if log handler isn't set
	if(Niagara::log_handler == nullptr) return;

	//Send the string to the handler based on the log level
	switch(log_level) {
		case LOG_TERMINAL:
			Niagara::log_handler(extended.c_str());
			break;
		case LOG_DISPLAY:
			Niagara::log_handler(concise.c_str());
			break;
	}
}

void Niagara::log_print(Niagara_LogLevel level, str text) {
	if(Niagara::log_level != level) return;
	log_print(text);
}

Niagara::Niagara(NiagaraLogHandler _log_handler, Niagara_LogLevel _log_level) {
	//Save the log handler callback pointer
	Niagara::log_handler = _log_handler;
	//Save the log level enum
	Niagara::log_level = _log_level;

	/*
	 * Save this object's pointer to the static class'
	 * pointer so ISR callbacks for the receive function
	 * will edit this object's flag containing whether data is received.
	 * This static parameter set is the reason why creating multiple istances
	 * of this class on the same code is not supported, any additional
	 * istance of this class will override this static object and
	 * make receiving data not work because the flag would never be set.
	 */
	this_object = this;
	// Initialise the radio device
	int error;
	Niagara::lora = new AsyncDevice(&error);
	if(error) {
		return;
	}
	Niagara::chip_mtu = lora->getMTU();

	//Initialise the identifier as empty
	Niagara::identifier = "";
}

/* If the boolean for the log variable isn't defined, then just don't define a log handler */
Niagara::Niagara() : Niagara(nullptr, NO_LOG) {}

Niagara::~Niagara() {
	if(Niagara::lora) {
		delete Niagara::lora;
		Niagara::lora = nullptr;
	}
	
	#ifndef ARDUINO
	if(Niagara::hal) {
		delete Niagara::hal;
		Niagara::hal = nullptr;
	}
	#endif
}

bool Niagara::set_identifier(str _identifier) {
	if(Niagara::lora == nullptr) return false;

	if(!check_identifier(_identifier))
		return false;

	Niagara::identifier = _identifier;
	return true;
}

Niagara_Ret Niagara::send(str destination, str message) {
	log_print("[SEND START]\n");

	//Istance a new fragmenter with the message passed
	FragmentConstructor fragmenter(message, chip_mtu - 50);
	if(log_level == LOG_TERMINAL) log_printf("Initialised fragmenter for send - MTU: %d\n", chip_mtu);

	//Keep sending data to the specified destination using the fragments created
	int more_fragments;
	int repeat_fragment = 0;
	// String space for fragment data
	str chunk;
	do {
		//Get the next fragment
		if(repeat_fragment > 0) more_fragments = fragmenter.next_fragment(&chunk);
		
		if(more_fragments >= 0) {
			if(log_level == LOG_TERMINAL) log_printf("Fragments left: %d\n", more_fragments);
			else log_printf("%d Frag left.\n", more_fragments);
			//Send this fragment
			Niagara_Ret status = Niagara::send_fragment(destination, chunk);
			if(status != NIAGARA_OK) return status; //In case the function produced an error then return it and stop

			// Stop until confirmation is received
			str output_payload, source;
			status = Niagara::receive_fragment(&output_payload, &source, destination);
			if(status != NIAGARA_OK) {
				if(log_level == LOG_TERMINAL) log_printf("Error while receiving fragment confirmation! [%d]\n", static_cast<int>(status));
				else log_printf("Frag Recv ERR! [%d]\n", static_cast<int>(status));
				repeat_fragment++;
				continue;
			}
			// Check if the payload of the received fragment is correct
			int err_msg;
			if(!fragmenter.check_confirmation(output_payload, &err_msg)) {
				if(log_level == LOG_TERMINAL) log_printf("Error while checking fragment confirmation! [%d]\n", err_msg);
				else log_printf("Frag Check ERR! [%d]\n", err_msg);
				repeat_fragment++;
				continue;
			}
		}

		repeat_fragment = 0;
	} while(more_fragments > 0); //Keep sending fragments while there are more available

	log_print("[SEND DONE]\n");

	//Return OK once done
	return NIAGARA_OK;
}

Niagara_Ret Niagara::receive(str* output, str* source) {
	log_print("[RECV START]\n");

	//Create the object to handle fragmented data
	FragmentDestructor defragmenter;

	//Buffers for the data received
	str current_payload;
	str message_source;

	log_print("Receiving first fragment...\n", "First fragment.\n");
	//Receive the first fragmented packet from whoever is available
	Niagara_Ret status;
	int frag_status;
	do {
		status = receive_fragment(&current_payload, &message_source); //Receive a fragment
		if(status != NIAGARA_OK) continue; //If an error occurred then retry

		//Add this first fragment to the fragmenter
		str confirmation_str;
		frag_status = defragmenter.add_fragment(current_payload, &confirmation_str);
		if(frag_status < 0) {
			log_printf(LOG_TERMINAL, "Error while defragmenting! [%d]\n", frag_status);
			log_printf(LOG_DISPLAY, "Frag Err [%d]\n", frag_status);
			continue;
		} else log_print(LOG_TERMINAL, "Succesful defragmentation.\n");

		// Send the confirmation message to the other end
		status = send_fragment(message_source, confirmation_str);
		if(status != NIAGARA_OK) {
			if(log_level == LOG_TERMINAL) log_printf("Error while sending fragment confirmation! [%d]\n", static_cast<int>(status));
			else log_printf("Frag Send ERR! [%d]\n", static_cast<int>(status));
		}
	} while(status != NIAGARA_OK || frag_status < 0); //Repeat in case the status of any function is negative

	// Save the source of this fragment to filter all newly received packets
	str remote_dev = message_source;

	log_printf(LOG_TERMINAL, "Established connection with remote device: '%s' - Looking for additional fragments..\n", remote_dev.c_str());
	log_printf(LOG_DISPLAY, "Remote: '%s'\n", remote_dev.c_str());

	while(frag_status > 0) { // Keep executing while there still are fragments left to read
		//Don't allow incoming packets from external sources other than the remote device we're communicating with
		status = receive_fragment(&current_payload, &message_source, remote_dev);
		if(status != NIAGARA_OK) return status; //If an error occurred then return that error and exit

		//Add this first fragment to the fragmenter
		str confirmation_str;
		frag_status = defragmenter.add_fragment(current_payload, &confirmation_str);
		if(frag_status < 0) {
			log_printf(LOG_TERMINAL, "Error while defragmenting! [%d]\n", frag_status);
			log_printf(LOG_DISPLAY, "Frag Err [%d]\n", frag_status);
			continue;
		} else log_print(LOG_TERMINAL, "Succesful defragmentation.\n");

		// Send the confirmation message to the other end
		status = send_fragment(message_source, confirmation_str);
		if(status != NIAGARA_OK) {
			if(log_level == LOG_TERMINAL) log_printf("Error while sending fragment confirmation! [%d]\n", static_cast<int>(status));
			else log_printf("Frag Send ERR! [%d]\n", static_cast<int>(status));
			return status;
		}
	} while(frag_status > 0); // Keep executing while there still are fragments left to read

	log_print(LOG_TERMINAL, "Retrieving final message...");

	//Once there are no more packets to receive, then save the result in the output
	if(source != nullptr) *source = remote_dev;
	*output = defragmenter.get_message();

	log_print("[RECV DONE]\n");

	//Now return status OK
	return NIAGARA_OK;
}

Niagara_Ret Niagara::receive_fragment(str* output, str* source, str filter) {
	//Check if the identifier hasn't yet been initialised
	if(identifier.length() == 0) {
		log_print("Cannot start receive - identifier not set yet.\n", "[RECV] No identifier!\n");
		return NIAGARA_NO_IDENTIFIER;
	}

	//Saves the current receive state of the handshake  
	enum class RxState {
		WAIT_SYN,
		WAIT_FINAL_ACK
	};

	//Variable which keeps track of the handshake's state
	RxState state = RxState::WAIT_SYN;
	//Timer used to keep track of timeouts
	Timer session_timer;
	//Timer used to receive data only at specified intervals
	Timer receive_timer;

	//The remote device's identifier to communicate with
	str device = filter;
	//Buffer for the payload
	str payload;
	// Buffer for the actual message output
	str message_output;
	//Buffer for the control message
	Niagara_Control control;
	//Return status of send and receive methods
	Niagara_Ret status;

	//Start the timer to count for timeouts
	session_timer.start();
	//Start the timer used for receive intervals
	receive_timer.start();

	log_print("Starting message receive..\n", "[RECV] Start.\n");

	//Keep reiterating until the amount of retransmissions reaches the maximum amount of retransmissions
	while(true) {
		// Initialise status as NIAGARA_TIMEOUT so the loop check will work
		status = NIAGARA_TIMEOUT;
		do {
			//Check if the session time has exceeded the timeout in which case stop the communication
			if(session_timer.elapsed() > MAX_RECV_WAIT){
				log_print("[RECV] Timeout!\n");
				return NIAGARA_TIMEOUT;
			}

			//Only check for available data each set amount of time
			if(receive_timer.elapsed() >= RECEIVE_CHECK_MS) {
				log_printf(LOG_TERMINAL, "[RECV] Session timer: %ldms\n", session_timer.elapsed());
				//Reset the timer
				receive_timer.start();
				//Receive raw data from the radio module and process it at the lower layer
				status = Niagara::receive_raw(source, &control, &payload);
			}
		} while(status == NIAGARA_TIMEOUT || status == NIAGARA_NOT_DESTINATION || status == NIAGARA_NOT_RECEIVING); //Keep running while no data is received until external timeout is reached

		// Return no data in case no data is available
		if(status == NIAGARA_NO_DATA) {
			log_print(LOG_TERMINAL, "No data available to read!");
			return NIAGARA_NO_DATA;
		}
		//If the receive returned an error then propagate it to the whole method
		if(status != NIAGARA_OK) {
			log_printf(LOG_TERMINAL, "[RECV] Error while receiving: %d!\n", (int)status);
			return NIAGARA_RECEIVE_ERROR;
		}

		log_printf(LOG_TERMINAL, "[RECV] Received Data - [Current Remote Device] : '%s' - [Receive source] : '%s' - [Handshake Status] : %d - [Control Message] : %d - [Payload] : '%s'\n", device.c_str(), source->c_str(), static_cast<int>(state), static_cast<int>(control), payload.c_str());

		/*
		 * Check if the remote device has been set and it's the source of this message,
		 * in which case, if the control message is a SYNCHRONIZE request, reset the handshake
		 * status to restart. This situation might happen when the CRC32 check failed on the other
		 * side and the other side sent the message again with the control message set as a SYN.
		*/
		if(device == *source && control == SYN) {
			log_print("[RECV] SYN Request received, restarting handshake.\n", "[RECV] reSYN.\n");
			state = RxState::WAIT_SYN;
		}

		//Check based on the current handshake's state
		switch(state) {
			//If the handshake is waiting for a SYN message and it got received
			case RxState::WAIT_SYN:
				log_print("==== [WAITING FOR MESSAGE SYN] ====\n", "[RECV] Waiting for SYN\n");
				//If the maximum retransmission amount was reached on the remote side then close
				if(control == RETRANSMISSION_TIMEOUT) {
					log_print("[RECV] Max retransmission amount reached!\n", "MAX RETRANSMISSIONS!\n");
					return NIAGARA_RETRANSMISSION_ERROR;
				}
				//Check if the control message is SYN, otherwise ignore this message
				if(control != SYN) {
					if(log_level == LOG_TERMINAL) log_print("[RECV] Ignoring non-SYN request in WAIT_SYN state.\n");
					break;
				}

				//Check if a remote device's filter is specified
				if(device == "") 
					//The remote device we're communicating with is the source of this message
					device = *source;
				else if(device != *source) //If a remote device has been specified already and it's different from this packet's source, then ignore this packet
					break;

				//Reply with the acknowledgement 
				{
					//Compute the CRC to send back to the remote device
					str crc = crc32_to_str(crc32(payload));
					log_printf(LOG_TERMINAL, "[RECV] Computed payload CRC [%s]\n", crc.c_str());
					//Send the crc back as the payload of the acknowledgement
					log_print("==== [SENDING CRC ACKNOWLEDGEMENT] ====\n", "[RECV] Sending ACK\n");
					if(Niagara::send_raw(device, ACK, crc) != NIAGARA_OK)
						return NIAGARA_SEND_ERROR; //In case of any send error then propagate it to the whole method
				}

				//Set the state to wait for the final acknowledgement
				state = RxState::WAIT_FINAL_ACK;
				// Save current payload buffer
				message_output = payload;
				//Reset the session timer
				session_timer.start();
				if(log_level == LOG_TERMINAL) log_print("[RECV] State set to WAIT_FINAL_ACK. Restarted session timer.\n");
				//Go back and wait for data to receive
				break;

			//If the handshake was pending for final acknowledgement
			case RxState::WAIT_FINAL_ACK:
				log_print("==== [WAITING FOR FINAL ACKNOWLEDGEMENT] ====\n", "[RECV] Waiting for ACK\n");

				//Check if the source is the remote device we're communicating with
				if(*source != device) {
					log_printf(LOG_TERMINAL, "[RECV] Message source [%s] isn't the handshake device [%s]!\n", source->c_str(), device.c_str());
					break;
				}

				//If the control message matches with the requested acknowledgement
				if(control == ACK) {
					log_print("[RECV] Done receive.\n");
					//Return the output and return the OK status
					*output = message_output;
					return NIAGARA_OK;
				}

				//If the maximum retransmission amount was reached on the remote side then close
				if(control == RETRANSMISSION_TIMEOUT) {
					log_print("[RECV] Max retransmission amount reached!\n", "MAX RETRANSMISSIONS!\n");
					return NIAGARA_RETRANSMISSION_ERROR;
				}

				break;
			}
	}
}

bool Niagara::check_crc(str received_crc, str expected_crc) {
	//Clean up the received crc
	str received_crc_clean = "";

	//Iterate through the crc's characters
	for(int i = 0; i < received_crc.length(); i++) {
		//Check if the current character's a digit
		if(received_crc[i] >= '0' && received_crc[i] <= '9')
			//If so, then append it to the output string
			received_crc_clean += (char)received_crc[i];
	}

	//If the crc changed then print the log of it
	if(received_crc != received_crc_clean)
		log_printf(LOG_TERMINAL, "(Cleaned received crc before checking: [%s] -> [%s])", received_crc.c_str(), received_crc_clean.c_str());
	

	//Now that the received crc is clean, it can be checked with the expected one
	return received_crc_clean == expected_crc;
}

Niagara_Ret Niagara::send_fragment(str destination, str message) {
	//Check if the identifier hasn't yet been initialised
	if(identifier.length() == 0) {
		log_print("Cannot start receive - identifier not set yet.\n", "[SEND] No identifier!\n");
		return NIAGARA_NO_IDENTIFIER;
	}

	//Saves the current send state of the handshake  
	enum class TxState {
		SEND_SYN,
		WAIT_ACK
	};

	//Variable to keep track of the current handshake's state
	TxState state = TxState::SEND_SYN;
	//Timer to check for timeouts
	Timer timer;
	//Timer to check for received data
	Timer receive_timer;
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
	log_printf(LOG_TERMINAL, "[SEND] Computed input CRC [%s].\n", expected_crc.c_str());

	//Start the timer counting for retransmissions
	timer.start();
	log_print("Starting send...\n", "[SEND] Start.");
	//Loop until the maximum amount of retransmissions is reached
	while(retransmissions < NIAGARA_RETRANSMISSIONS) {
		log_printf(LOG_TERMINAL, "[SEND] Current State - [Current Remote Device] : '%s' - [Handshake Status] : %d\n", remote.c_str(), static_cast<int>(control));
		switch(state) { //Check the current state of the handshake
			case TxState::SEND_SYN: { //If the SYN request should be sent:
				log_print("==== [SENDING MESSAGE SYN] ====\n", "[SEND] Sending SYN.\n");
				//Send the SYN request with the message as payload
				if(Niagara::send_raw(destination, SYN, message) != NIAGARA_OK)
					return NIAGARA_SEND_ERROR; //In case of any error propagate it to the whole method

				//Start the timer counting for retransmission timeouts
				timer.start();
				//Wait for the acknowledgement
				state = TxState::WAIT_ACK;
				//Start the reception timer
				receive_timer.start();
				break;
			}	

			//If the ACK should be received
			case TxState::WAIT_ACK:
				log_print("==== [WAITING FOR CRC ACKNOWLEDGEMENT] ====\n", "[SEND] Waiting ACK.\n");

				// Before the loop, initialise status as NIAGARA_TIMEOUT so the while loop won't exit immeditaely
				status = NIAGARA_TIMEOUT;
				do {
					//In case the acknowledgement timed out then try to retransmit the message
					if(timer.elapsed() > MAX_RECV_WAIT) {
						//Increment the retransmission counter
						retransmissions++;
						if(log_level == LOG_TERMINAL) log_printf("Timeout! %d retransmissions, retrying.\n", retransmissions);
						else log_printf("[SEND] Timeout [%d]\n", retransmissions);
						//Go back to the SYN
						state = TxState::SEND_SYN;
						//Exit from the switch condition
						break;
					}

					//Only check for received data each set amount of time
					if(receive_timer.elapsed() >= RECEIVE_CHECK_MS && received_data) {
						//Reset the timer for receive
						receive_timer.start();
						//Wait for it to be actually received
						status = Niagara::receive_raw(&remote, &control, &received_crc);
					}
				} while(status == NIAGARA_TIMEOUT || status == NIAGARA_NOT_DESTINATION); // Keep receiving until valid data is received or external timeout is reached
				//If the state has changed, exit from the switch's condition and restart
				if(state == TxState::SEND_SYN) break;
				
				if(status == NIAGARA_NO_DATA) {
					log_printf(LOG_TERMINAL, " ['%s']", receive_output);
					return NIAGARA_NO_DATA;
				}
				//If the receive method returned an error then propagate it to the whole method
				if(status != NIAGARA_OK){
					log_printf("[SEND] Closing send with status code [%d]\n", static_cast<int>(status));
					return NIAGARA_RECEIVE_ERROR;
				}

				//If the remote device isn't correct or the control message isn't an acknowledgement then ignore this message
				if(remote != destination || control != ACK) {
					if(log_level == LOG_TERMINAL) log_print("Message received is irrelevant to the handshake.\n");
					break;
				}

				log_print("Checking CRC... ", "[SEND] Error check\n");
				//If the CRC check doesn't work correctly then go back to the SYN and retransmit
				if(!check_crc(received_crc, expected_crc)) {
					//Increase the retransmission counter
					retransmissions++;
					if(log_level == LOG_TERMINAL) log_printf(" Invalid! Received [%s], Expected [%s].\n%d retransmissions. Retrying.\n", received_crc.c_str(), expected_crc.c_str(), retransmissions);
					else log_print("[SEND] Check invalid\n");
					//Go back to sending the SYN request
					state = TxState::SEND_SYN;
					//Exit from the switch condition
					break;
				}

				log_print("==== [SENDING FINAL ACKNOWLEDGEMENT] ====\n", "[SEND] Check OK.\n");
				//Send the final acknowledgement with no payload and exit
				status = Niagara::send_raw(destination, ACK, "");
				//If there has been a problem sending the message then propagate that problem back to the whole method
				if(status != NIAGARA_OK)
					return NIAGARA_SEND_ERROR;
				//If the final acknowledgement had no internal error then the message got sent correctly
				log_print("Message sent.\n", "[SEND] Done.\n");
				return NIAGARA_OK;
		}
	}

	//In case of no retransmissions left then send a retransmission timeout message and exit
	if(log_level == LOG_TERMINAL) log_printf("Maximum retransmissions reached [%d]! - Sending retransmission timeout packet!\n", NIAGARA_RETRANSMISSIONS);
	else log_print("MAX RETRANSMISSIONS!\n");
	Niagara::send_raw(destination, RETRANSMISSION_TIMEOUT, "");
	return NIAGARA_TIMEOUT;
}

Niagara_Ret Niagara::receive_raw(str* source, Niagara_Control* control_output, str* message_output) {
	//If the identifier is empty and not yet initialized then return an error
	if(Niagara::identifier.length() == 0)
		return NIAGARA_NO_IDENTIFIER;

	log_print(LOG_TERMINAL, "\t[RECV_RAW] Reading packet ");ƒ
	// Create the string object to fit the data into
	str receive_output;
	//Read the data into the buffer
	bool state = lora->recv(&receive_output);
	// IF no data is available to receive
	if(!state) {
		log_print("No Data!\n");
		return NIAGARA_NO_DATA;
	}
	log_printf(LOG_TERMINAL, " ['%s']", receive_output);
	str processed_output[3];
	log_print(LOG_TERMINAL, "\t [RECV_RAW] Processing packet... ");
	//Check for errors on the process message method
	state = Niagara::process_message(processed_output, receive_output);
	if(state == 5) {
		log_print(LOG_TERMINAL, "This is not the destination!\n");
		return NIAGARA_NOT_DESTINATION;
	}
	else if(state != 0) {
		log_print(LOG_TERMINAL, "\n");
		return NIAGARA_INVALID_DATA;
	}

	int control_value = processed_output[1].toInt();

	log_printf(LOG_TERMINAL, "Done! -> src[%s], ctrl[%d], msg[%s]\n", processed_output[0].c_str(), control_value, processed_output[2].c_str());

	*source = processed_output[0];
	*control_output = static_cast<Niagara_Control>(control_value);
	*message_output = processed_output[2];

	return NIAGARA_OK;
}

int Niagara::process_message(str* output, str message) {
	//If the identifier is empty and not yet initialized then return an error
	if(identifier.length() == 0) {
		log_print("INVALID process_message CALL! - identifier is not set!", "[RX_PROC] Error 1\n");
		return 1;
	}
	if(message.length() == 0) {
		log_print("INVALID process_message CALL! - message is empty!", "[RX_PROC] Error 2\n");
		return 2;
	}

	//In case the message destination isn't this board then exit
	int separatorIndex = message.indexOf('|');
	if(separatorIndex <= 0) {
		log_print("INVALID process_message CALL! - missing '|' separator!", "[RX_PROC] Error 3\n");
		return 3;
	}
	str callsign = message.substring(0, separatorIndex);
	//Separate the destination id in source and destination id
	str sourceID, destinationID;
	int identifierSeparator = callsign.indexOf('.');
	if(identifierSeparator <= 0 || identifierSeparator >= callsign.length() - 1 || callsign.length() <= identifierSeparator) {
		log_print("INVALID process_message CALL! - missing callsign separator!", "[RX_PROC] Error 4\n");
		return 4;
	}
	sourceID = callsign.substring(0, identifierSeparator);
	destinationID = callsign.substring(identifierSeparator + 1);
	if(!valid_destination(destinationID)) {
		if(log_level == LOG_TERMINAL) log_printf("INVALID process_message CALL! - invalid destination ['%s'] !", destinationID.c_str());
		else log_print("[RX_PROC] Error 5\n");
		return 5;
	}
	if(message.length() <= separatorIndex + 1) {
		log_print("INVALID process_message CALL! - missing message chunk!", "[RX_PROC] Error 6\n");
		return 6;
	}
	int secondSeparatorIndex = message.substring(separatorIndex + 1).indexOf('|') + separatorIndex + 1;
	if(secondSeparatorIndex <= 0) {
		log_print("INVALID process_message CALL! - missing second separator!", "[RX_PROC] Error 7\n");
		return 7;
	}
	
	//Separate the str into the control sequence and the message itself
	output[0] = sourceID;
	output[1] = message.substring(separatorIndex + 1, secondSeparatorIndex);
	if(message.length() > secondSeparatorIndex + 1)
		output[2] = message.substring(secondSeparatorIndex + 1);
	else output[2] = "";

	return 0;
}

Niagara_Ret Niagara::send_raw(str destination, Niagara_Control control, str message) {
	//If the identifier is empty and not yet initialized then return an error
	if(Niagara::identifier.length() == 0)
		return NIAGARA_NO_IDENTIFIER;

	log_print(LOG_TERMINAL, "\t [SEND_RAW] Sending packet [");
	str formattedMessage = Niagara::format_message(destination, control, message);
	if(formattedMessage.length() == 0) {
		log_print(LOG_TERMINAL, "] Error!\n");
		return NIAGARA_INVALID_DATA;
	}
	log_printf(LOG_TERMINAL, "%s]... ", formattedMessage.c_str());
	//Check if the formatted message to send is bigger than the maximum transmission unit to send over the radio
	if(formattedMessage.length() >= Niagara::chip_mtu) return NIAGARA_TOO_LARGE;
	int status = Niagara::lora->send(formattedMessage);
	// After a transmit, make sure to reset the received_data handler, because it has now been set to true due to the ISR, but no data has actually been received
	Niagara::this_object->received_data = false;
	rxActive = false;
	if(status == RADIOLIB_ERR_TX_TIMEOUT) {
		log_print(LOG_TERMINAL, "Timeout!\n");
		return NIAGARA_TIMEOUT;
	}
	if(status != RADIOLIB_ERR_NONE) {
		if(log_level == LOG_TERMINAL) log_printf("Error [%d]!\n", status);
		else log_printf("[RAW_TX] Error %d\n", status);
		return RADIOLIB_ERROR;
	}
	log_print(LOG_TERMINAL, " OK.\n");
	return NIAGARA_OK;
}

str Niagara::format_message(str destination, Niagara_Control control, str message) {
		if(identifier.length() == 0) {
			log_print("INVALID format_message CALL! - identifier is not set!", "[TX_PROC] Error 1\n");
			return "";
		}

		if(destination.length() == 0) {
			log_print("INVALID format_message CALL! - destination is empty!", "[TX_PROC] Error 2\n");
			return "";
		}

		if(destination.indexOf('|') >= 0 || destination.indexOf('.') >= 0) {
			if(log_level == LOG_TERMINAL) log_printf("INVALID format_message CALL! - destination ['%s'] contains illegal characters.", destination.c_str());
			else log_print("[TX_PROC] Error 3\n");
			return "";
		}

		if(message.indexOf('|') >= 0) {
			if(log_level == LOG_TERMINAL) log_printf("INVALID format_message CALL! - message ['%s'] contains illegal characters.", message.c_str());
			else log_print("[TX_PROC] Error 4\n");
			return "";
		}

		if(control < 0 || control >= END) {
			if(log_level == LOG_TERMINAL) log_printf("INVALID format_message CALL! - control [%d] is invalid.", static_cast<int>(control));
			else log_print("[TX_PROC] Error 5\n");
			return "";
		}

		return identifier + "." + destination + "|" +
					 str(static_cast<int>(control)) + "|" +
					 message;
}

bool Niagara::valid_destination(str destination) {
	//Check for broadcast destination
	if(destination == BROADCAST)
		return true;

	//Check if the two strings match in size, if they don't then skip
	if(Niagara::identifier.length() != destination.length()) return false;

	#if defined(ARDUINO)
	for (size_t i = 0; i < Niagara::identifier.length(); ++i) {
			if (tolower(Niagara::identifier[i]) != tolower(destination[i])) return false;
	}
	#else
	for (size_t i = 0; i < Niagara::identifier.length(); ++i) {
			if (std::tolower(static_cast<unsigned char>(Niagara::identifier[i])) !=
					std::tolower(static_cast<unsigned char>(destination[i]))) {
					return false;
			}
	}
	#endif

	return true;
}

bool Niagara::check_identifier(str identifier) {
	//Print to the log the check for the identifier
	if(log_level == LOG_TERMINAL) log_printf("Checking identifier: '%s'.. ", identifier.c_str());
	else log_print("Checking callsign.\n");
	//The identifier should be between 4 and 12 characters long
	size_t len = identifier.length();
	if (len < 4 || len > 12) {
		log_print("Invalid size (must be between 4 and 12 characters).\n", "Invalid size.\n");
		return false;
	}
	if(identifier == BROADCAST) {
		log_print("Invalid, it's broadcast.\n", "Can't be broadcast.\n");
		return false;
	}

	//Check if the identifier contains only alphanumeric values
	#if defined(ARDUINO)
	for (int i = 0; i < len; ++i) {
			char c = identifier[i];
			if (!isAlphaNumeric(c)) {
				if(log_level == LOG_TERMINAL) log_printf("Invalid, character %d isn't alphanumeric (%c)", i, c);
				else log_print("Invalid characters.\n");
				return false;
			}
	}
	#else
	const char* identifier_array = identifier.c_str();
	for (int i = 0; i < identifier.length(); i++) {
			if (!std::isalnum(static_cast<unsigned char>(identifier_array[i]))) {
				if(log_level == LOG_TERMINAL) log_printf("Invalid, character %d isn't alphanumeric (%c)", i, identifier_array[i]);
				else log_print("Invalid characters.\n");
				return false;
			}
	}
	#endif
	log_print("Valid.\n");
	return true;
}
