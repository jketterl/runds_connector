#include "eb200_parser.hpp"

#include <arpa/inet.h>
#include <iostream>
#include <cstring>

using namespace RundS;

char* Eb200Parser::parse(char* raw, int len, uint32_t* parsed_len, bool* swap) {
    char* read_pointer = raw;
    if (len < sizeof(struct eb200_header_t)) {
        std::cerr << "WARNING: incomplete data\n";
        return nullptr;
    }

    struct eb200_header_t* eb200_header = reinterpret_cast<struct eb200_header_t*>(read_pointer);
    read_pointer += sizeof(struct eb200_header_t);
    if (ntohl(eb200_header->magic) != 0x000eb200) {
        std::cerr << "WARNING: invalid magic word: " << std::hex << eb200_header->magic << "\n";
        return nullptr;
    }
    if (ntohl(eb200_header->data_size) != len) {
        std::cerr << "WARNING: data size mismatch\n";
        return nullptr;
    }

    struct eb200_generic_attribute_t* eb200_generic_attribute = reinterpret_cast<struct eb200_generic_attribute_t*>(read_pointer);
    read_pointer += sizeof(struct eb200_generic_attribute_t);
    if (ntohs(eb200_generic_attribute->tag) != 901) {
        std::cerr << "WARNING: unexpected tag type\n";
        return nullptr;
    }

    struct eb200_if_attribute_t* eb200_if_attribute = reinterpret_cast<struct eb200_if_attribute_t*>(read_pointer);
    read_pointer += sizeof(struct eb200_if_attribute_t);

    *parsed_len = ntohs(eb200_if_attribute->number_of_trace_values) * 2;

    uint32_t flags = ntohl(eb200_if_attribute->selector_flags);

    if ((flags & ~0xA0000000) != 0) {
        std::cerr << "WARNING: unexpected selector flags: " << std::hex << flags << "\n";
    }

    if (!(flags & 0x80000000) && eb200_if_attribute->optional_header_length > 0) {
        std::cerr << "WARNING: unexpected optional header\n";
    }
    *swap = (flags & 0x20000000) > 0;

    // we don't really need anything from the optional header.
    // since it's optional, we cannot rely on it anyway...
    read_pointer += eb200_if_attribute->optional_header_length;

    return read_pointer;
}