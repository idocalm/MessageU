#include "menu.h"

Menu::Menu(Client& client) : client_(client) {}

/**
 * @brief Prints the menu with the different options for the user to select from 
 * 
 */
void Menu::print_menu() {
    std::cout << Color::CYAN << "\nMessageU client at your service.\n" << Color::RESET;
    // The codes for each request are stored inside the MenuOptions enum in codes.h so we print it dynamically

    std::cout
            << static_cast<int>(MenuOptions::REGISTER)      << ") Register\n"
            << static_cast<int>(MenuOptions::LIST_CLIENTS)  << ") Request for clients list\n"
            << static_cast<int>(MenuOptions::REQ_PUB_KEY)   << ") Request for public key\n"
            << static_cast<int>(MenuOptions::PULL_MESSAGES) << ") Request for waiting messages\n"
            << static_cast<int>(MenuOptions::SEND_TEXT_MSG) << ") Send a text message\n"
            << static_cast<int>(MenuOptions::REQ_SYM_KEY)   << ") Send a request for symmetric key\n"
            << static_cast<int>(MenuOptions::SEND_SYM_KEY)  << ") Send your symmetric key\n"
            << static_cast<int>(MenuOptions::EXIT)          << ") Exit client\n"
            << std::endl;
}

/**
 * @brief Run the correct client function given the user selected choice, or report if its invalid
 * 
 * @param choice 
 */
void Menu::handle_choice(MenuOptions choice) {
    try {
        switch (choice) {
            case MenuOptions::REGISTER:
                client_.register_user(); 
                break;
            case MenuOptions::LIST_CLIENTS:
                client_.list_clients(); 
                break;
            case MenuOptions::REQ_PUB_KEY:
                client_.get_pubkey(); 
                break;
            case MenuOptions::PULL_MESSAGES:
                client_.pull_messages(); 
                break;
            case MenuOptions::SEND_TEXT_MESSAGE:
                client_.send_message_to_client(); 
                break;
            case MenuOptions::REQ_SYM_KEY:
                client_.request_sym_key(); 
                break;
            case MenuOptions::SEND_SYM_KEY:
                client_.send_sym_key(); 
                break;
            case MenuOptions::EXIT:
                break;
            default: 
                std::cout << Color::RED << "Invalid choice" << Color::RESET << std::endl;
                break;
        }
    } catch (std::exception& e) {
        std::cout << Color::RED << "Error: " << e.what() << Color::RESET << std::endl;
    }
}

/**
 * @brief Entry function for the Menu, 
 * initalizes the menu and repeatedly takes a choice from the user
 * This function is called from main.cpp. 
 */
void Menu::run() {
    int choice = -1;
    std::string line;

    do {
        print_menu();
        if (!std::getline(std::cin, line)) break;
        if (line.empty()) continue;


        choice = std::stoi(line);
        handle_choice(static_cast<MenuOptions>(choice));
        std::cout << std::endl;
        
    } while (choice != static_cast<int>(MenuOptions::EXIT));
}