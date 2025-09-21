#include "client.h"
#include <iostream>

Client::Client(const std::string& host, uint16_t port)
    : tcp_(host, port) {
    tcp_.connect();
}

void Client::register_user(const std::string& username, const std::vector<uint8_t>& pubkey) {
    std::vector<uint8_t> payload(255 + 160, 0);
    memcpy(payload.data(), username.c_str(), std::min<size_t>(username.size(), 254));
    memcpy(payload.data() + 255, pubkey.data(), 160);

    RequestFrame req;
    // TODO: What's the client ID for registration?
    req.client_id.fill(0);
    req.version = version_;
    req.code = (uint16_t) RequestCode::REGISTER;
    req.payload = payload;  

    tcp_.send(req.to_bytes());
    auto raw = tcp_.receive();
    auto resp = ResponseFrame::from_bytes(raw);

    if (resp.code == (uint16_t) ResponseCode::REGISTER_OK) {
        memcpy(id_.data(), resp.payload.data(), 16);
        std::cout << "[+] Registered with ID: ";
        for (auto b : id_) {
            printf("%02x", b);
        }

        std::cout << std::endl;
    } else {
        std::string msg(resp.payload.begin(), resp.payload.end());
        std::cerr << "[-] Registration failed with code: " << resp.code << "; server message=" << msg << std::endl;    
    }


}

void Client::list_clients() {
    RequestFrame req;
    req.client_id = id_;
    req.version = version_;
    req.code = (uint16_t) RequestCode::LIST_CLIENTS;
    req.payload.clear();

    tcp_.send(req.to_bytes());
    auto raw = tcp_.receive();
    auto resp = ResponseFrame::from_bytes(raw);

    if (resp.code != (uint16_t) ResponseCode::LIST_CLIENTS) {
        std::cerr << "[-] LIST_CLIENTS failed with code: " << resp.code << std::endl;
        return;
    }

    size_t rec_size = 16 + 255; // 16 bytes ID + 255 bytes username
    if (resp.payload.size() % rec_size != 0) {
        std::cerr << "[-] LIST_CLIENTS payload size invalid" << std::endl;
        return;
    }

    size_t num_clients = resp.payload.size() / rec_size;
    std::cout << "[*] " << num_clients << " clients registered:" << std::endl;
    for (size_t i = 0; i < num_clients; i++) {
        auto offset = i * rec_size;
        std::array<uint8_t, 16> client_id;
        memcpy(client_id.data(), resp.payload.data() + offset, 16);
        offset += 16;

        char username[256] = {0};
        memcpy(username, resp.payload.data() + offset, 255);

        printf("    ID: ");
        for (auto b : client_id) {
            printf("%02x", b);
        }
        printf(" | Username: %s\n", username);
    }
}
