#ifndef CONSTANTS_H
#define CONSTANTS_H

#define CLIENT_FILE "me.info"
#define CLIENT_CONFIG "server.info"
#include <iomanip>
#include <iostream>
#include <vector>
#include <array>
#include <string>
#include <sstream>
#include <iomanip>
#include <cstdint>
#include <algorithm>

// Logging macros 
#define OK(msg)  std::cout << Color::GREEN  << "[+] " << msg << Color::RESET << std::endl
#define ERR(msg) std::cerr << Color::RED    << "[-] " << msg << Color::RESET << std::endl
#define INFO(msg) std::cout << Color::YELLOW << "[*] " << msg << Color::RESET << std::endl
#define TITLE(msg) std::cout << Color::CYAN << msg << Color::RESET << std::endl

// This lets us print to console in different colors
namespace Color {
    const std::string RESET = "\033[0m";
    const std::string RED = "\033[31m";
    const std::string GREEN = "\033[32m";
    const std::string YELLOW = "\033[33m";
    const std::string BLUE = "\033[34m";
    const std::string CYAN = "\033[36m";
};

namespace Protocol {
    const int version = 2;

    const int version_len = 1;
    const int code_len = 2;
    const int payload_size_len = 4;

    const int message_id_len = 4;
    const int message_type_len = 1;

    const int client_id_len = 16;
    const int max_username_len = 255;
    const int max_pubkey_len = 160;

    const int header_len_req = client_id_len + version_len + code_len + payload_size_len;
    const int header_len_resp = version_len + code_len + payload_size_len;
    const int symkey_length = 16; // in bytes
}

inline uint16_t read_le16(const uint8_t* p) {
    // Read a 16-bit little-endian value from a byte pointer
    // p[0] is 8 bits 0-7
    // p[1] is 8 bits - 8 to 15

    // if we shift p[1] by 8 bits we adjust the position and then or with p[0] we 
    // get p[1] ... p[0]
    return (uint16_t(p[1]) << 8) | uint16_t(p[0]);
}

inline uint32_t read_le32(const uint8_t* p) {
    // Read a 32-bit little-endian value from a byte pointer

    /*
        p[0] is bits o-7
        p[1] is 8-15
        and so on
    */

    // we need to shift each according to the position and then or them all together
    return (uint32_t(p[0])) |
           (uint32_t(p[1]) <<  8) |
           (uint32_t(p[2]) << 16) |
           (uint32_t(p[3]) << 24);
}

inline void put_le16(std::vector<uint8_t>& b, uint16_t v) {
    // Push a 16-bit value in little-endian order into the vector b
    // the lsb is v & 0xFF (we mask to get only the lowest 8 bits and ignore the rest), 
    // the msb is v >> 8 (we ignore the lower bits) 
    // we then place them lsb and then msb

    b.push_back(uint8_t(v & 0xFF));
    b.push_back(uint8_t(v >> 8));
}

inline void put_le32(std::vector<uint8_t>& b, uint32_t v) {
    // Push a 32-bit value in little-endian order into b
    /*
        v & 0xFF gives LSB, bits 0-7
        v >> 8 & 0xFF gives the next 8 bits - 8-15  
        v >> 16 & 0xFF gives bits 16-23
        v >> 24 & 0xFF gives bits 24-31
    */
    // then we just need to put them in this order

    b.push_back(uint8_t(v & 0xFF));
    b.push_back(uint8_t((v >> 8) & 0xFF));
    b.push_back(uint8_t((v >> 16) & 0xFF));
    b.push_back(uint8_t((v >> 24) & 0xFF));
}

/**
 * @brief converts a byte vector a hex string
 * 
 * @param data the vector
 * @return std::string 
 */
inline std::string to_hex(const std::vector<uint8_t>& data) {
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (uint8_t b : data)
        oss << std::setw(2) << static_cast<int>(b);
    return oss.str();
}

/**
 * @brief Another version of to_hex, but for arrays with fixed size
 * 
 * @tparam N the array size
 * @param data  the arr 
 * @return std::string 
 */
template <size_t N>
inline std::string to_hex(const std::array<uint8_t, N>& data) {
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (uint8_t b : data)
        oss << std::setw(2) << static_cast<int>(b);
    return oss.str();
}

/**
 * @brief Turns a hex string to an array of client id
 * 
 * @param hex the hex 
 * @param id the client id to fill
 * @return true if succeeded 
 * @return false if failed for any reason
 */
inline bool hex_to_id(const std::string& hex, std::array<uint8_t, Protocol::client_id_len>& id) {
    if (hex.size() != Protocol::client_id_len * 2)
        return false;

    for (size_t i = 0; i < Protocol::client_id_len; i++) {
        std::string byte_str = hex.substr(i * 2, 2);
        if (!std::all_of(byte_str.begin(), byte_str.end(), ::isxdigit))
            return false;

        id[i] = static_cast<uint8_t>(std::stoul(byte_str, nullptr, 16));
    }
    return true;
}

/**
 * @brief Checks if a vector is all zeroed
 * 
 * @param val the vector
 * @return true  if zeroed
 * @return false  otherwise
 */
inline bool zeroed(const std::vector<uint8_t>& val) {
    for (uint8_t b : val) {
        if (b != 0)
            return false;
    }
    return true;
}

/**
 * @brief Another version of zereoed, but for fixed size arrays
 * 
 * @tparam N arraysize 
 * @param val array
 * @return true  if zeroed
 * @return false  otherwise
 */
template <size_t N>
inline bool zeroed(const std::array<uint8_t, N>& val) {
    for (uint8_t b : val)
        if (b != 0) return false;
    return true;
}

#endif