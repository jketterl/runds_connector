#include "eb200_connector.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <iostream>
#include <cstring>
#include <unistd.h>

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
    return 16 * 32 * 512;
};

int Eb200Connector::open() {
    struct hostent* hp = gethostbyname(host.c_str());
    if (hp == NULL) {
        std::cerr << "gethostbyname() failed\n";
        return 3;
    }

    if ((control_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "eb200 control socket creation error: " << control_sock << "\n";
        return 1;
    }

    struct sockaddr_in control_remote;

    std::memset(&control_remote, 0, sizeof(control_remote));
    control_remote.sin_family = AF_INET;
    control_remote.sin_port = htons(port);
    control_remote.sin_addr = *((struct in_addr *) hp->h_addr);

    if (connect(control_sock, (struct sockaddr *)&control_remote, sizeof(control_remote)) < 0) {
        std::cerr << "eb200 connection failed\n";
        return 2;
    }

    if (send_command("*RST\r\n") != 0) {
        std::cerr << "eb200 reset failed\n";
        return 2;
    }

    if ((data_sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        std::cerr << "eb200 data socket creation error: " << data_sock << "\n";
        return 1;
    }

    struct sockaddr_in data_remote;

    std::memset(&data_remote, 0, sizeof(data_remote));
    data_remote.sin_family = AF_INET;
    // TODO automatically get port
    data_remote.sin_port = htons(6000);
    data_remote.sin_addr.s_addr = INADDR_ANY;

    if (bind(data_sock, (struct sockaddr*) &data_remote, sizeof(data_remote)) < 0) {
        std::cerr << "eb200 data socket bind error\n";
        return 1;
    }

    return 0;
};

int Eb200Connector::send_command(std::string cmd) {
    ssize_t len = cmd.size();
    ssize_t sent = send(control_sock, cmd.c_str(), len, 0);
    if (len != sent) return -1;

    char* buf = (char*) malloc(sizeof(char) * 255);
    int bytes_read = recv(control_sock, buf, 255, MSG_DONTWAIT);
    if (bytes_read > 0) {
        std::string response = std::string(buf, bytes_read);
        std::cerr << "command response: " << response << "\n";
    }
    free(buf);
    return 0;
}

int Eb200Connector::read() {
    char* buffer = (char*) malloc(sizeof(char) * get_buffer_size());
    char* read_pointer;
    int read;
    struct sockaddr_in cliaddr;
    memset(&cliaddr, 0, sizeof(cliaddr));
    socklen_t len = sizeof(cliaddr);
    struct eb200_header_t eb200_header;
    struct eb200_generic_attribute_t eb200_generic_attribute;
    struct eb200_if_attribute_t eb200_if_attribute;

    // TODO automatically fill in IP and port
    if (send_command("trace:udp:tag:on \"192.168.1.223\",6000,if\r\n") != 0) {
        std::cerr << "sending trace command failed\n";
        return 1;
    }

    if (send_command("SYST:IF:REM:MODE SHORT\r\n") != 0) {
        std::cerr << "sending mode command failed\n";
        return 1;
    }

    while (run) {
        read = recvfrom(data_sock, buffer, get_buffer_size(), 0, (struct sockaddr*) &cliaddr, &len);
        read_pointer = buffer;
        if (read < sizeof(struct eb200_header_t)) {
            std::cerr << "WARNING: incomplete data\n";
            continue;
        }
        std::memcpy(&eb200_header, read_pointer, sizeof(eb200_header));
        read_pointer += sizeof(eb200_header);
        if (ntohl(eb200_header.magic) != 0x000eb200) {
            std::cerr << "WARNING: invalid magic word: " << std::hex << eb200_header.magic << "\n";
            continue;
        }
        if (ntohl(eb200_header.data_size) != read) {
            std::cerr << "WARNING: data size mismatch\n";
            continue;
        }

        std::memcpy(&eb200_generic_attribute, read_pointer, sizeof(eb200_generic_attribute));
        read_pointer += sizeof(eb200_generic_attribute);
        if (ntohs(eb200_generic_attribute.tag) != 901) {
            std::cerr << "WARNING: unexpected tag type\n";
            continue;
        }

        std::memcpy(&eb200_if_attribute, read_pointer, sizeof(eb200_if_attribute));
        read_pointer += sizeof(eb200_if_attribute);

        processSamples((int16_t*) (read_pointer - 1), ntohs(eb200_if_attribute.number_of_trace_values) * 2);
    }

    free(buffer);
    return 0;
};

int Eb200Connector::close() {
    if (::close(data_sock) < 0) {
        std::cerr << "eb200 data socket close error\n";
        return 1;
    }

    if (::close(control_sock) < 0) {
        std::cerr << "eb200 control socket close error\n";
        return 1;
    }

    return 0;
};

int Eb200Connector::set_center_frequency(double frequency) {
    return send_command("FREQ " + std::to_string((long) frequency) + "\r\n");
};

int Eb200Connector::set_sample_rate(double sample_rate) {
    return send_command("BAND " + std::to_string((long) (sample_rate / 1.28)) + "\r\n");
};

int Eb200Connector::set_gain(Owrx::GainSpec* gain) {
    return 0;
};

int Eb200Connector::set_ppm(int ppm) {
    return 0;
};
