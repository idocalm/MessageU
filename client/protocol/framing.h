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
};

#endif