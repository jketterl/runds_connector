#pragma once

#include <owrx/connector.hpp>
#include <owrx/gainspec.hpp>
#include <netinet/in.h>
#include "runds_connector.hpp"

namespace RundS {

    enum DataMode { SHORT, LONG };
    enum ProtocolType { EB200, AMMOS };

    class RundSConnector: public Owrx::Connector {
        public:
            RundSConnector(): Owrx::Connector::Connector() {};
        protected:
            virtual uint32_t get_buffer_size() override;
            std::stringstream get_usage_string() override;
            std::vector<struct option> getopt_long_options() override;
            int receive_option(int c, char* optarg) override;
            virtual int parse_arguments(int argc, char** argv) override;
            virtual void print_version() override;
            virtual int open() override;
            virtual int read() override;
            virtual int close() override;
            virtual int set_center_frequency(double frequency) override;
            virtual int set_sample_rate(double sample_rate) override;
            virtual int set_gain(Owrx::GainSpec* gain) override;
            virtual int set_ppm(double ppm) override;
        private:
            std::string host = "127.0.0.1";
            struct in_addr host_addr;
            uint16_t port = 5555;
            std::string local_data_ip;
            uint16_t data_port;
            DataMode data_mode = DataMode::SHORT;
            ProtocolType protocol_type = ProtocolType::EB200;
            int control_sock;
            int data_sock;

            int send_command(std::string cmd);
            template <typename T>
            int read();
    };

}