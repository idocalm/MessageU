#include <cstdint>
#include <vector>
#include <string>
#include <cstring>

struct RequestFrame {
    std::array<uint8_t, 16> client_id;
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
        buf.resize(16 + 1 + 2 + 4 + payload.size());
        size_t offset = 0;
        memcpy(buf.data() + offset, client_id.data(), 16);
        offset += 16;
        buf[offset++] = version;
        *reinterpret_cast<uint16_t*>(buf.data() + offset) = code; 
        offset += 2;
        *reinterpret_cast<uint32_t*>(buf.data() + offset) = payload.size();
        offset += 4;

        if (!payload.empty()) {
            memcpy(buf.data() + offset, payload.data(), payload.size());
        }

        return buf;
    }

};

struct ResponseFrame {
    uint8_t version; 
    uint16_t code;
    std::vector<uint8_t> payload;

    static ResponseFrame from_bytes(const std::vector<uint8_t> &buf) {
        ResponseFrame frame; 
        if (buf.size() < 1 + 2 + 4) {
            throw std::runtime_error("Response buffer too small");
        }

        size_t offset = 0;
        frame.version = buf[offset++];
        
        frame.code = *reinterpret_cast<const uint16_t*>(buf.data() + offset);
        offset += 2;

        uint32_t payload_size = *reinterpret_cast<const uint32_t*>(buf.data() + offset);
        offset += 4;

        if (buf.size() < offset + payload_size) {
            throw std::runtime_error("Response buffer too small for payload");
        }

        frame.payload.assign(buf.begin() + offset, buf.begin() + offset + payload_size);

        return frame;
    }
};

