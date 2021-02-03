#pragma once

#include "parser.hpp"

namespace RundS {

    class Eb200Parser: public Parser {
        public:
            virtual char* parse(char* raw, int len, uint32_t* parsed_len, bool* swap);
    };

    typedef struct eb200_header_s {
        uint32_t magic;
        uint16_t version_minor;
        uint16_t version_major;
        uint16_t sequence;
        uint16_t reserved;
        uint32_t data_size;
    } eb200_header_t;

    typedef struct eb200_generic_attribute_s {
        uint16_t tag;
        uint16_t length;
    } eb200_generic_attribute_t;

    typedef struct eb200_if_attribute_s {
        int16_t number_of_trace_values;
        int8_t reserved;
        uint8_t optional_header_length;
        uint32_t selector_flags;
    } eb200_if_attribute_t;

    typedef struct eb200_if_trace_header_s {
        int16_t if_mode;
        int16_t frame_length;
        uint32_t sample_rate;
        uint32_t freq_low;
        uint32_t bandwidth;
        uint16_t demodulation;
        int16_t rx_attenuation;
        int16_t flags;
        int16_t reserved;
        char demodulation_string[8];
        uint64_t sample_count;
        uint32_t freq_high;
        uint8_t reserved_2[4];
        uint64_t start_timestamp;
    } eb200_if_trace_header_t;

}