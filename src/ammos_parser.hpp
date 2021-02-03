#pragma once

#include "parser.hpp"

namespace RundS {

    template <typename T>
    class AmmosParser: public Parser<T> {
        public:
            AmmosParser();
            ~AmmosParser();
            virtual T* parse(char* raw, int len, uint32_t* parsed_len);
        private:
            int32_t* conversion_buffer;
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