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

// Logging macros 
#define OK(msg)  std::cout << Color::GREEN  << "[+] " << msg << Color::RESET << std::endl
#define ERR(msg) std::cerr << Color::RED    << "[-] " << msg << Color::RESET << std::endl
#define INFO(msg) std::cout << Color::YELLOW << "[*] " << msg << Color::RESET << std::endl

// This lets us print to console in different colors
namespace Color {
    inline const std::string RESET   = "\033[0m";
    inline const std::string RED     = "\033[31m";
    inline const std::string GREEN   = "\033[32m";
    inline const std::string YELLOW  = "\033[33m";
    inline const std::string BLUE    = "\033[34m";
    inline const std::string CYAN    = "\033[36m";
}

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
        std::string pubkey_;
        std::string privkey_;
        uint8_t version_;

        /*
            To save information about other clients (names and symmetric keys), I use this mini client struct
            We then have a table with other clients that gets filled when the user activates LIST_CLIENTS
        */
        
        struct OtherClient {
            std::string username;
            std::string pubkey;
            std::vector<uint8_t> symkey;
        };
        std::unordered_map<std::string, OtherClient> other_clients_;


        std::vector<uint8_t> build_register_payload(const std::string& username, const std::string& pubkey);
        bool send_message(const std::array<uint8_t, Protocol::client_id_len>& to_id, MessageType type, const std::vector<uint8_t>& content);
        bool get_dest_user(std::array<uint8_t, Protocol::client_id_len>& id);
        std::string decrypt_message(const std::string& content, const std::vector<uint8_t>& symkey);
        void handle_incoming_sym_key(const std::array<uint8_t, Protocol::client_id_len>& from_id, const std::vector<uint8_t>& encrypted);
        void save_client_file();
    };

#endif