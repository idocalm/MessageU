#include "client.h"
#include "menu.h"

Client::Client() : tcp_() {
    tcp_.connect();
    version_ = Protocol::version;

    // check if already registered
    std::ifstream f(CLIENT_FILE);
    if (f) {
        std::string name;
        std::getline(f, name);
        if (!name.empty()) {
            username_ = name;
        }

        std::string id_hex;
        std::getline(f, id_hex);

        // Convert hex string to byte array
        if (id_hex.size() == Protocol::client_id_len * 2) {
            for (size_t i = 0; i < Protocol::client_id_len; i++) {
                std::string byte_str = id_hex.substr(i * 2, 2);
                id_[i] = std::stoul(byte_str, nullptr, 16); 
            }
        }

        std::string privkey_data;
        std::string line;
        while (std::getline(f, line)) {
            privkey_data += line;
        }

        privkey_ = Base64Wrapper::decode(privkey_data);
        
        if (!username_.empty() && !id_.empty() && !privkey_.empty()) {
            std::cout << Color::GREEN << "Welcome, " << username_ << "! You are already registered." << Color::RESET << std::endl;
            return;
        } else {
            std::cout << Color::YELLOW << "[*] Incomplete client details in " << CLIENT_FILE << ", please re-register" << Color::RESET << std::endl;
        }
    }

    id_.fill(0);
}

bool Client::find_user_id(const std::string& username, std::array<uint8_t, Protocol::client_id_len>& id) {
    for (const auto& [id_hex, oc] : other_clients_) {
        if (oc.username == username) {
            auto bytes = hex_to_bytes(id_hex);
            if (bytes.size() == Protocol::client_id_len) {
                memcpy(id.data(), bytes.data(), Protocol::client_id_len);
                return true;
            }
        }
    }

    return false;
}

bool Client::get_dest_user(std::array<uint8_t, Protocol::client_id_len>& id) {
    std::string username;
    std::cout << "Enter username: ";
    std::getline(std::cin, username);

    std::array<uint8_t, 16> target_id;
    if (!find_user_id(username, target_id)) {
        std::cout << Color::YELLOW << "[!] Username not found. Enter user id (32 hex) or press Enter to cancel: " << Color::RESET;
        std::string id_hex;
        std::getline(std::cin, id_hex);
        if (id_hex.empty()) {
            std::cerr << Color::RED << "[-] Aborted. Use option 120 to fetch users first." << Color::RESET << std::endl;
            return false;
        }
        auto bytes = hex_to_bytes(id_hex);
        if (bytes.size() != Protocol::client_id_len) {
            std::cerr << Color::RED << "[-] Invalid user ID length. Must be 16 bytes (32 hex chars)." << Color::RESET << std::endl;
            return false;
        }

        std::copy(bytes.begin(), bytes.end(), id.begin());
        return true;
    }

    std::copy(target_id.begin(), target_id.end(), id.begin());
    return true;

}

bool Client::send_message(const std::array<uint8_t, Protocol::client_id_len>& to_id, MessageType type, const std::vector<uint8_t>& content) {
    if (type == MessageType::SYM_REQ && !content.empty()) {
        std::cerr << Color::RED << "[-] SYM_REQ message can't contain payload." << Color::RESET << std::endl;
        return false;
    }

    MessagePayload msg;
    msg.dest_id = to_id;
    msg.type = static_cast<uint8_t>(type); 
    msg.content_size = content.size();
    msg.content = content;

    RequestFrame req;
    req.client_id = id_;
    req.version = version_;
    req.code = (uint16_t) RequestCode::SEND_MESSAGE;
    req.payload = msg.to_bytes();

    // send and wait for response
    tcp_.send(req.to_bytes());
    auto raw = tcp_.receive();
    auto resp = ResponseFrame::from_bytes(raw);

    if (resp.code == (uint16_t) ResponseCode::SEND_OK) {
        std::cout << Color::GREEN << "[+] Message sent successfully." << Color::RESET << std::endl;
        return true;
    }

    std::cerr << Color::RED << "[-] Message send failed. Response code: " << resp.code << Color::RESET << std::endl;
    return false;

}


