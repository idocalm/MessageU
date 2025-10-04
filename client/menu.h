#ifndef MENU_H
#define MENU_H

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <functional>

#include "crypto/RSAWrapper.h"
#include "core/client.h"

using Handler = std::function<void()>;

class Menu {
    public:
        explicit Menu(Client& client);
        void run();

    private:
        void print_menu();
        void handle_choice(int choice);
        void build_handlers();

        Client& client_;
        std::unordered_map<int, Handler> handlers_;
};

namespace Color {
    inline const std::string RESET   = "\033[0m";
    inline const std::string RED     = "\033[31m";
    inline const std::string GREEN   = "\033[32m";
    inline const std::string YELLOW  = "\033[33m";
    inline const std::string BLUE    = "\033[34m";
    inline const std::string CYAN    = "\033[36m";
}



#endif