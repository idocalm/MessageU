#include "tcp_client.h"

using boost::asio::ip::tcp;

TCPClient::TCPClient() : socket_(io_context_) {
    std::ifstream f(CLIENT_CONFIG);
    if (!f.is_open()) {
        throw std::runtime_error("Could not open " CLIENT_CONFIG);
    }

    std::string line;
    std::getline(f, line);

    auto pos = line.find(':');
    if (pos == std::string::npos)
        throw std::runtime_error("Invalid format in server.info (expected HOST:PORT)");

    host_ = line.substr(0, pos);
    port_ = static_cast<uint16_t>(std::stoi(line.substr(pos + 1)));
}

void TCPClient::connect() {
    tcp::resolver resolver(io_context_);
    auto endpoints = resolver.resolve(host_, std::to_string(port_));
    boost::asio::connect(socket_, endpoints);
}

void TCPClient::send(const std::vector<uint8_t>& data) {
    boost::asio::write(socket_, boost::asio::buffer(data));
}

std::vector<uint8_t> TCPClient::receive_n(size_t n) {
    std::vector<uint8_t> buf(n);
    boost::asio::read(socket_, boost::asio::buffer(buf, n));
    return buf;
}

std::vector<uint8_t> TCPClient::receive() {
    // header
    std::vector<uint8_t> header = receive_n(Protocol::header_len_resp);

    uint32_t payload_len = read_le32(&header[Protocol::version_len + Protocol::code_len]);

    std::vector<uint8_t> payload = receive_n(payload_len);
    
    header.insert(header.end(), payload.begin(), payload.end());
    return header;
}