void Client::register_user() {
    if (!privkey_.empty() && !username_.empty() && !id_.empty()) {
        std::cout << Color::RED << "You are already connected with username: " << username_ << Color::RESET << std::endl; 
        return;
    } 

    std::string username;
    std::cout << "Enter username: ";
    std::getline(std::cin, username);

    RSAPrivateWrapper private_key;
    std::string privkey = private_key.getPrivateKey();
    std::string pubkey = private_key.getPublicKey();

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

        std::ofstream f(CLIENT_FILE, std::ios::trunc);
        if (!f) {
            throw std::runtime_error("Could not open " CLIENT_FILE " for writing");
        }

        f << username << std::endl;
        for (auto b : id_) {
            f << std::hex << std::setw(2) << std::setfill('0') << (int)b;
        }
        f << std::endl;


        f << Base64Wrapper::encode(privkey) << std::endl;

        std::cout << "[*] Client details saved to " << CLIENT_FILE << std::endl;

    } else {
        std::string msg(resp.payload.begin(), resp.payload.end());
        std::cerr << Color::RED << "[-] Registration failed with code: " << resp.code << "; server message = " << msg << Color::RESET << std::endl;    
    }
}

void Client::get_pubkey() {
    std::array<uint8_t, Protocol::client_id_len> dest_id;
    if (!get_dest_user(dest_id)) {
        return;
    }

    RequestFrame req; 
    req.client_id = id_; 
    req.version = version_; 
    req.code = (uint16_t) RequestCode::GET_PUBKEY; 
    req.payload.assign(dest_id.begin(), dest_id.end());

    tcp_.send(req.to_bytes());
    auto raw = tcp_.receive();
    auto resp = ResponseFrame::from_bytes(raw);

    if (resp.code != (uint16_t) ResponseCode::GET_PUBKEY) {
        std::cerr << Color::RED << "[-] GET_PUBKEY failed with code: " << resp.code << Color::RESET << std::endl;
        return;
    }

    if (resp.payload.size() < Protocol::client_id_len) {
        std::cerr << Color::RED << "[-] Response too short." << Color::RESET << std::endl;
        return;
    }

    std::vector<uint8_t> resp_client_id(resp.payload.begin(), resp.payload.begin() + Protocol::client_id_len);
    std::vector<uint8_t> pubkey(resp.payload.begin() + Protocol::client_id_len, resp.payload.end());

    if (!std::equal(dest_id.begin(), dest_id.end(), resp_client_id.begin())) {
        std::cerr << Color::RED << "[-] Mismatched client ID in response!" 
                << Color::RESET << std::endl;
        return;
    }


    const std::string peerIdHex = to_hex(resp_client_id);
    OtherClient& entry = other_clients_[peerIdHex];
    entry.pubkey.assign(reinterpret_cast<const char*>(pubkey.data()), pubkey.size());
    std::cout << Color::GREEN << "[+] Public key saved successfully.";

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
        std::cerr << Color::RED << "[-] LIST_CLIENTS failed with code: " << resp.code << Color::RESET << std::endl;
        return;
    }

    size_t rec_size = Protocol::client_id_len + Protocol::max_username_len;
    if (resp.payload.size() % rec_size != 0) {
        std::cerr << Color::RED << "[-] LIST_CLIENTS payload size invalid" << Color::RESET << std::endl;
        return;
    }

    size_t num_clients = resp.payload.size() / rec_size;
    std::cout << Color::GREEN << "[*] " << num_clients << " clients registered (excluding you):" << std::endl;
    for (size_t i = 0; i < num_clients; i++) {
        auto offset = i * rec_size;
        std::array<uint8_t, Protocol::client_id_len> client_id;
        memcpy(client_id.data(), &resp.payload[offset], Protocol::client_id_len);

        char username[Protocol::max_username_len + 1] = {0};
        memcpy(username, &resp.payload[offset + Protocol::client_id_len], Protocol::max_username_len);

        std::cout << Color::GREEN << "    ID: "; 
        for (auto b : client_id) printf("%02x", b);
        std::cout << " | Username: " << username << std::endl; 

        // Enter username into other_clients chace
        std::vector<uint8_t> id_vec(client_id.begin(), client_id.end());
        const std::string id_hex = to_hex(id_vec);
        auto& entry = other_clients_[id_hex];
        entry.username = username;
    }
    std::cout << Color::RESET << std::endl;
}


