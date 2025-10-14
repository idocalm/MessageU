#include "main.h"

/**
 * @brief Project main - creates a client instance and inits the menu,
 * or reports if unable to connect to server
 * 
 * @return int - 0 for success, 1 for failure
 */
int main() {
    try {
        Client client; 
        Menu menu(client);
        menu.run();
    } catch (const std::exception& e) {
        ERR("Error: " << e.what() << "; Please check " << CLIENT_CONFIG << " is correct and server is up");
        return 1; 
    }

    return 0;
}