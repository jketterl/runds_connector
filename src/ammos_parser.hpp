#pragma once

#include "parser.hpp"

namespace RundS {

    class AmmosParser: public Parser {
        public:
            virtual void parse(char* raw);
    };

}