/**
 * @file fragmenter.h
 * @brief Handlers for packet fragmentation and defragmentation in the Niagara communication protocol.
 */

#ifndef FRAGMENTER_H
#define FRAGMENTER_H

#include "str.h"

/**
 * @author Zanotti Enrico
 * @class FragmentConstructor
 * @brief Manager for fragmentation on the sending end.
 * 
 * This class, given a big payload of data which might exceed the chip's MTU, manages
 * its fragmentation into packets which fit the latter.
 * 
 * The class separates an initial passed string containing the message, into 
 * multiple packets' payloads which are sequentially returned from calls to the `next_fragment(str*)` method,
 * alongside a header which identifies each fragment, allowing the receiving end to reconstruct the message
 * accordingly.
 * 
 * During fragment communication, the receiving end constructs a message containing either the header of the last
 * fragment received or an error code. This message is then sent back and should be passed to the `check_confirmation(str, int*)`
 * method so the devices make sure the fragment communication is sequentially correct. In case some fragments are lost or
 * there is an error code during packet defragmentation, a retransmission of the entire message must be performed.
 * 
 * The choice of not incorporating single fragment retransmission by keeping track of sequential identificators in the
 * fragments has been made for two main reasons:
 * 
 * - The added complexity to the protocol, resulting also in more needed computing power which isn't ideal
 *   for this kind of IoT application.
 * 
 * - The simplicity and low duty-cycle of the packets which will be sent through this protocol make it so
 *   these complications aren't necessarily required and a simple full packet retransmission is enough for
 *   the sake of this project.
 */
class FragmentConstructor {
	public:
		/**
		 * @brief Instances a new fragment constructor with the data passed.
		 * @param data The raw data to be fragmented
		 * @param max_size Maximum size for each fragment, which should be set to the device's MTU.
		 *                 This parameter is optional and thus has a default value of `256`.
		 * 
		 * The `max_size` parameter does keep track of the size of the overhead caused by
		 * the niagara protocol, other than the fragmentation header too.
		 * Any fragmented section of data should thus be considered safe to send through niagara's
		 * lower layers without worrying about header sizes.
		 */
		FragmentConstructor(str data, const int max_size = 256);

		/**
		 * @brief Constructs the next fragment to send.
		 * @param output Packet containing the next fragment to send
		 * @returns the amount of fragments left to read.
		 * 
		 * This method sequentially returns each fragmented packet which must be sent
		 * to the destination.
		 */
		int next_fragment(str* output);

		/**
		 * @brief Checks the confirmation message received on the sending end.
		 * @param confirmation_received The string containing the confirmation received from the remote device
		 * @param err_msg The error code set in case the confirmation is incorrect.
		 * @returns `true` if the remote response is valid, `false` otherwise.
		 * 
		 * This method should be called after each `next_fragment(int*)` call with the 
		 * received response on the receiving end. It checks whether the remote device is
		 * aligned with the fragment receiving and if it has lost any packets or
		 * encountered any error while defragmenting.
		 * 
		 * In case this method returns a non valid confirmation check, the message should
		 * be retransmitted entirely.
		*/
		bool check_confirmation(str confirmation_received, int* err_msg);
	
	private:
		int FRAG_SIZE;

		str _data;
		int _cursor;
		int _total_fragments;
		int _current_index;

		int count_digits(unsigned int input);
};

/**
 * @author Zanotti Enrico
 * @class FragmentDestructor
 * @brief Defragmentation manager for the receiving end
 * 
 * This class is meant to construct back the original message being provided all packets' of data
 * constituting its fragments.
 * 
 * During fragmented message communication, the sending end communicates each fragment with
 * a header identifying it, this class uses said header to construct the final message and
 * check if the packets are all being sent in the correct sequential order.
 * 
 * When each fragment is received, a confirmation is built which contains either the header
 * of the last received fragment or an error code. This confirmation message must be sent back 
 * to the sending end. In case there was an error in the defragmentation or if a fragment was
 * lost, the sending end must retransmit the entire message from the beginning.
 */
class FragmentDestructor {
	public:
		/**
		 * @brief Istances a new fragment destructor.
		*/
		FragmentDestructor();

		/**
		 * @brief Adds the new fragment read to the message.
		 * @param data The fragment packet
		 * @param confirmation_msg Pointer to a `str` object which is filled with the confirmation
		 *                         message to send back to the remote device.
		 * @returns The amount of fragments still left to retrieve.
		 *          Otherwise negative value in case the fragment data passed is invalid,
		 *          or `0` if no more fragments are there to compute in the message,
		*/
		int add_fragment(str data, str* confirmation_msg);

		/**
		 * @brief This method collects the defragmented final message once all fragments are provided
		 * @returns The raw whole defragmented message
		 * 
		 * After this method's call, all buffers are then cleared and all memory is free'd.
		*/
		str get_message();

	private:
		str _buffer;
		int _expected_fragments;
		int _received_count;
		int _expected_total;
		int _next_expected_index;
};

#endif