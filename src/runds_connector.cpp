#include "runds_connector.hpp"
#include "protocol.hpp"
#include "eb200_protocol.hpp"
#include "ammos_protocol.hpp"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <iostream>
#include <cstring>
#include <unistd.h>

using namespace RundS;

int main (int argc, char** argv) {
    RundSConnector* connector = new RundSConnector();
    return connector->main(argc, argv);
}

std::stringstream RundSConnector::get_usage_string() {
    std::stringstream s = Owrx::Connector::get_usage_string();
    s <<
        " -m, --protocol          select data protocol to be used (eb200, ammos)\n" <<
        " -l, --long              use long (32bit samples) sample size\n";
    return s;
}

std::vector<struct option> RundSConnector::getopt_long_options() {
    std::vector<struct option> long_options = Owrx::Connector::getopt_long_options();
    long_options.push_back({"long", no_argument, NULL, 'l'});
    long_options.push_back({"protocol", required_argument, NULL, 'm'});
    return long_options;
}

int RundSConnector::receive_option(int c, char* optarg) {
    switch (c) {
        case 'l':
            data_mode = DataMode::LONG;
            break;
        case 'm':
            if (strcmp(optarg, "eb200") == 0) {
                protocol_type = ProtocolType::EB200;
            } else if (strcmp(optarg, "ammos") == 0) {
                protocol_type = ProtocolType::AMMOS;
            } else {
                std::cerr << "Invalid protocol: " << optarg << "\n";
                return -1;
            }
            break;
        default:
            return Owrx::Connector::receive_option(c, optarg);
    }
    return 0;
}


