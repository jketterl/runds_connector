#pragma once

#include "parser.hpp"

namespace RundS {

    class AmmosParser: public Parser {
        public:
            virtual char* parse(char* raw, int len, uint32_t* parsed_len, bool* swap);
    };

}