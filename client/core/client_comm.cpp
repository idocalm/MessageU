#include "client.h"

/**
 * @brief Helper function for receiving dest user from client.
 * 
 * In default, prompt for username. if it's cached in other_clients_, we can get the id from there. 
 * If it's not, it fallbacks for manual id
 * 
 * This allows users to search other users by their name and not id given that its after they did LIST_CLIENTS.
 * 
 * @param id - buffer to fill with the dest id 
 * @return true - if a valid dest user id was received
 * @return false - otherwise
 */
bool Client::get_dest_user(std::array<uint8_t, Protocol::client_id_len>& id) {
    std::string username;
    std::cout << "Enter username: ";
    std::getline(std::cin, username);

    if (username.empty()) {
        ERR("Username cannot be empty.");
        return false;
    }

    

    // Check cache first
    for (const auto& [id_hex, oc] : other_clients_) {
        if (oc.username == username) {
            auto hex = id_hex;
            for (size_t i = 0; i < Protocol::client_id_len; i++) {
                std::string byte_str = hex.substr(i * 2, 2);
                // check that the hex is really hex
                if (byte_str.size() != 2 || !std::all_of(byte_str.begin(), byte_str.end(), ::isxdigit)) {
                    ERR("Invalid hex string");
                    return false;
                }
                id[i] = static_cast<uint8_t>(std::stoul(byte_str, nullptr, 16));
            }
            return true;
        }
    }

    // Fallback to manual ID
    INFO("Username not found. Enter 32-hex client ID manually:");
    std::string id_hex;
    std::getline(std::cin, id_hex);
    if (id_hex.size() != Protocol::client_id_len * 2) {
        ERR("Invalid ID length. Must be 32 hex characters.");
        return false;
    }

    for (size_t i = 0; i < Protocol::client_id_len; i++) {
        std::string byte_str = id_hex.substr(i * 2, 2);
        id[i] = static_cast<uint8_t>(std::stoul(byte_str, nullptr, 16));
    }
    return true;
}

/**
 * @brief Helper to send a message request (code 603) easily
 * It handles filling the RequestFrame with the correct payload of a MessagePayload
 * according to the MessageType
 * 
 * @param to_id - message dest user id
 * @param type - message type
 * @param content - message content
 * @return true - if sent successful
 * @return false - otherwise
 */
