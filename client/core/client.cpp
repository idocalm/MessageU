#include "client.h"
#include "menu.h"

Client::Client() : tcp_() {
    tcp_.connect();
    version_ = Protocol::version;

    // check if already registered via my.info
    std::ifstream f("my.info");
    if (f) {
        std::string name;
        std::getline(f, name);
        if (!name.empty()) {
            username_ = name;
            std::cout<< "Username read from my.info: " << username_ << std::endl;
        }

        std::string id_hex;
        std::getline(f, id_hex);

        // Convert hex string to byte array
        if (id_hex.size() == Protocol::client_id_len * 2) {
            for (size_t i = 0; i < Protocol::client_id_len; i++) {
                std::string byte_str = id_hex.substr(i * 2, 2);
                id_[i] = std::stoul(byte_str, nullptr, 16); 
            }
            std::cout << "Client ID read from my.info: ";
            for (auto b : id_) printf("%02x", b);
            std::cout << std::endl;
        }

        std::string privkey_data;
        std::string line;
        while (std::getline(f, line)) {
            printf("%s", line.c_str());
            privkey_data += line;
        }

        privkey_ = Base64Wrapper::decode(privkey_data);
        
        if (!username_.empty() && !id_.empty() && !privkey_.empty()) {
            std::cout << Color::GREEN << "Welcome, " << username_ << "! You are already registered." << Color::RESET << std::endl;
            return;
        } else {
            std::cout << Color::YELLOW << "[*] Incomplete client details in my.info, please re-register" << Color::RESET << std::endl;
        }
    }

    id_.fill(0);
}

void Client::register_user(const std::string& username, const std::string& pubkey, const std::string& privkey) {
    std::vector<uint8_t> payload;
    payload.resize(Protocol::max_username_len + Protocol::max_pubkey_len, 0);

    const auto name_len = std::min<std::size_t>(username.size(), Protocol::max_username_len - 1);

    memcpy(payload.data(), username.data(), name_len);

    if (pubkey.size() != Protocol::max_pubkey_len) {
        throw std::runtime_error("Public key must be exactly 160 bytes");
    }

    memcpy(payload.data() + Protocol::max_username_len, pubkey.data(), Protocol::max_pubkey_len);

    RequestFrame req;
    req.client_id.fill(0);
    req.version = version_;
    req.code = (uint16_t) RequestCode::REGISTER;
    req.payload = std::move(payload);

    tcp_.send(req.to_bytes());
    auto raw = tcp_.receive();
    auto resp = ResponseFrame::from_bytes(raw);

    if (resp.code == (uint16_t) ResponseCode::REGISTER_OK) {
        memcpy(id_.data(), resp.payload.data(), Protocol::client_id_len);
        username_ = username;
        pubkey_ = pubkey;
        privkey_ = privkey;

        std::cout << "[+] Registered with ID: ";
        for (auto b : id_) printf("%02x", b);
        std::cout << std::endl;

        // Save client details to my.info 
        std::ofstream f("my.info", std::ios::trunc);
        if (!f) {
            throw std::runtime_error("Could not open my.info for writing");
        }

        f << username << std::endl;
        for (auto b : id_) {
            f << std::hex << std::setw(2) << std::setfill('0') << (int)b;
        }
        f << std::endl;

        // TODO: Don't magic write to my.info, use #define

        f << Base64Wrapper::encode(privkey) << std::endl;

        std::cout << "[*] Client details saved to my.info" << std::endl;

    } else {
        std::string msg(resp.payload.begin(), resp.payload.end());
        std::cerr << Color::RED << "[-] Registration failed with code: " << resp.code << "; server message=" << msg << Color::RESET << std::endl;    
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

    if (resp.code != (uint16_t)ResponseCode::LIST_CLIENTS) {
        std::cerr << "[-] LIST_CLIENTS failed with code: " << resp.code << std::endl;
        return;
    }

    size_t rec_size = Protocol::client_id_len + Protocol::max_username_len;
    if (resp.payload.size() % rec_size != 0) {
        std::cerr << "[-] LIST_CLIENTS payload size invalid" << std::endl;
        return;
    }

    size_t num_clients = resp.payload.size() / rec_size;
    std::cout << "[*] " << num_clients << " clients registered:" << std::endl;
    for (size_t i = 0; i < num_clients; i++) {
        auto offset = i * rec_size;
        std::array<uint8_t, Protocol::client_id_len> client_id;
        memcpy(client_id.data(), &resp.payload[offset], Protocol::client_id_len);

        char username[Protocol::max_username_len + 1] = {0};
        memcpy(username, &resp.payload[offset + Protocol::client_id_len], Protocol::max_username_len);

        printf("    ID: ");
        for (auto b : client_id) {
            printf("%02x", b);
        }
        printf(" | Username: %s\n", username);
    }
}
