#include "client.h"

/**
 * @brief Construct a new Client object
 * Connects to the server, loads local client data if exists 
 * and inits username, id and private key

 */
Client::Client() : tcp_() {
    tcp_.connect();
    version_ = Protocol::version;

    // check if already registered
    std::ifstream f(CLIENT_FILE);
    if (!f) {
        id_.fill(0);
        return;
    }

    std::getline(f, username_);
    std::string id_hex; 
    std::getline(f, id_hex);

    // parse ID from hex if valid
    if (!hex_to_id(id_hex, id_)) {
        id_.fill(0);
    }
    
    // read private key which is in base64 and decod e
    std::string privkey_data((std::istreambuf_iterator<char>(f)), {});
    if (!privkey_data.empty()) {
        std::string decoded = Base64Wrapper::decode(privkey_data);
        privkey_.assign(decoded.begin(), decoded.end());
    }

    if (!username_.empty() && !privkey_.empty()) {
        OK("Welcome, " << username_ << "! Loaded existing client.");
    } else {
        INFO("Incomplete client details in " << CLIENT_FILE << "; please re-register.");
        id_.fill(0);
    }

}

/**
 * @brief Helper function to save client data after a registration into CLIENT_fILE
 * @throws std::runtime_error if file can't be opened
 * 
 */
void Client::save_client_file() {
    // File format:
    /*
        first line is username
        second is id in hex 
        third and on is the priv key in base64
    */
    std::ofstream f(CLIENT_FILE, std::ios::trunc);
    if (!f)
        throw std::runtime_error("Cannot open " CLIENT_FILE " for writing");

    f << username_ << "\n";
    f << to_hex(std::vector<uint8_t>(id_.begin(), id_.end())) << "\n";
    f << Base64Wrapper::encode(std::string(privkey_.begin(), privkey_.end())) << "\n";
    INFO("Client data saved to " << CLIENT_FILE);
}

/**
 * @brief Creates the registration payload from the username and the public key generated
 * 
 * @param username - up to Protocol::max_username_len bytes 
 * @param pubkey  - up to Protcol::max_pubkey_len bytes
 * @return std::vector<uint8_t> - serialized register payload
 * @throws std::runtime_error if public key length is bad
 */
std::vector<uint8_t> Client::build_register_payload(const std::string& username, const std::array<uint8_t, Protocol::max_pubkey_len>& pubkey) {
    std::vector<uint8_t> payload(Protocol::max_username_len + Protocol::max_pubkey_len, 0);
    size_t name_len = std::min(username.size(), static_cast<size_t>(Protocol::max_username_len - 1));
    memcpy(payload.data(), username.data(), name_len);

    memcpy(payload.data() + Protocol::max_username_len, pubkey.data(), Protocol::max_pubkey_len);
    return payload;
}

/**
 * @brief Main entry for register a new user
 * Prompts user for userrname, generates pub key & privkey pair
 * Builds a RequestFrame with build_register_payload and sends it. 
 * Saves client details if success with save_client_file
 * 
 */
void Client::register_user() {
    if (!privkey_.empty() && !username_.empty() && !id_.empty()) {
        ERR("Already registered as " << username_);
        return;
    } 

    std::cout << "Enter username: ";
    std::getline(std::cin, username_);
    if (username_.empty()) {
        ERR("Username cannot be empty.");
        return;
    }

    // generate new RSA keypair
    RSAPrivateWrapper priv;
    std::string privkey_str = priv.getPrivateKey();
    privkey_.assign(privkey_str.begin(), privkey_str.end());
    
    std::string pubkey_str = priv.getPublicKey();
    if (pubkey_str.size() != Protocol::max_pubkey_len) {
        ERR("Generated public key has wrong size");
        return;
    }

    std::copy_n(pubkey_str.begin(), Protocol::max_pubkey_len, pubkey_.begin());

    RequestFrame req;
    req.client_id.fill(0);
    req.version = version_;
    req.code = (uint16_t) RequestCode::REGISTER;
    req.payload = build_register_payload(username_, pubkey_);

    tcp_.send(req.to_bytes());
    auto resp = ResponseFrame::from_bytes(tcp_.receive());
    
    // receive server response
    if (resp.code != (uint16_t) ResponseCode::REGISTER_OK) {
        std::string msg(resp.payload.begin(), resp.payload.end());
        ERR("Registration failed (" << resp.code << "): " << msg);
        return;
    }

    std::copy_n(resp.payload.begin(), Protocol::client_id_len, id_.begin());
    OK("Registered successfully. ID: " << to_hex(std::vector<uint8_t>(id_.begin(), id_.end())));

    save_client_file();
}