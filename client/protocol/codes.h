#ifndef CODES_H
#define CODES_H

#include <cstdint>

enum class RequestCode : uint16_t {
    REGISTER = 600, 
    LIST_CLIENTS = 601, 
    GET_PUBKEY = 602,
    SEND_MESSAGE = 603,
    PULL_MESSAGES = 604,
};

enum class ResponseCode : uint16_t {
    REGISTER_OK = 2100,
    LIST_CLIENTS = 2101,
    GET_PUBKEY = 2102,
    SEND_OK = 2103,
    PULL_MESSAGES = 2104,
    ERROR = 2500,
};

enum class MessageType : uint16_t {
    SYM_REQ = 1,
    SYM_KEY = 2,
    TEXT = 3,
    FILE = 4,
};

#endif