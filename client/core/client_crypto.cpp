#include "client.h"

/**
 * @brief Decrypts a message given the symmetric key used to encrypt it
 * 
 * @param content - the encrypted content 
 * @param symkey  - the key
 * @return std::string - readable unencrypted string / error message 
 */
std::string Client::decrypt_message(const std::string& content, const std::vector<uint8_t>& symkey) {
    if (symkey.empty()) {
        return "[no symmetric key established]";
    }

    try {
        AESWrapper aes(reinterpret_cast<const unsigned char*>(symkey.data()), AESWrapper::DEFAULT_KEYLENGTH);
        return aes.decrypt(content.data(), content.size());
    } catch (const std::exception& e) {
        return std::string("[decryption failed: ") + e.what() + "]";
    }
}

/**
 * @brief Sends a symmetric key request message to another client 
 */
void Client::request_sym_key() {
    std::array<uint8_t, Protocol::client_id_len> dest_id{};
    if (!get_dest_user(dest_id))
        return;

    // use the send_message helper with type SYM_REQ that has no content
    if (send_message(dest_id, MessageType::SYM_REQ, {}))
        OK("Symmetric key request sent.");
    else
        ERR("Failed to send symmetric key request.");
}

/**
 * @brief Generates and sends a new sym key to another client
 * 
 * The key is encrypted using the destination user public key!
 * 
 */
void Client::send_sym_key() {
    std::array<uint8_t, Protocol::client_id_len> dest_id{};
    if (!get_dest_user(dest_id))
        return;


    // If we don't know the user public key, we can't send a sym key.
    const std::string dest_hex = id_to_hex(dest_id);
    auto it = other_clients_.find(dest_hex);
    if (it == other_clients_.end() || it->second.pubkey.empty()) {
        ERR("Missing public key. Run GET_PUBKEY first.");
        return;
    }

    // generate symmetric key
    unsigned char sym_key[AESWrapper::DEFAULT_KEYLENGTH];
    AESWrapper::GenerateKey(sym_key, AESWrapper::DEFAULT_KEYLENGTH);

    // encrypt key with RSA public key
    RSAPublicWrapper rsa_pub(it->second.pubkey);
    std::string encrypted_key;
    try {
        encrypted_key = rsa_pub.encrypt(reinterpret_cast<const char*>(sym_key), AESWrapper::DEFAULT_KEYLENGTH);
    } catch (const std::exception& e) {
        ERR("RSA encryption failed: " << e.what());
        return;
    }

    std::vector<uint8_t> payload(encrypted_key.begin(), encrypted_key.end());

    // use the send_message helper this time with content
    if (send_message(dest_id, MessageType::SYM_KEY, payload)) {
        OK("Symmetric key sent successfully.");
        it->second.symkey.assign(sym_key, sym_key + AESWrapper::DEFAULT_KEYLENGTH);
    } else {
        ERR("Failed to send symmetric key.");
    }
}

/**
 * @brief Handles a received sym key from another client
 * Decrypts the key with the local private key (paired with the pubkey), and stores it 
 * in the other_clients_ table for that client. 
 * 
 * @param from_id 
 * @param encrypted 
 */
void Client::handle_incoming_sym_key(const std::array<uint8_t, Protocol::client_id_len>& from_id, const std::vector<uint8_t>& encrypted) {

    // We must be familiar already with that user
    const std::string from_hex = id_to_hex(from_id);
    auto it = other_clients_.find(from_hex);
    if (it == other_clients_.end()) {
        ERR("Unknown user for received symmetric key.");
        return;
    }

    try {
        RSAPrivateWrapper rsa_priv(privkey_);
        std::string decrypted = rsa_priv.decrypt(reinterpret_cast<const char*>(encrypted.data()),
                                                 encrypted.size());
        if (decrypted.size() != AESWrapper::DEFAULT_KEYLENGTH) {
            ERR("Invalid decrypted key length.");
            return;
        }

        it->second.symkey.assign(decrypted.begin(), decrypted.end());
        OK("Symmetric key stored for " << (it->second.username.empty() ? from_hex : it->second.username));
    } catch (const std::exception& e) {
        ERR("Failed to decrypt symmetric key: " << e.what());
    }
}

