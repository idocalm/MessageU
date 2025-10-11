#ifndef TCP_CLIENT_H
#define TCP_CLIENT_H

#include <boost/asio.hpp>
#include <protocol/constants.h>
#include <vector>
#include <string>
#include <fstream>

#define MIN_PORT 1
#define MAX_PORT 65535

/**
 * @brief A tcp client using boost to send and receive data from the server, used by the Client
 */
class TCPClient {
    public: 
        TCPClient();

        void connect();
        void send(const std::vector<uint8_t>& data);

        std::vector<uint8_t> receive_n(size_t n);
        std::vector<uint8_t> receive(); 

    private:
        std::string host_; // Server ip
        uint16_t port_; // Server port 
        boost::asio::io_context io_context_;
        boost::asio::ip::tcp::socket socket_;
};

#endif