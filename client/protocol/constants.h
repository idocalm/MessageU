#ifndef CONSTANTS_H
#define CONSTANTS_H

#define CLIENT_FILE "me.info"
#define CLIENT_CONFIG "server.info"
#include <iomanip>

namespace Protocol {
    const int version = 2;

    const int version_len = 1;
    const int code_len = 2;
    const int payload_size_len = 4;

    const int client_id_len = 16;
    const int max_username_len = 255;
    const int max_pubkey_len = 160;

    const int header_len_req = client_id_len + version_len + code_len + payload_size_len;
    const int header_len_resp = version_len + code_len + payload_size_len;
}

inline uint16_t read_le16(const uint8_t* p) {
    // Read a 16-bit little-endian value from a byte pointer
    return (uint16_t(p[1]) << 8) | uint16_t(p[0]);
}

inline uint32_t read_le32(const uint8_t* p) {
    // Read a 32-bit little-endian value from a byte pointer
    return (uint32_t(p[0])      ) |
           (uint32_t(p[1]) <<  8) |
           (uint32_t(p[2]) << 16) |
           (uint32_t(p[3]) << 24);
}

inline void put_le16(std::vector<uint8_t>& b, uint16_t v) {
    // Push a 16-bit value in little-endian order
    b.push_back(uint8_t(v & 0xFF));
    b.push_back(uint8_t(v >> 8));
}

inline void put_le32(std::vector<uint8_t>& b, uint32_t v) {
    // Push a 32-bit value in little-endian order
    b.push_back(uint8_t(v & 0xFF));
    b.push_back(uint8_t((v >> 8) & 0xFF));
    b.push_back(uint8_t((v >> 16) & 0xFF));
    b.push_back(uint8_t((v >> 24) & 0xFF));
}

inline std::vector<uint8_t> hex_to_bytes(const std::string& hex) {
    std::vector<uint8_t> bytes;
    bytes.reserve(hex.size() / 2);
    for (size_t i = 0; i < hex.size(); i += 2) {
        std::string byte_str = hex.substr(i, 2);
        uint8_t byte = std::stoul(byte_str, nullptr, 16);
        bytes.push_back(byte);
    }
    return bytes;
}

inline std::string to_hex(const std::vector<uint8_t>& data) {
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (uint8_t b : data)
        oss << std::setw(2) << static_cast<int>(b);
    return oss.str();
}


#endif