#ifndef GAME_H
#define GAME_H

#pragma once

#include "framework/includes.h"
#include "framework/camera.h"
#include "framework/utils.h"
#include "graphics/mesh.h"
#include "graphics/texture.h"
#include "graphics/shader.h"
#include "framework/entities/entity.h"
#include "graphics/material.h"
#include "framework/input.h"
#include "MainMenu.h"

#include <SDL2/SDL.h>

class Game
{
public:
    static Game* instance;

    // window
    SDL_Window* window;
    int window_width;
    int window_height;

    // some globals
    long frame;
    float time;
    float elapsed_time;
    int fps;
    bool must_exit;

    // some vars
    Camera* camera; // our global camera
    Entity* root; // root entity for the scene
    MainMenu* mainMenu; // our main menu

    Game(int window_width, int window_height, SDL_Window* window);

    // main functions
    void render();
    void renderMainMenu();
    void renderDebugCollisions();
    void update(double dt);

    // input events
    void onKeyDown(SDL_KeyboardEvent event);
    void onKeyUp(SDL_KeyboardEvent event);
    void onMouseButtonDown(SDL_MouseButtonEvent event);
    void onMouseButtonUp(SDL_MouseButtonEvent event);
    void onMouseWheel(SDL_MouseWheelEvent event);
    void onGamepadButtonDown(SDL_JoyButtonEvent event);
    void onGamepadButtonUp(SDL_JoyButtonEvent event);
    void onResize(int width, int height);

private:
    // private variables
    bool mouse_locked;
    bool game_started; // New variable to track if the game has started
};

#endif // GAME_H