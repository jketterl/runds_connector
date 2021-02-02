#pragma once

namespace RundS {

    class Parser {
        public:
            virtual void parse(char* raw) = 0;
    };

}