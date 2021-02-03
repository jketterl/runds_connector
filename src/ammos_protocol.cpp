#include "ammos_protocol.hpp"

#include <iostream>
#include <arpa/inet.h>

using namespace RundS;

template <typename T>
AmmosProtocol<T>::AmmosProtocol() {
    conversion_buffer = (int32_t*) malloc(sizeof(int32_t) * 65536);
}

template <typename T>
AmmosProtocol<T>::~AmmosProtocol() {
    free(conversion_buffer);
}

template <typename T>
std::string AmmosProtocol<T>::getTrace() {
    return "AIF";
}

template <>
std::string AmmosProtocol<int16_t>::getModeString() {
    return "ASHORT";
}

template<>
std::string AmmosProtocol<int32_t>::getModeString() {
    return "ALONG";
}

template <typename T>
T* AmmosProtocol<T>::parse(char* raw, int len, uint32_t* parsed_len) {
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

    uint32_t data_block_length = ntohl(ammos_data_header->data_block_length);
    *parsed_len = data_block_length * sizeof(uint32_t) / sizeof(T);

    // unused
    // ammos_data_block_header_t* ammos_data_block_header = reinterpret_cast<ammos_data_block_header_t*>(read_pointer);
    read_pointer += sizeof(ammos_data_block_header_t);

    this->convertFromNetwork((int32_t*) read_pointer, conversion_buffer, data_block_length);

    return (T*) conversion_buffer;
}

template class AmmosProtocol<int16_t>;
template class AmmosProtocol<int32_t>;