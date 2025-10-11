#ifndef FRAMING_H
#define FRAMING_H

#include "protocol/constants.h"
#include <cstdint>
#include <vector>
#include <string>
#include <cstring>

/**
 * @brief A request frame is the main object send through a TCP connection to the server, after calling to_bytes(). 
 * To initialize a RequestFrame you need the user client_id, the clients version, operation code, and a payload 
 */
struct RequestFrame {
    std::array<uint8_t, Protocol::client_id_len> client_id;
    uint8_t version;
    uint16_t code;
    std::vector<uint8_t> payload;

    // A request frame is made out of: (ALL little endian)
    /*
        - 16 bytes client ID
        - 1 byte version
        - 2 bytes request code
        - 4 bytes payload size
        - payload
    */

    /**
     * @brief Serializes the RequestFrame to a byte buffer 
     * @return std::vector<uint8_t> - byte vector of the message in little-endian format.
     */
    std::vector<uint8_t> to_bytes() const {
        std::vector<uint8_t> buf; 
        buf.insert(buf.end(), client_id.begin(), client_id.end());
        buf.push_back(version);
        put_le16(buf, code);
        put_le32(buf, static_cast<uint32_t>(payload.size()));
        buf.insert(buf.end(), payload.begin(), payload.end());
        return buf;
    }

};

/**
 * @brief A response frame is the main object recieved from the server TCP connection 
 * To initialize a ResponseFrame you need the server version, response code and response payload. 
 */
struct ResponseFrame {
    uint8_t version; 
    uint16_t code;
    std::vector<uint8_t> payload;

    // A response frame is made out of (ALL little endian):
    /*
        - 1 byte version
        - 2 bytes response code
        - 4 bytes payload size
        - payload
    */

    /**
     * @brief Parses a byte stream into a ResponseFrame struct object
     * 
     * @param buf - the raw response buffer, should be at least Protocol::header_len_resp bytes long
     * @return ResponseFrame 
     * @throws std::runtime_error If the buffer is too small or theres any size mismatch
     */
    static ResponseFrame from_bytes(const std::vector<uint8_t> &buf) {
        ResponseFrame frame; 
        if (buf.size() < Protocol::header_len_resp) {
            throw std::runtime_error("Response buffer too small");
        }

        size_t offset = 0;
        frame.version = buf[offset++];
        frame.code = read_le16(&buf[offset]);
        offset += Protocol::code_len;

        uint32_t payload_size = read_le32(&buf[offset]);
        offset += Protocol::payload_size_len;

        if (buf.size() < offset + payload_size) {
            throw std::runtime_error("Response buffer too small for payload");
        }

        frame.payload.assign(buf.begin() + offset, buf.begin() + offset + payload_size);

        return frame;
    }
};

/**
 * @brief A message payload sent between clients
 * Includes the target client id, message type and content
 */
struct MessagePayload {
    std::array<uint8_t, Protocol::client_id_len> dest_id;
    uint8_t type;
    uint32_t content_size;
    std::vector<uint8_t> content;

    // A message payload is made out of (ALL little endian):
    /*
        - 16 bytes destination client ID
        - 1 byte message type
        - 4 bytes content size
        - content
    */

    /**
     * @brief Serializes the MessagePayload into a byte vector little endian
     * @return std::vector<uint8_t> - payload in the correct format 
     */
    std::vector<uint8_t> to_bytes() const {
        std::vector<uint8_t> buf;
        buf.insert(buf.end(), dest_id.begin(), dest_id.end());
        buf.push_back(type);
        put_le32(buf, content_size);
        buf.insert(buf.end(), content.begin(), content.end());
        return buf;
    }
};

#endif