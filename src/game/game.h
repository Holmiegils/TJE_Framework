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

enum GameState {
    STATE_MAIN_MENU,
    STATE_PLAYING,
    STATE_DEATH,
    STATE_VICTORY
};

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
    bool death_sound_played;
    bool hulda_death_sound_played;

    // some vars
    Camera* camera;
    Entity* root;
    MainMenu* mainMenu;

    int flask_uses = 3;
    const float heal_amount = 40.0f;

    float current_health = 100.0f;
    const float max_health = 100.0f;

    // Stamina variables
    float current_stamina = 100.0f;
    const float max_stamina = 100.0f;
    const float stamina_depletion_rate = 20.0f; // Stamina depletion rate per second
    const float stamina_regen_rate = 10.0f; // Stamina regeneration rate per second

    void setStamina(float new_stamina);
    float getStamina() const;

    void playAudio();

    Game(int window_width, int window_height, SDL_Window* window);

    void initialize();

    void renderGameScene();

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
    void renderDeathOverlay();
    void renderVictoryOverlay();
    void renderHUD();

    void setState(GameState newState) {
        currentState = newState;
    }

private:
    bool mouse_locked;
    bool game_started;
    GameState currentState;
};

#endif // GAME_H
