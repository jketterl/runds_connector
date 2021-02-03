#include "eb200_protocol.hpp"

#include <arpa/inet.h>
#include <iostream>
#include <cstring>

using namespace RundS;

template <typename T>
Eb200Protocol<T>::Eb200Protocol() {
    conversion_buffer = (T*) malloc(65536 * sizeof(T));
}

template <typename T>
Eb200Protocol<T>::~Eb200Protocol() {
    free(conversion_buffer);
}

template <typename T>
std::string Eb200Protocol<T>::getTrace() {
    return "IF";
}

template<>
std::string Eb200Protocol<int16_t>::getModeString() {
    return "SHORT";
}

template<>
std::string Eb200Protocol<int32_t>::getModeString() {
    return "LONG";
}

template <typename T>
T* Eb200Protocol<T>::parse(char* raw, int len, uint32_t* parsed_len) {
    char* read_pointer = raw;
    if (len < sizeof(eb200_header_t)) {
        std::cerr << "WARNING: incomplete data\n";
        return nullptr;
    }

    eb200_header_t* eb200_header = reinterpret_cast<eb200_header_t*>(read_pointer);
    read_pointer += sizeof(eb200_header_t);
    if (ntohl(eb200_header->magic) != 0x000eb200) {
        std::cerr << "WARNING: invalid magic word: " << std::hex << eb200_header->magic << "\n";
        return nullptr;
    }
    if (ntohl(eb200_header->data_size) != len) {
        std::cerr << "WARNING: data size mismatch\n";
        return nullptr;
    }

    eb200_generic_attribute_t* eb200_generic_attribute = reinterpret_cast<eb200_generic_attribute_t*>(read_pointer);
    read_pointer += sizeof(eb200_generic_attribute_t);
    if (ntohs(eb200_generic_attribute->tag) != 901) {
        std::cerr << "WARNING: unexpected tag type\n";
        return nullptr;
    }

    eb200_if_attribute_t* eb200_if_attribute = reinterpret_cast<eb200_if_attribute_t*>(read_pointer);
    read_pointer += sizeof(eb200_if_attribute_t);

    *parsed_len = ntohs(eb200_if_attribute->number_of_trace_values) * 2;

    uint32_t flags = ntohl(eb200_if_attribute->selector_flags);

    if ((flags & ~0xA0000000) != 0) {
        std::cerr << "WARNING: unexpected selector flags: " << std::hex << flags << "\n";
    }

    if (!(flags & 0x80000000) && eb200_if_attribute->optional_header_length > 0) {
        std::cerr << "WARNING: unexpected optional header\n";
    }

    // we don't really need anything from the optional header.
    // since it's optional, we cannot rely on it anyway...
    read_pointer += eb200_if_attribute->optional_header_length;

    if (flags & 0x20000000) {
        return (T*) read_pointer;
    } else {
        this->convertFromNetwork((T*) read_pointer, conversion_buffer, *parsed_len);
        return conversion_buffer;
    }
}

template class Eb200Protocol<int16_t>;
template class Eb200Protocol<int32_t>;