#ifndef CODES_H
#define CODES_H
#include <cstdint>

// Codes are 2 bytes in the packet so we use uint16_t for RequestCode and ResponseCode which is 16 bits = 2 bytes

// RequestCode defines the protocol code for different types of requests a client can send. They will be added to the header. 
enum class RequestCode : uint16_t {
    REGISTER = 600, 
    LIST_CLIENTS = 601, 
    GET_PUBKEY = 602,
    SEND_MESSAGE = 603,
    PULL_MESSAGES = 604
};

// ResponseCode defines possible codes to arrive from server in a packet header.   
enum class ResponseCode : uint16_t {
    REGISTER_OK = 2100,
    LIST_CLIENTS = 2101,
    GET_PUBKEY = 2102,
    SEND_OK = 2103,
    PULL_MESSAGES = 2104,
    ERROR = 2500
};

// There are a few types of messages, first 2 are for symmetric key exchange between clients, 
// and we also have a normal end-to-end encrypted text or file (bonus!) 
// MessageType is 1 byte in the packet so we use uint8_t (8 bit = 1 byte)
enum class MessageType : uint8_t {
    SYM_REQ = 1,
    SYM_KEY = 2,
    TEXT = 3,
    FILE = 4
};

// These are for the menu.h and menu.cpp 
enum class MenuOptions: int {
    REGISTER = 110,
    LIST_CLIENTS = 120,
    REQ_PUB_KEY = 130, 
    PULL_MESSAGES = 140,
    SEND_TEXT_MESSAGE = 150,
    REQ_SYM_KEY = 151, 
    SEND_SYM_KEY = 152,
    EXIT = 0
};

#endif