#include "fragmenter.h"

int FragmentConstructor::count_digits(unsigned int input) {
    if (input == 0) return 1;
    int count = 0;

    while (input > 0) {
        input /= 10;
        count++;
    }
    return count;
}

// === FragmentConstructor ===
FragmentConstructor::FragmentConstructor(str data, const int _max_size) {
    // Initialise values
    _data = data;
    _cursor = 0;
    _current_index = 0;
    
    int len = _data.length();
    
    if (len == 0) {
        _total_fragments = 0;
        FRAG_SIZE = _max_size; 
        return;
    }

    // Iterative loop to compute the MTU based on header overhead
    // We start by "guessing" we only need 1 fragment.
    unsigned int n_fragments = 1;
    int payload_size = 0;
    while (true) {
        // 1. Calculate the digit length of our current fragment guess 'n_fragments'
		int max_digits = count_digits(n_fragments);
        
        // 2. Compute worst-case header size: "index#total#" 
        // Worst-case is when index has the same amount of digits as total fragments.
        // Example: "12#12#" -> 2 digits + 2 digits + 2 separators = 6 bytes
        int max_header_len = max_digits + max_digits + 2; 
        
        // 3. Deduct header from the max size to get our actual Payload MTU
        payload_size = _max_size - max_header_len;
        
        // Failsafe: if the max size is too small to even fit the header
        if (payload_size <= 0) {
            _total_fragments = 0;
            FRAG_SIZE = 0;
            return;
        }
        
        // 4. Test our guess: How many fragments do we ACTUALLY need with this payload size?
        int required_fragments = (len + payload_size - 1) / payload_size;
        
        // 5. Did our guess provide enough capacity?
        if (required_fragments <= n_fragments) {
            // Yes! We found the exact amount of fragments needed.
            _total_fragments = required_fragments; 
            FRAG_SIZE = payload_size;
            break;
        }
        
        // No, our guess was too low (the header ate up too much space). 
        // We jump our guess to the new required amount and try again.
        n_fragments = required_fragments; 
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

	// Construct the confirmation message to send to the sending end
	*confirmation_msg = str(index) + "#" + str(total) + "#";

	// Check if the fragments have been completed
	if (_next_expected_index == _expected_total) {
		return 0; // Message ready
	}

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
