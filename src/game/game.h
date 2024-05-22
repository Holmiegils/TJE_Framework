#ifndef GAME_H
#define GAME_H

#include "framework/utils.h"
#include "graphics/mesh.h"
#include "graphics/texture.h"
#include "graphics/shader.h"
#include "framework/entities/entity.h"
#include "graphics/material.h"
#include "framework/input.h"
#include "framework/camera.h"  // Include Camera header

#include <SDL2/SDL.h>

class Game
{
public:
    static Game* instance;
    Shader* shader; // Declare shader here

    Game(int window_width, int window_height, SDL_Window* window);
    void render();
    void update(double seconds_elapsed);
    void onKeyDown(SDL_KeyboardEvent event);
    void onKeyUp(SDL_KeyboardEvent event);
    void onMouseButtonDown(SDL_MouseButtonEvent event);
    void onMouseButtonUp(SDL_MouseButtonEvent event);
    void onMouseWheel(SDL_MouseWheelEvent event);
    void onGamepadButtonDown(SDL_JoyButtonEvent event);
    void onGamepadButtonUp(SDL_JoyButtonEvent event);
    void onResize(int width, int height);

    bool must_exit; // Make public
    float time; // Make public
    float frame;
    float elapsed_time; // Make public
    float fps; // Make public
    int window_width; // Make public
    int window_height; // Make public
    SDL_Window* window; // Make public
    Camera* camera; // Ensure camera is declared here
    Entity* root; // Add root entity for the scene

private:
    bool mouse_locked;
};

#endif
