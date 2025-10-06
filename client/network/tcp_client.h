#ifndef TCP_CLIENT_H
#define TCP_CLIENT_H

#include <boost/asio.hpp>
#include <protocol/constants.h>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>

class TCPClient {
    public: 
        TCPClient();

        void connect();
        void send(const std::vector<uint8_t>& data);

        // get exactly n bytes
        std::vector<uint8_t> receive_n(size_t n);
        std::vector<uint8_t> receive(); 

    private:
        std::string host_;
        uint16_t port_;
        boost::asio::io_context io_context_;
        boost::asio::ip::tcp::socket socket_;
};

#endif