void Client::pull_messages() {
    RequestFrame req;
    req.client_id = id_;
    req.version = version_;
    req.code = (uint16_t)RequestCode::PULL_MESSAGES;
    req.payload.clear();

    tcp_.send(req.to_bytes());
    auto raw = tcp_.receive();
    auto resp = ResponseFrame::from_bytes(raw);

    if (resp.code != (uint16_t)ResponseCode::PULL_MESSAGES) {
        std::cerr << Color::RED << "[-] Pull messages failed with code: "
                  << resp.code << Color::RESET << std::endl;
        return;
    }

    const auto& data = resp.payload;
    size_t offset = 0;
    while (offset + Protocol::client_id_len + 1 + 4 <= data.size()) {
        std::array<uint8_t, Protocol::client_id_len> from_id;
        memcpy(from_id.data(), &data[offset], Protocol::client_id_len);
        offset += Protocol::client_id_len;

        uint8_t type = data[offset++];
        uint32_t content_size = read_le32(&data[offset]);
        offset += 4;

        if (offset + content_size > data.size()) {
            std::cerr << Color::RED << "[-] Malformed message payload." << Color::RESET << std::endl;
            break;
        }

        std::vector<uint8_t> content(data.begin() + offset, data.begin() + offset + content_size);
        offset += content_size;

        const std::string from_hex = to_hex(std::vector<uint8_t>(from_id.begin(), from_id.end()));
        std::string from_user = from_hex;
        auto it = other_clients_.find(from_hex);
        if (it != other_clients_.end() && !it->second.username.empty())
            from_user = it->second.username;

        std::cout << "----<SOM>----" << std::endl;

        std::cout << Color::CYAN << "From: " << from_user << Color::RESET << std::endl;

        if (type == static_cast<uint8_t>(MessageType::SYM_REQ)) {
            std::cout << "Content: A Request for a symmetric key" << std::endl;
        } 
        else if (type == static_cast<uint8_t>(MessageType::SYM_KEY)) {
            std::cout << "Content: Symmetric key received" << std::endl;

            if (it == other_clients_.end()) {
                std::cerr << Color::YELLOW
                          << "[!] Unknown user for received symmetric key. Run option 120 first."
                          << Color::RESET << std::endl;
            } else {
                try {
                    RSAPrivateWrapper rsa_priv(privkey_);
                    std::string decrypted_key = rsa_priv.decrypt(
                        reinterpret_cast<const char*>(content.data()), content.size());
                    if (decrypted_key.size() == AESWrapper::DEFAULT_KEYLENGTH) {
                        it->second.symkey.assign(decrypted_key.begin(), decrypted_key.end());
                        std::cout << Color::GREEN << "[+] Symmetric key stored for user. You can now communicate freely."
                                  << Color::RESET << std::endl;
                    } else {
                        std::cerr << Color::YELLOW << "[!] Decrypted key length invalid." << Color::RESET << std::endl;
                    }
                } catch (const std::exception& e) {
                    std::cerr << Color::RED << "[-] Failed to decrypt symmetric key: " << e.what()
                              << Color::RESET << std::endl;
                }
            }
        } 
        else {
            // Regular text message
            std::string text;
            if (it == other_clients_.end() || it->second.symkey.empty()) {
                text = "Can't decrypt message; no key";
            } else {
                try {
                    AESWrapper aes(reinterpret_cast<const unsigned char*>(it->second.symkey.data()),
                                   AESWrapper::DEFAULT_KEYLENGTH);
                    text = aes.decrypt(reinterpret_cast<const char*>(content.data()), content.size());
                } catch (...) {
                    text = "Can't decrypt message; wrong key";
                }
            }
            std::cout << "Content: " << text << std::endl;
        }

        std::cout << "----<EOM>----\n" << std::endl;
    }

    if (offset == 0)
        std::cout << Color::YELLOW << "[*] No messages found." << Color::RESET << std::endl;
}

