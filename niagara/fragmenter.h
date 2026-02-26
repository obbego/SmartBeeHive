#ifndef FRAGMENTER_H
#define FRAGMENTER_H

#include "str.h"

class FragmentConstructor {
    public:
        /*
         * Istances a new fragment constructor with the 
         * data passed to this constructor.
        */
        FragmentConstructor(str data);

        /*
         * Constructs the next fragment to send, returns a negative value
         * in case no more fragments are there to read.
         */
        int next_fragment(str* output);

};

class FragmentDestructor {
    public:
        /*
         * Istances a new fragment destructor.
        */
        FragmentDestructor();

        /*
         * Adds the new fragment read to the message
        */
        int add_fragment(str data);

        /*
         * Gets the whole message and clears all buffers.
        */
        str get_message();
};

#endif