#include "tcp_client.h"
using boost::asio::ip::tcp;

/**
 * @brief Construct a new TCPClient::TCPClient object
 * Loads server details from CLIENT_CONFIG
 * @throws std::runtime_error if CLIENT_CONFIG is missing or has an invalid format
 */
TCPClient::TCPClient() : socket_(io_context_) {
    std::ifstream f(CLIENT_CONFIG);
    if (!f.is_open()) {
        throw std::runtime_error("Could not open " CLIENT_CONFIG);
    }

    std::string line;
    std::getline(f, line);

    // We expect a IP:PORT syntax!
    auto pos = line.find(':');
    if (pos == std::string::npos)
        throw std::runtime_error("Invalid format in " CLIENT_CONFIG " (expected IP:PORT)");

    // validate port is in range
    int port = std::stoi(line.substr(pos + 1));
    if (port < MIN_PORT || port > MAX_PORT)
        throw std::runtime_error("Invalid port in " CLIENT_CONFIG);

    host_ = line.substr(0, pos);
    port_ = static_cast<uint16_t>(port);
}

/**
 * @brief Creates the TCP connection for the saved host_, port_
 */
void TCPClient::connect() {
    tcp::resolver resolver(io_context_);
    auto endpoints = resolver.resolve(host_, std::to_string(port_));
    boost::asio::connect(socket_, endpoints);
}

/**
 * @brief Sends raw bytes over the connection
 * 
 * @param data - byte vecto r to send 
 */
void TCPClient::send(const std::vector<uint8_t>& data) {
    boost::asio::write(socket_, boost::asio::buffer(data));
}

/**
 * @brief Reads exactly n bytes from the socket
 * 
 * @param n - the number of bytes to read
 * @return std::vector<uint8_t> - the data
 */
std::vector<uint8_t> TCPClient::receive_n(size_t n) {
    std::vector<uint8_t> buf(n);
    boost::asio::read(socket_, boost::asio::buffer(buf, n));
    return buf;
}

/**
 * @brief Reads the whole header + payload from the server socket
 * 
 * @return std::vector<uint8_t> - header + payload buffer
 */
std::vector<uint8_t> TCPClient::receive() {
    // get all the header bytes (header len is const)
    std::vector<uint8_t> header = receive_n(Protocol::header_len_resp);

    // get the payload length from the header
    uint32_t payload_len = read_le32(&header[Protocol::version_len + Protocol::code_len]);

    // get the payload itself
    std::vector<uint8_t> payload = receive_n(payload_len);
    
    header.insert(header.end(), payload.begin(), payload.end());
    return header;
}

