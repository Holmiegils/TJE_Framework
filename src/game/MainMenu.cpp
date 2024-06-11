#include "MainMenu.h"
#include "framework/utils.h"
#include "graphics/texture.h"
#include "graphics/shader.h"
#include "graphics/mesh.h"
#include "framework/input.h"
#include <iostream>
#include <chrono>
#include "game.h"

MainMenu::MainMenu() : backgroundTexture(nullptr), shader(nullptr), active(true), selectedOption(0) {
    backgroundTexture = Texture::Get("data/textures/back.png");
    shader = Shader::Get("data/shaders/basic.vs", "data/shaders/texture.fs");

    if (!backgroundTexture) {
        std::cerr << "Failed to load background texture" << std::endl;
    }
    if (!shader) {
        std::cerr << "Failed to load shader" << std::endl;
    }

    menuItems.push_back({ "Start", START, Vector2(400, 300) });
    menuItems.push_back({ "Exit", EXIT, Vector2(400, 400) });

    lastKeyPressTime = std::chrono::high_resolution_clock::now();
}

MainMenu::~MainMenu() {}

void MainMenu::render() {
    if (!active) return;

    // Render background
    shader->enable();
    shader->setUniform("u_texture", backgroundTexture, 0);

    Matrix44 ortho_matrix;
    ortho_matrix.ortho(0, 800, 600, 0, -1, 1); // Set orthographic projection matrix

    Matrix44 identity_matrix;
    identity_matrix.setIdentity();
    shader->setUniform("u_model", identity_matrix);
    shader->setUniform("u_viewprojection", ortho_matrix);

    Mesh quad;
    quad.createQuad(400, 300, 800, 600, true); // Centered quad, size 800x600
    quad.render(GL_TRIANGLES);
    shader->disable();

    // Render menu items
    for (size_t i = 0; i < menuItems.size(); ++i) {
        Vector3 color = (i == selectedOption) ? Vector3(1, 1, 0) : Vector3(1, 1, 1);
        drawText(menuItems[i].position.x, menuItems[i].position.y, menuItems[i].text, color, 3);
    }

    // Render instructions
    drawText(200, 500, "Use arrow keys to navigate and Enter to select", Vector3(1, 1, 1), 2);
}

void MainMenu::update(double seconds_elapsed) {
    if (!active) return;

    // Debounce mechanism for arrow key sensitivity
    auto now = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastKeyPressTime);

    if (duration.count() > 200) { // 200 ms debounce time
        if (Input::isKeyPressed(SDL_SCANCODE_DOWN)) {
            selectedOption = (selectedOption + 1) % menuItems.size();
            lastKeyPressTime = now;
        }
        if (Input::isKeyPressed(SDL_SCANCODE_UP)) {
            selectedOption = (selectedOption - 1 + menuItems.size()) % menuItems.size();
            lastKeyPressTime = now;
        }
    }

    if (Input::wasKeyPressed(SDL_SCANCODE_RETURN)) {
        switch (menuItems[selectedOption].option) {
        case START:
            std::cout << "Start Game" << std::endl;
            active = false;
            Game::instance->setState(STATE_PLAYING); // Change game state to playing
            Game::instance->playAudio(); // Play background music
            break;
        case EXIT:
            std::cout << "Exit Game" << std::endl;
            exit(0); // Terminate the program
            break;
        }
    }
}

void MainMenu::handleInput(SDL_KeyboardEvent event) {
    // Handle input in the update method instead
}

// Fix for the blinking issue: modify the Game::render method to ensure it renders the main menu correctly

