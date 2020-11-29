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
};