#include "menu.h"

Menu::Menu(Client& client) : client_(client) {
    build_handlers();
}

void Menu::print_menu() {
    std::cout << Color::CYAN << "\nMessageU client at your service.\n" << Color::RESET;
    std::cout << "110) Register\n"
              << "120) Request for clients list\n"
              << "130) Request for public key\n"
              << "140) Request for waiting messages\n"
              << "150) Send a text message\n"
              << "151) Send a request for symmetric key\n"
              << "152) Send your symmetric key\n"
              << "0) Exit client\n" << std::endl;
}

void Menu::build_handlers() {
    handlers_ = {
        {110, [this] {
            client_.register_user();
        }},
        {120, [this] { client_.list_clients(); }},
        {130, [this] { client_.get_pubkey(); }},
        {140, [this] { client_.pull_messages(); }},
        {150, [this] { client_.send_mesage_to_client(); }},
        {151, [this] { client_.request_sym_key(); }},
        {152, [this] { client_.send_sym_key(); }},
        {0,   [this] {
            std::cout << Color::YELLOW << "Goodbye!" << Color::RESET << std::endl;
        }},
    };
}

void Menu::handle_choice(int choice) {
    const std::unordered_set<int> allowed = {0,110,120,130,140,150,151,152};
    if (!allowed.count(choice)) {
        std::cout << Color::RED << "Invalid choice" << Color::RESET << std::endl;
        return;
    }

    try {
        if (auto it = handlers_.find(choice); it != handlers_.end()) {
            it->second();
        } else {
            std::cout << Color::RED << "Invalid choice" << Color::RESET << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << Color::RED << "Error: " << e.what() << Color::RESET << std::endl;
    }
}


void Menu::run() {
    int choice = -1;
    do {
        print_menu();
        std::string line;
        if (!std::getline(std::cin, line)) break;
        if (line.empty()) continue;
        choice = std::stoi(line);
        handle_choice(choice);
        std::cout << std::endl;
    } while (choice != 0);
}