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
    Camera* camera;
    Entity* root;
    MainMenu* mainMenu;

    int flask_uses = 3;
    float current_health = 10.0f;
    const float max_health = 100.0f;
    const float heal_amount = 10.0f;

    void playAudio();

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

    void renderQuad(Texture* texture, Vector2 position, Vector2 size, float scale);
    void loadAudio();
    void renderHUD();

private:
    bool mouse_locked;
    bool game_started;
};

#endif // GAME_H
