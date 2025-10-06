#ifndef FRAMING_H
#define FRAMING_H

#include "protocol/constants.h"
#include <cstdint>
#include <vector>
#include <string>
#include <cstring>

struct RequestFrame {
    std::array<uint8_t, Protocol::client_id_len> client_id;
    uint8_t version;
    uint16_t code;
    std::vector<uint8_t> payload;

    // A request frame is made out of:
    /*
        - 16 bytes client ID
        - 1 byte version
        - 2 bytes request code
        - 4 bytes payload size
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

struct ResponseFrame {
    uint8_t version; 
    uint16_t code;
    std::vector<uint8_t> payload;

    static ResponseFrame from_bytes(const std::vector<uint8_t> &buf) {
        ResponseFrame frame; 
        if (buf.size() < Protocol::header_len_resp) {
            throw std::runtime_error("Response buffer too small");
        }

        size_t offset = 0;
        frame.version = buf[offset++];
        frame.code = read_le16(&buf[offset]);
        offset += Protocol::code_len;

        uint32_t payload_size =  read_le32(&buf[offset]);
        offset += Protocol::payload_size_len;

        if (buf.size() < offset + payload_size) {
            throw std::runtime_error("Response buffer too small for payload");
        }

        frame.payload.assign(buf.begin() + offset, buf.begin() + offset + payload_size);

        return frame;
    }

    void debug() const {
        std::cout << "=== ResponseFrame Debug ===\n";
        std::cout << "Version: " << static_cast<int>(version) << "\n";
        std::cout << "Code: 0x" << std::hex << std::setw(4) << std::setfill('0') << code << std::dec << "\n";
        std::cout << "Payload size: " << payload.size() << " bytes\n";

        std::cout << "Payload (hex): ";
        for (uint8_t b : payload)
            std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(b);
        std::cout << std::dec << "\n";
        std::cout << "===========================\n";
    }
};

struct MessagePayload {
    std::array<uint8_t, Protocol::client_id_len> dest_id;
    uint8_t type;
    uint32_t content_size;
    std::vector<uint8_t> content;

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