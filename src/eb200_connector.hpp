#pragma once

#include <owrx/connector.hpp>
#include <owrx/gainspec.hpp>

class Eb200Connector: public Owrx::Connector {
    public:
        Eb200Connector(): Owrx::Connector::Connector() {};
    protected:
        virtual uint32_t get_buffer_size() override;
        virtual int parse_arguments(int argc, char** argv) override;
        virtual int open() override;
        virtual int read() override;
        virtual int close() override;
        virtual int set_center_frequency(double frequency) override;
        virtual int set_sample_rate(double sample_rate) override;
        virtual int set_gain(Owrx::GainSpec* gain) override;
        virtual int set_ppm(int ppm) override;
    private:
        std::string host = "127.0.0.1";
        uint16_t port = 5555;
        int control_sock;
        int data_sock;

        int send_command(std::string cmd);
};

struct eb200_header_t {
    uint32_t magic;
    uint16_t version_major;
    uint16_t version_minor;
    uint16_t sequence;
    uint16_t reserved;
    uint32_t data_size;
};

struct eb200_generic_attribute_t {
    uint16_t tag;
    uint16_t length;
};

struct eb200_if_trace_header_t {
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
};

struct eb200_if_attribute_t {
    int16_t number_of_trace_values;
    int8_t reserved;
    uint8_t optional_header_length;
    uint32_t selector_flags;
    struct eb200_if_trace_header_t if_trace_header;
};