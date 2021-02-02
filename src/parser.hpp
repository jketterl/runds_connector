#pragma once

#include <stdint.h>

namespace RundS {

    class Parser {
        public:
            virtual char* parse(char* raw, int len, uint32_t* parsed_len, bool* swap) = 0;
    };

}