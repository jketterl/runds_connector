#pragma once

#include "parser.hpp"

namespace RundS {

    class Eb200Parser: public Parser {
        public:
            virtual void parse(char* raw);
    };

}