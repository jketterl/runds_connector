#include "protocol.hpp"

#include <arpa/inet.h>

using namespace RundS;

template <typename T>
void Protocol<T>::convertFromNetwork(int16_t* input, int16_t* output, uint32_t length) {
    for (int i = 0; i < length; i++) {
        output[i] = ntohs(input[i]);
    }
}

template <typename T>
void Protocol<T>::convertFromNetwork(int32_t* input, int32_t* output, uint32_t length) {
    for (int i = 0; i < length; i++) {
        output[i] = ntohl(input[i]);
    }
}

template class Protocol<int32_t>;
template class Protocol<int16_t>;