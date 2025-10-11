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
    if (id_hex.size() == Protocol::client_id_len * 2) {
        auto bytes = hex_to_bytes(id_hex);
        std::copy_n(bytes.begin(), Protocol::client_id_len, id_.begin());
    } else {
        id_.fill(0);
    }
    
    // read private key which is in base64 and needs decoding
    std::string privkey_data((std::istreambuf_iterator<char>(f)), {});
    if (!privkey_data.empty())
        privkey_ = Base64Wrapper::decode(privkey_data);

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
    f << Base64Wrapper::encode(privkey_) << "\n";
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
std::vector<uint8_t> Client::build_register_payload(const std::string& username, const std::string& pubkey) {
    std::vector<uint8_t> payload(Protocol::max_username_len + Protocol::max_pubkey_len, 0);
    size_t name_len = std::min(username.size(), static_cast<size_t>(Protocol::max_username_len - 1));
    memcpy(payload.data(), username.data(), name_len);

    if (pubkey.size() != Protocol::max_pubkey_len)
        throw std::runtime_error("Public key must be exactly 160 bytes");

    memcpy(payload.data() + Protocol::max_username_len, pubkey.data(), Protocol::max_pubkey_len);
    return payload;
}

/**
 * @brief Main entry for register a new user
 * Prompts user for userrname, generates RSA pubkey & privkey pair
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
    privkey_ = priv.getPrivateKey();
    pubkey_ = priv.getPublicKey();

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