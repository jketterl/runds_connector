#pragma once

#include <stdint.h>
#include <string>

namespace RundS {

    template <typename T>
    class Protocol {
        public:
            virtual T* parse(char* raw, int len, uint32_t* parsed_len) = 0;
            virtual std::string getTrace() = 0;
        protected:
            void convertFromNetwork(int16_t* input, int16_t* output, uint32_t length);
            void convertFromNetwork(int32_t* input, int32_t* output, uint32_t length);
    };

}