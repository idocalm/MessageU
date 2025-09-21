#include "tcp_client.h"
#include <iostream>

using boost::asio::ip::tcp;

TCPClient::TCPClient(const std::string& host, uint16_t port)
    : host_(host), port_(port), socket_(io_context_) {}

void TCPClient::connect() {
    tcp::resolver resolver(io_context_);
    auto endpoints = resolver.resolve(host_, std::to_string(port_));
    boost::asio::connect(socket_, endpoints);
}

void TCPClient::send(const std::vector<uint8_t>& data) {
    boost::asio::write(socket_, boost::asio::buffer(data));
}

std::vector<uint8_t> TCPClient::receive() {
    std::vector<uint8_t> buf(4096);
    size_t len = socket_.read_some(boost::asio::buffer(buf));
    buf.resize(len);

    return buf;
}

