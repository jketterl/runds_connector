#include "ammos_parser.hpp"

#include <iostream>
#include <arpa/inet.h>

using namespace RundS;

AmmosParser::AmmosParser(DataMode my_data_mode) {
    data_mode = my_data_mode;
    switch (data_mode) {
        case DataMode::SHORT:
            sample_size = sizeof(int16_t);
            break;
        case DataMode::LONG:
            sample_size = sizeof(int32_t);
            break;
    }
}

char* AmmosParser::parse(char* raw, int len, uint32_t* parsed_len, bool* swap) {
    char* read_pointer = raw;
    if (len < sizeof(ammos_frame_header_t)) {
        std::cerr << "WARNING: incomplete data\n";
        return nullptr;
    }

    ammos_frame_header_t* ammos_frame_header = reinterpret_cast<ammos_frame_header_t*>(read_pointer);
    read_pointer += sizeof(ammos_frame_header_t);
    if (ntohl(ammos_frame_header->magic) != 0xfb746572) {
        std::cerr << "WARNING: invalid magic word: " << std::hex << ammos_frame_header->magic << "\n";
        return nullptr;
    }
    if (ntohl(ammos_frame_header->frame_length) * 4 != len) {
        std::cerr << "WARNING: data size mismatch\n";
        return nullptr;
    }

    uint32_t data_header_length = ntohl(ammos_frame_header->data_header_length) * sizeof(uint32_t);
    ammos_data_header_t* ammos_data_header = reinterpret_cast<ammos_data_header_t*>(read_pointer);
    if (ntohl(ammos_data_header->data_block_count) != 1) {
        std::cerr << "WARNING: unexpected number of ammos data blocks\n";
    }
    read_pointer += data_header_length;

    *parsed_len = ntohl(ammos_data_header->data_block_length) * sizeof(uint32_t) / sample_size;

    ammos_data_block_header_t* ammos_data_block_header = reinterpret_cast<ammos_data_block_header_t*>(read_pointer);
    read_pointer += sizeof(ammos_data_block_header_t);

    // does ammos support swap? there seems to be no flag in the documentation.
    *swap = false;
    return read_pointer;
}