#include <string>
#include "network/tcp_client.h"
#include "protocol/codes.h"
#include "protocol/framing.h"

class Client {
    public: 
        Client(const std::string& host, uint16_t port);

        void register_user(const std::string& username, const std::vector<uint8_t>& pubkey);
        void list_clients();
        void get_pubkey(const std::array<uint8_t, 16>& client_id);
        void send_message(const std::array<uint8_t, 16>& to_id, MessageType type, const std::vector<uint8_t>& content);
        void pull_messages();

    private: 
        TCPClient tcp_; 
        std::array<uint8_t, 16> id_;
        uint8_t version_ = 2;
};