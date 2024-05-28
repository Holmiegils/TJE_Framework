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
    //bool mouse_locked; // tells if the mouse is locked (not seen)
    Entity* root; // root entity for the scene

    Game(int window_width, int window_height, SDL_Window* window);

    // main functions
    void render();
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
    };

#endif // GAME_H
