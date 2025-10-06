#ifndef CLIENT_H
#define CLIENT_H

#include <string>
#include <iostream>
#include <algorithm>
#include <iomanip>
#include <vector>
#include <unordered_map>

#include "network/tcp_client.h"
#include "crypto/Base64Wrapper.h"
#include "crypto/RSAWrapper.h"
#include "crypto/AESWrapper.h"

#include "protocol/codes.h"
#include "protocol/framing.h"
#include "protocol/constants.h"

class Client {
    public: 
        Client();

        void register_user();
        void list_clients();
        void get_pubkey();
        void pull_messages();
        void send_mesage_to_client();
        void request_sym_key();
        void send_sym_key();

    private: 
        TCPClient tcp_; 
        
        std::array<uint8_t, 16> id_;
        std::string username_;
        std::string pubkey_;
        std::string privkey_;
        uint8_t version_;

        struct OtherClient {
            std::string username;
            std::string pubkey;
            std::vector<uint8_t> symkey;
        };

        std::unordered_map<std::string, OtherClient> other_clients_;

        bool get_dest_user(std::array<uint8_t, Protocol::client_id_len>& id);
        bool find_user_id(const std::string& username, std::array<uint8_t, Protocol::client_id_len>& id); 
        bool send_message(const std::array<uint8_t, Protocol::client_id_len>& to_id, MessageType type, const std::vector<uint8_t>& content);
};

#endif