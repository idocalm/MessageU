#ifndef CLIENT_H
#define CLIENT_H

#include <string>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <array>

#include "tcp_client.h"
#include "Base64Wrapper.h"
#include "RSAWrapper.h"
#include "AESWrapper.h"

#include "codes.h"
#include "framing.h"
#include "constants.h"

/**
 * @brief Main object that handles registeration, messages or other operations
 */
class Client {
    public: 
        Client();

        void register_user();
        void list_clients();
        void get_pubkey();
        void pull_messages();
        void send_message_to_client();
        void request_sym_key();
        void send_sym_key();

    private: 
        TCPClient tcp_; 
        
        std::array<uint8_t, Protocol::client_id_len> id_{}; // The clients ID
        std::string username_;
        std::array<uint8_t, Protocol::max_pubkey_len> pubkey_{};
        std::vector<uint8_t> privkey_;
        uint8_t version_;

        /*
            To save information about other clients (names and symmetric keys), I use this mini client struct
            We then have a table with other clients that gets filled when the user activates LIST_CLIENTS
        */
        
        struct OtherClient {
            std::string username;
            std::array<uint8_t, Protocol::max_pubkey_len> pubkey{};
            std::array<uint8_t, Protocol::symkey_length> symkey{};
        };
        std::unordered_map<std::string, OtherClient> other_clients_;


        std::vector<uint8_t> build_register_payload(const std::string& username, const std::array<uint8_t, Protocol::max_pubkey_len>& pubkey);
        bool send_message(const std::array<uint8_t, Protocol::client_id_len>& to_id, MessageType type, const std::vector<uint8_t>& content);
        bool get_dest_user(std::array<uint8_t, Protocol::client_id_len>& id);
        std::string decrypt_message(const std::string& content, const std::array<uint8_t, Protocol::symkey_length>& symkey);
        void handle_incoming_sym_key(const std::array<uint8_t, Protocol::client_id_len>& from_id, const std::vector<uint8_t>& encrypted);
        void save_client_file();
    };

#endif