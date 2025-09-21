#include "core/client.h"
#include <iostream>

int main() {
    try {
        Client client("127.0.0.1", 1357);

        std::vector<uint8_t> pubkey(160, 'A'); // Dummy public key
        client.register_user("testuser", pubkey);

        client.list_clients();
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}