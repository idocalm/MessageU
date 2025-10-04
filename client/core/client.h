#ifndef CLIENT_H
#define CLIENT_H

#include <string>
#include <iostream>
#include <algorithm>
#include <iomanip>

#include "network/tcp_client.h"
#include "crypto/Base64Wrapper.h"
#include "protocol/codes.h"
#include "protocol/framing.h"
#include "protocol/constants.h"

class Client {
    public: 
        Client();

        void register_user(const std::string& username, const std::string& pubkey, const std::string& privkey);
        void list_clients();
        void get_pubkey(const std::array<uint8_t, 16>& client_id);
        void send_message(const std::array<uint8_t, 16>& to_id, MessageType type, const std::vector<uint8_t>& content);
        void pull_messages();

    private: 
        TCPClient tcp_; 
        
        std::array<uint8_t, 16> id_;
        std::string username_;
        std::string pubkey_;
        std::string privkey_;
        uint8_t version_;
};

#endif