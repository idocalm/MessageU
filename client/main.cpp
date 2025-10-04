#include "core/client.h"
#include <iostream>
#include "menu.h"

int main() {
    try {
        Client client; 
        Menu menu(client);
        menu.run();
    } catch (const std::exception& e) {
        std::cerr << Color::RED << "Error: " << e.what() << "; Please check " << CLIENT_CONFIG << " is correct and server is up" << Color::RESET << std::endl;
    }

    return 0;
}