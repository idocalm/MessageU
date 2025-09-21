#include <boost/asio.hpp>
#include <vector>
#include <string>

class TCPClient {
    public: 
        TCPClient(const std::string& host, uint16_t port);

        void connect();
        void send(const std::vector<uint8_t>& data);
        std::vector<uint8_t> receive(); 

    private:
        std::string host_;
        uint16_t port_;
        boost::asio::io_context io_context_;
        boost::asio::ip::tcp::socket socket_;

};