int RundSConnector::parse_arguments(int argc, char** argv) {
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

void RundSConnector::print_version() {
    std::cout << "runds_connector version " << VERSION << "\n";
    Connector::print_version();
}

uint32_t RundSConnector::get_buffer_size() {
    // initial guess
    return 16 * 32 * 512;
};

int RundSConnector::open() {
    struct hostent* hp = gethostbyname(host.c_str());
    if (hp == NULL) {
        std::cerr << "gethostbyname() failed\n";
        return 3;
    }

    if ((control_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "eb200 control socket creation error: " << control_sock << "\n";
        return 1;
    }

    host_addr = *((struct in_addr*) hp->h_addr);

    struct sockaddr_in control_remote;

    std::memset(&control_remote, 0, sizeof(control_remote));
    control_remote.sin_family = AF_INET;
    control_remote.sin_port = htons(port);
    control_remote.sin_addr = host_addr;

    if (connect(control_sock, (struct sockaddr *)&control_remote, sizeof(control_remote)) < 0) {
        std::cerr << "eb200 connection failed\n";
        return 2;
    }

    if (send_command("*RST\r\n") != 0) {
        std::cerr << "eb200 reset failed\n";
        return 2;
    }

    socklen_t len = sizeof(control_remote);
    if (getsockname(control_sock, (struct sockaddr *) &control_remote, &len) < 0) {
        std::cerr << "getting local IP failed \n";
        return 3;
    }

    local_data_ip = std::string(inet_ntoa(control_remote.sin_addr));

    if ((data_sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        std::cerr << "eb200 data socket creation error: " << data_sock << "\n";
        return 1;
    }

    struct sockaddr_in data_remote;

    std::memset(&data_remote, 0, sizeof(data_remote));
    data_remote.sin_family = AF_INET;
    // automatically assign port
    data_remote.sin_port = 0;
    data_remote.sin_addr.s_addr = INADDR_ANY;

    if (bind(data_sock, (struct sockaddr*) &data_remote, sizeof(data_remote)) < 0) {
        std::cerr << "eb200 data socket bind error\n";
        return 1;
    }

    len = sizeof(data_remote);
    if (getsockname(data_sock, (struct sockaddr*) &data_remote, &len) < 0) {
        std::cerr << "eb200 data socket getsockname error\n";
        return 1;
    }

    data_port = ntohs(data_remote.sin_port);

    struct timeval tv;
    tv.tv_sec = 2;
    tv.tv_usec = 0;
    if (setsockopt(data_sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        std::cerr << "setting data socket timeout failed\n";
        return 1;
    }

    return 0;
};

int RundSConnector::send_command(std::string cmd) {
    ssize_t len = cmd.size();
    ssize_t sent = send(control_sock, cmd.c_str(), len, 0);
    if (len != sent) return -1;

    char* buf = (char*) malloc(sizeof(char) * 255);
    int bytes_read = recv(control_sock, buf, 255, MSG_DONTWAIT);
    if (bytes_read > 0) {
        std::string response = std::string(buf, bytes_read);
        std::cout << "command response: " << response << "\n";
    }
    free(buf);
    return 0;
}

int RundSConnector::read() {
    if (data_mode == DataMode::LONG) {
        return read<int32_t>();
    } else if (data_mode == DataMode::SHORT) {
        return read<int16_t>();
    }
    std::cerr << "unsupported data mode: " << data_mode << "\n";
    return 1;
}

template <typename T>
int RundSConnector::read() {
    char* buffer = (char*) malloc(sizeof(char) * get_buffer_size());
    T* conversion_buffer = (T*) malloc(sizeof(T) * get_buffer_size());
    int read;
    uint32_t parsed_len;
    struct sockaddr_in cliaddr;
    memset(&cliaddr, 0, sizeof(cliaddr));
    socklen_t len = sizeof(cliaddr);

    Protocol<T>* protocol;
    switch (protocol_type) {
        case ProtocolType::EB200:
            protocol = new Eb200Protocol<T>();
            break;
        case ProtocolType::AMMOS:
            protocol = new AmmosProtocol<T>();
            break;
    }

    if (send_command("trace:udp:tag:on \"" + local_data_ip + "\"," + std::to_string(data_port) + "," + protocol->getTrace() + "\r\n") != 0) {
        std::cerr << "registering trace failed\n";
        return 1;
    }

    // send "PRIVATE" flag as a separate command since it isn't supported by all receiver types
    if (send_command("trace:udp:flag:on \"" + local_data_ip + "\"," + std::to_string(data_port) + ", \"PRIVATE\"\r\n") != 0) {
        std::cerr << "registering trace flags failed\n";
        return 1;
    }

    if (send_command("SYST:IF:REM:MODE " + protocol->getModeString() + "\r\n") != 0) {
        std::cerr << "sending mode command failed\n";
        return 1;
    }

    while (run) {
        read = recvfrom(data_sock, buffer, get_buffer_size(), 0, (struct sockaddr*) &cliaddr, &len);
        if (read < 0) {
            std::cerr << "ERROR: data socket read error; shutting down\n";
            run = false;
            break;
        }

        if (host_addr.s_addr != cliaddr.sin_addr.s_addr) {
            std::cerr << "WARNING: discarding data coming from unexpected source (" << inet_ntoa(cliaddr.sin_addr) << ")\n";
            continue;
        }

        T* converted = protocol->parse(buffer, read, &parsed_len);
        if (converted == nullptr) {
            continue;
        }

        processSamples(converted, parsed_len);
    }

    if (send_command("trace:udp:tag:off \"" + local_data_ip + "\"," + std::to_string(data_port) + "," + protocol->getTrace() + "\r\n") != 0) {
        std::cerr << "deregistering trace failed\n";
        return 1;
    }

    if (send_command("trace:udp:delete \"" + local_data_ip + "\"," + std::to_string(data_port) + "\r\n") != 0) {
        std::cerr << "deregistering trace failed\n";
        return 1;
    }

    delete protocol;
    free(conversion_buffer);
    free(buffer);
    return 0;
};

int RundSConnector::close() {
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

int RundSConnector::set_center_frequency(double frequency) {
    return send_command("FREQ " + std::to_string((long) frequency) + "\r\n");
};

int RundSConnector::set_sample_rate(double sample_rate) {
    return send_command("BAND " + std::to_string((long) (sample_rate / 1.28)) + "\r\n");
};

int RundSConnector::set_gain(Owrx::GainSpec* gain) {
    if (dynamic_cast<Owrx::AutoGainSpec*>(gain) != nullptr) {
        return send_command("GCON:MODE AGC\r\n");
    }

    Owrx::SimpleGainSpec* simple_gain;
    if ((simple_gain = dynamic_cast<Owrx::SimpleGainSpec*>(gain)) != nullptr) {
        int r = send_command("GCON:MODE FIX\r\n");
        if (r != 0) {
            std::cerr << "setting gain mode failed\r\n";
            return 1;
        }

        r = send_command("GCON " + std::to_string((int) simple_gain->getValue()) + "\r\n");
        if (r != 0) {
            std::cerr << "setting gain failed\r\n";
            return 1;
        }

        return 0;
    }

    std::cerr << "unsupported gain settings\n";
    return 100;
};

int RundSConnector::set_ppm(double ppm) {
    return 0;
};