void Client::request_sym_key() {
    std::array<uint8_t, Protocol::client_id_len> dest_id;
    if (!get_dest_user(dest_id))
        return;

    if (send_message(dest_id, MessageType::SYM_REQ, {})) {
        std::cout << Color::GREEN << "[+] Symmetric key request sent." << Color::RESET << std::endl;
    } else {
        std::cerr << Color::RED << "[-] Failed to send symmetric key request." << Color::RESET << std::endl;
    }
}

void Client::send_mesage_to_client() {
    std::array<uint8_t, Protocol::client_id_len> dest_id;

    if (!get_dest_user(dest_id))
        return;

    const std::string dest_hex = to_hex(std::vector<uint8_t>(dest_id.begin(), dest_id.end()));
    auto it = other_clients_.find(dest_hex);
    if (it == other_clients_.end()) {
        std::cerr << Color::RED 
                  << "[-] Unknown destination. Run option 120 (LIST_CLIENTS) first."
                  << Color::RESET << std::endl;
        return;
    }

    OtherClient &dest = it->second;
    if (dest.symkey.empty()) {
        std::cerr << Color::RED 
                  << "[-] No symmetric key established with this client. "
                  << Color::RESET << std::endl;
        return;
    }

     std::string message;
    std::cout << "Enter message: ";
    std::getline(std::cin, message);
    if (message.empty()) {
        std::cerr << Color::YELLOW << "[!] Message cannot be empty." << Color::RESET << std::endl;
        return;
    }

    AESWrapper aes(reinterpret_cast<const unsigned char*>(dest.symkey.data()),
                   AESWrapper::DEFAULT_KEYLENGTH);

    std::string cipher;
    try {
        cipher = aes.encrypt(message.c_str(), message.size());
    } catch (const std::exception &e) {
        std::cerr << Color::RED << "[-] AES encryption failed: " << e.what() << Color::RESET << std::endl;
        return;
    }

    std::vector<uint8_t> payload(cipher.begin(), cipher.end());

    if (send_message(dest_id, MessageType::TEXT, payload)) {
        std::cout << Color::GREEN << "[+] Message sent successfully to " 
                  << (dest.username.empty() ? dest_hex : dest.username) 
                  << Color::RESET << std::endl;
    } else {
        std::cerr << Color::RED << "[-] Failed to send message." << Color::RESET << std::endl;
    }
}

void Client::send_sym_key() {
    std::array<uint8_t, Protocol::client_id_len> dest_id;

    if (!get_dest_user(dest_id))
        return;

    const std::string dest_hex = to_hex(std::vector<uint8_t>(dest_id.begin(), dest_id.end()));
    auto it = other_clients_.find(dest_hex);
    if (it == other_clients_.end() || it->second.pubkey.empty()) {
        std::cerr << Color::RED 
                  << "[-] Missing public key for this user. "
                  << "Use option 130 (GET_PUBKEY) first."
                  << Color::RESET << std::endl;
        return;
    }

    const std::string& dest_pubkey = it->second.pubkey;

    // Create a new symmetric key 
    unsigned char sym_key[AESWrapper::DEFAULT_KEYLENGTH];
    AESWrapper::GenerateKey(sym_key, AESWrapper::DEFAULT_KEYLENGTH);

    RSAPublicWrapper rsa_pub(dest_pubkey);
    std::string encrypted_key;
    try {
        encrypted_key = rsa_pub.encrypt(reinterpret_cast<const char*>(sym_key), AESWrapper::DEFAULT_KEYLENGTH);
    } catch (const std::exception& e) {
        std::cerr << Color::RED << "[-] RSA encryption failed: " << e.what() << Color::RESET << std::endl;
        return;
    }

    std::vector<uint8_t> payload(encrypted_key.begin(), encrypted_key.end());

    if (send_message(dest_id, MessageType::SYM_KEY, payload)) {
        std::cout << Color::GREEN << "[+] Symmetric key sent successfully." << Color::RESET << std::endl;
        it->second.symkey.assign(sym_key, sym_key + AESWrapper::DEFAULT_KEYLENGTH);
    } else {
        std::cerr << Color::RED << "[-] Failed to send symmetric key." << Color::RESET << std::endl;
    }
}