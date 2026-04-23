#include "fragmenter.h"

// === FragmentConstructor ===

FragmentConstructor::FragmentConstructor(str data, const int _max_size) {
	// Initialise values
	_data = data;
	_cursor = 0;
	_current_index = 0;
	FRAG_SIZE = _max_size; // Defines the maximum size of a single packet.
	
	// Retrieve the fragment amount
	int len = _data.length();
	if (len == 0) {
		_total_fragments = 0;
	} else {
		// Computed necessary fragments
		_total_fragments = (len + FRAG_SIZE - 1) / FRAG_SIZE;
	}
}

int FragmentConstructor::next_fragment(str* output) {
	// If all fragments have been received then ignore call
	if (_current_index >= _total_fragments || _total_fragments == 0)
		return -1; //No more fragments

	// Define the index of this fragment's end character
	int end = _cursor + FRAG_SIZE;
	if (end > _data.length()) end = _data.length();
	//Retrieve the fragment's substring of the whole data
	str payload = _data.substring(_cursor, end);

	// Packet's construction: "indice#totale#payload"
	*output = str(_current_index) + "#" + str(_total_fragments) + "#" + payload;

	//Set the start of the new fragment to the end of this one
	_cursor = end;
	//Increment the fragment index
	_current_index++;

	// Return the amount of fragments left
	return _total_fragments - _current_index;
}

bool FragmentConstructor::check_confirmation(str confirmation_received, int* err_msg = nullptr) {
	// Check for problems during defragmentation, which require fragment retransmission
	if(confirmation_received.c_str() == "!!!!") {
		if(err_msg != nullptr) *err_msg = 1;
		return false;
	}

	//Retrieve the separator indexes
	int firstSep = confirmation_received.indexOf('#');
	int secondSepRelative = confirmation_received.substring(firstSep + 1).indexOf('#');
	int secondSep = firstSep + 1 + secondSepRelative;

	// Check separator presence
	if (firstSep <= 0 || secondSepRelative < 0 || secondSep <= firstSep + 1) {
		if(err_msg != nullptr) *err_msg = 2;
		return false; // Malformed header
	}

	//Retrieve the three parameters contained in the header
	int index = confirmation_received.substring(0, firstSep).toInt();
	int total = confirmation_received.substring(firstSep + 1, secondSep).toInt();

	// Check fragment indexes
	if(total != _total_fragments) {
		if(err_msg != nullptr) *err_msg = 3;
		return false;
	}
	if(index != _current_index - 1) {
		if(err_msg != nullptr) *err_msg = 4;
		return false;
	}

	if(err_msg != nullptr) *err_msg = 0;
	// IF no problem is occurred, then return true
	return true;
}

// === FragmentDestructor ===

FragmentDestructor::FragmentDestructor() {
	// Initialise values
	_expected_total = -1;
	_next_expected_index = 0;
	_buffer = "";
}

int FragmentDestructor::add_fragment(str data, str* confirmation_msg) {
	//Retrieve the separator indexes
	int firstSep = data.indexOf('#');
	int secondSepRelative = data.substring(firstSep + 1).indexOf('#');
	int secondSep = firstSep + 1 + secondSepRelative;

	// Fill the confirmation string in case of error
	*confirmation_msg = "!!!!";

	// Check separator presence
	if (firstSep <= 0 || secondSepRelative < 0 || secondSep <= firstSep + 1) {
		return -2; // Malformed header
	}

	//Retrieve the three parameters contained in the header
	int index = data.substring(0, firstSep).toInt();
	int total = data.substring(firstSep + 1, secondSep).toInt();
	str payload = data.substring(secondSep + 1);

	// Check for errors in toInt() conversion
	if (index == -1 || total == -1) {
		return -3; // Non-numeric indexes
	}

	// Handling new messages or fragment reset
	// If a fragment with index 0 is received, always reset the buffer and restart
	if (index == 0) {
		_buffer = "";
		_next_expected_index = 0;
		_expected_total = total;
	} 

	// Synchronization check
	// If a valid total hasn't been specified or the index value isn't expected
	if (_expected_total == -1 || index != _next_expected_index) {
		return -4; // Error: out-of-sequence or orphan fragment
	}

	// Total consistency
	// Check if fragments start modifying their total amount
	if (total != _expected_total) {
		_expected_total = -1; // Invalid state
		return -5; 
	}

	// Now accept the payload
	_buffer += payload;
	_next_expected_index++;

	// Check if the fragments have been completed
	if (_next_expected_index == _expected_total) {
		return 0; // Message ready
	}

	// Construct the confirmation message to send to the sending end
	*confirmation_msg = str(index) + "#" + str(total) + "#";

	// Return amount of fragments left
	return _expected_total - _next_expected_index;
}

str FragmentDestructor::get_message() {
	// Move the buffer to a string containing the result
	str result = _buffer;
	
	// Reset the internal state
	_buffer = "";
	_expected_total = -1;
	_next_expected_index = 0;
	
	//Return the result
	return result;
}