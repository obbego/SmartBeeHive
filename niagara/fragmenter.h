#ifndef FRAGMENTER_H
#define FRAGMENTER_H

#include "str.h"

class FragmentConstructor {
    public:
        /*
         * Istances a new fragment constructor with the 
         * data passed to this constructor.
         * An optional second parameter constitutes the maximum size
         * for each fragment, which should be set to the device's MTU.
        */
        FragmentConstructor(str data, const int max_size);

        /*
         * Constructs the next fragment to send, returns the amount
         * of fragments left to read.
         */
        int next_fragment(str* output);
    
    private:
        int FRAG_SIZE;

        str _data;
        int _cursor;
        int _total_fragments;
        int _current_index;
};

class FragmentDestructor {
    public:
        /*
         * Istances a new fragment destructor.
        */
        FragmentDestructor();

        /*
         * Adds the new fragment read to the message.
         * Returns a negative value in case the fragment data
         * passed is invalid.
         * Returns 0 if no more fragments are there to compute in the message
         * Otherwise returns the amount of fragments still left to retrieve
        */
        int add_fragment(str data);

        /*
         * Gets the whole message and clears all buffers.
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