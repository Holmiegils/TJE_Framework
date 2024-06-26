#ifndef MAINMENU_H
#define MAINMENU_H

#include "framework/input.h"
#include "graphics/texture.h"
#include "graphics/shader.h"
#include "framework/utils.h" // For drawText function
#include <vector>
#include <string>
#include <chrono>

class MainMenu {
public:
    enum MenuOption {
        START,
        EXIT,
        RESTART
    };


    struct MenuItem {
        std::string text;
        MenuOption option;
        Vector2 position;
    };

    MainMenu();
    ~MainMenu();

    void initialize();  // Add this line
    void render();
    void update(double seconds_elapsed);
    void handleInput(SDL_KeyboardEvent event);

    void change_menuItems();

    bool isActive() const { return active; }
    void setActive(bool value) { active = value; }

private:
    std::vector<MenuItem> menuItems;
    bool gameStarted;
    Shader* shader;
    bool active;
    int selectedOption;
    std::chrono::high_resolution_clock::time_point lastKeyPressTime;
};

#endif // MAINMENU_H