bool Client::send_message(const std::array<uint8_t, Protocol::client_id_len>& to_id, MessageType type, const std::vector<uint8_t>& content) {
    if (type == MessageType::SYM_REQ && !content.empty()) {
        ERR("SYM_REQ cannot have a payload.");
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

    tcp_.send(req.to_bytes());
    auto resp = ResponseFrame::from_bytes(tcp_.receive());

    if (resp.code == static_cast<uint16_t>(ResponseCode::SEND_OK)) {
        OK("Message sent successfully.");
        return true;
    }

    ERR("Message send failed. Response code: " << resp.code << "; " << std::string(resp.payload.begin(), resp.payload.end()));
    return false;
}

/**
 * @brief Requests and prints the list of all registered clients from server
 * Updates the local cache with username and ids
 */
void Client::list_clients() {
    RequestFrame req;
    req.client_id = id_;
    req.version = version_;
    req.code = (uint16_t) RequestCode::LIST_CLIENTS;
    req.payload.clear();

    tcp_.send(req.to_bytes());
    auto resp = ResponseFrame::from_bytes(tcp_.receive());

    if (resp.code != static_cast<uint16_t>(ResponseCode::LIST_CLIENTS)) {
        ERR("LIST_CLIENTS failed with code: " << resp.code << "; " << std::string(resp.payload.begin(), resp.payload.end()));
        return;
    }

    const size_t rec_size = Protocol::client_id_len + Protocol::max_username_len;
    if (resp.payload.size() % rec_size != 0) {
        ERR("Invalid LIST_CLIENTS payload size");
        return;
    }

    // print all the users
    size_t num_clients = resp.payload.size() / rec_size;
    INFO(num_clients << " clients registered:");
    for (size_t i = 0; i < num_clients; i++) {
        size_t off = i * rec_size;
        std::array<uint8_t, Protocol::client_id_len> cid{};
        memcpy(cid.data(), &resp.payload[off], Protocol::client_id_len);

        char uname[Protocol::max_username_len + 1] = {0};
        memcpy(uname, &resp.payload[off + Protocol::client_id_len], Protocol::max_username_len);

        std::string id_hex = id_to_hex(cid);
        std::cout << "    ID: " << id_hex << " | Username: " << uname << std::endl;
        
        // Save to the cache
        auto& entry = other_clients_[id_hex];
        entry.username = uname;
    }
    std::cout << std::endl;
}

/**
 * @brief Pulls and displays all pending messages from the server. 
 * If the message contains text - it will attempt to decrypt it and display the content 
 * If the message is a symmetric key request - it will show "Request for symmetric key" as content
 * If the message is an encrytped symmetric key - it will decrypt it, and store it for the correct user  
 * 
 */
void Client::pull_messages() {
    RequestFrame req;
    req.client_id = id_;
    req.version = version_;
    req.code = (uint16_t) RequestCode::PULL_MESSAGES;
    req.payload.clear();

    tcp_.send(req.to_bytes());
    auto resp = ResponseFrame::from_bytes(tcp_.receive());

    if (resp.code != static_cast<uint16_t>(ResponseCode::PULL_MESSAGES)) {
        ERR("PULL_MESSAGES failed with code: " << resp.code << "; " << std::string(resp.payload.begin(), resp.payload.end()));
        return;
    }

    const auto& data = resp.payload;
    size_t off = 0;
    if (data.empty()) {
        INFO("No messages found.");
        return;
    }

    while (off + Protocol::client_id_len + 1 + 4 <= data.size()) {
        std::array<uint8_t, Protocol::client_id_len> from{};
        memcpy(from.data(), &data[off], Protocol::client_id_len);
        off += Protocol::client_id_len;

        uint8_t type = data[off++];
        uint32_t sz = read_le32(&data[off]);
        off += 4;

        if (off + sz > data.size()) {
            ERR("Malformed message payload.");
            break;
        }

        std::vector<uint8_t> content(data.begin() + off, data.begin() + off + sz);
        off += sz;

        std::string from_hex = id_to_hex(from);
        auto it = other_clients_.find(from_hex);

        std::string sender = from_hex;
        if (it != other_clients_.end() && !it->second.username.empty())
            sender = it->second.username;

        std::cout << "----<SOM>----\n";
        std::cout << Color::CYAN << "From: " << sender << Color::RESET << std::endl;

        if (type == static_cast<uint8_t>(MessageType::SYM_REQ)) {
            std::cout << "Content: Request for symmetric key" << std::endl;
        } else if (type == static_cast<uint8_t>(MessageType::SYM_KEY)) {
            // Save the smy key for that user so we can message end to end
            std::cout << "Content: Symmetric key received" << std::endl;
            handle_incoming_sym_key(from, content);
        } else {
            // for normal message try to decrypt and show content 
            std::string text = decrypt_message(std::string(content.begin(), content.end()), it->second.symkey);
            std::cout << "Content: " << text << std::endl;
        }
        std::cout << "----<EOM>----\n\n";
    }
}

/**
 * @brief Prompts for a user and a message, encrypts it with sym key and sends it
 * Checks if the user is known to us
 * Checks if we exchanged a sym key with the user
 * 
 */
void Client::send_mesage_to_client() {
    std::array<uint8_t, Protocol::client_id_len> dest{};
    if (!get_dest_user(dest))
        return;

    const std::string dest_hex = id_to_hex(dest);
    auto it = other_clients_.find(dest_hex);
    if (it == other_clients_.end()) {
        ERR("Unknown dest user. Run LIST_CLIENTS first.");
        return;
    }

    // 
    if (it->second.symkey.empty()) {
        ERR("No symmetric key established with this user. You first need to exchange.");
        return;
    }

    std::string message;
    std::cout << "Enter message: ";
    std::getline(std::cin, message);
    if (message.empty()) {
        ERR("Message cannot be empty.");
        return;
    }

    // Encrypt the message with the sym key 
    AESWrapper aes(reinterpret_cast<const unsigned char*>(it->second.symkey.data()), AESWrapper::DEFAULT_KEYLENGTH);
    std::string encrypted;
    try {
        encrypted = aes.encrypt(message.c_str(), message.size());
    } catch (const std::exception& e) {
        ERR("AES encryption failed: " << e.what());
        return;
    }

    // Payload is just the encrypted message, and send
    std::vector<uint8_t> payload(encrypted.begin(), encrypted.end());
    if (send_message(dest, MessageType::TEXT, payload))
        OK("Encrypted message sent to " << (it->second.username.empty() ? dest_hex : it->second.username));
    else
        ERR("Failed to send message.");
}

/**
 * @brief Requests the public key of another client and saves it locally
 * The public keys for all users are stored in server. 
 */
void Client::get_pubkey() {
    std::array<uint8_t, Protocol::client_id_len> dest_id;
    if (!get_dest_user(dest_id))
        return;

    RequestFrame req; 
    req.client_id = id_; 
    req.version = version_; 
    req.code = (uint16_t) RequestCode::GET_PUBKEY; 
    req.payload.assign(dest_id.begin(), dest_id.end());

    tcp_.send(req.to_bytes());
    auto resp = ResponseFrame::from_bytes(tcp_.receive());

    if (resp.code != (uint16_t) ResponseCode::GET_PUBKEY) {
        ERR("GET_PUBKEY failed with code: " << resp.code);
        return;
    }

    if (resp.payload.size() < Protocol::client_id_len) {
        ERR("Response too short for GET_PUBKEY.");
        return;
    }

    std::vector<uint8_t> resp_client_id(resp.payload.begin(), resp.payload.begin() + Protocol::client_id_len);
    std::vector<uint8_t> pubkey(resp.payload.begin() + Protocol::client_id_len, resp.payload.end());

    if (!std::equal(dest_id.begin(), dest_id.end(), resp_client_id.begin())) {
        ERR("Mismatched client ID in response!");
        return;
    }

    // Save to other_clients_
    const std::string id = to_hex(resp_client_id);
    OtherClient& entry = other_clients_[id];
    entry.pubkey.assign(reinterpret_cast<const char*>(pubkey.data()), pubkey.size());
    OK("Public key for " << (entry.username.empty() ? id : entry.username) << " stored successfully.");
}
