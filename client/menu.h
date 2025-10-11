#ifndef MENU_H
#define MENU_H

#include <iostream>
#include <string>

#include "core/client.h"

class Menu {
    public:
        Menu(Client& client);
        void run();

    private:
        void print_menu();
        void handle_choice(MenuOptions choice);
        
        Client& client_;
};


#endif