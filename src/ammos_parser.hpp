#pragma once

#include "parser.hpp"
#include "runds_connector.hpp"

namespace RundS {

    class AmmosParser: public Parser {
        public:
            AmmosParser(DataMode data_mode);
            virtual char* parse(char* raw, int len, uint32_t* parsed_len, bool* swap);
        private:
            DataMode data_mode;
            uint8_t sample_size;
    };

    typedef struct ammos_frame_header_s {
        uint32_t magic;
        uint32_t frame_length;
        uint32_t frame_count;
        uint32_t frame_type;
        uint32_t data_header_length;
        uint32_t reserved;
    } ammos_frame_header_t;

    typedef struct ammos_data_header_s {
        uint32_t data_block_count;
        uint32_t data_block_length;
        // there is typically more fields, depending on the device, but the above ones are all we need
    } ammos_data_header_t;

    typedef struct ammos_data_block_header_s {
        uint32_t status_word;
    } ammos_data_block_header_t;

}