#include "eb200_connector.hpp"

int main (int argc, char** argv) {
    Eb200Connector* connector = new Eb200Connector();
    return connector->main(argc, argv);
}

int Eb200Connector::parse_arguments(int argc, char** argv) {
    int r = Connector::parse_arguments(argc, argv);
    if (r != 0) return r;

    if (argc - optind >= 2) {
        host = std::string(argv[optind]);
        port = (uint16_t) strtoul(argv[optind + 1], NULL, 10);
    } else if (optind < argc) {
        std::string argument = std::string(argv[optind]);
        size_t colon_pos = argument.find(':');
        if (colon_pos == std::string::npos) {
            host = argument;
        } else {
            host = argument.substr(0, colon_pos);
            port = stoi(argument.substr(colon_pos + 1));
        }
    }

    return 0;
}

uint32_t Eb200Connector::get_buffer_size() {
    // initial guess
    return 16 * 32 * 512
};

int Eb200Connector::open() {
    return 0;
};

int Eb200Connector::read() {
    return 0;
};

int Eb200Connector::close() {
    return 0;
};

int Eb200Connector::set_center_frequency(double frequency) {
    return 0;
};

int Eb200Connector::set_sample_rate(double sample_rate) {
    return 0;
};

int Eb200Connector::set_gain(Owrx::GainSpec* gain) {
    return 0;
};

int Eb200Connector::set_ppm(int ppm) {
    return 0;
};
