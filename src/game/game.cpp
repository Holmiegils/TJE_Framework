#include "game.h"
#include "framework/utils.h"
#include "graphics/mesh.h"
#include "graphics/texture.h"
#include "graphics/fbo.h"
#include "graphics/shader.h"
#include "framework/input.h"
#include "framework/entities/entity.h"
#include "graphics/material.h"
#include "graphics/EntityMesh.h"
#include "EntityCollider.h"
#include "framework/bass.h"
#include <framework/animation.h>
#include <cmath>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include "character.h"
#include "hulda.h"

Shader* shader = NULL;

float angle = 0;
float mouse_speed = 100.0f;

// Load HUD textures
Texture* health_empty = NULL;
Texture* health_full = NULL;
Texture* stamina_empty = NULL;
Texture* stamina_full = NULL;
Texture* heal_button_0 = NULL;
Texture* heal_button_1 = NULL;
Texture* heal_button_2 = NULL;
Texture* heal_button_3 = NULL;
Texture* attack_tut = NULL;
Texture* heal_tut = NULL;
Texture* movement_tut = NULL;
Texture* death_screen_texture = NULL;
Texture* victory_screen_texture = NULL;

Game* Game::instance = NULL;

float camera_pitch = -0.5;
float camera_yaw = -PI/2;

float sphere_collision_radius = 4.0f;

float tutorial_time = 0.0f;

Character* character = new Character();
Hulda* hulda = new Hulda();

// Declaration of meshes_to_load
std::unordered_map<std::string, std::vector<Matrix44>> meshes_to_load;


GameState currentState = STATE_MAIN_MENU;



bool parseScene(const char* filename, Entity* root) {
    std::cout << " + Scene loading: " << filename << "..." << std::endl;
    std::ifstream file(filename);

    if (!file.good()) {
        std::cerr << "Scene [ERROR] File not found!" << std::endl;
        return false;
    }

    std::string scene_info, mesh_name, model_data;
    file >> scene_info >> scene_info;
    int mesh_count = 0;

    while (file >> mesh_name >> model_data) {
        if (mesh_name[0] == '#') continue;
        std::vector<std::string> tokens = tokenize(model_data, ",");
        Matrix44 model;
        for (int t = 0; t < tokens.size(); ++t) {
            model.m[t] = (float)atof(tokens[t].c_str());
        }
        meshes_to_load[mesh_name].push_back(model);
        mesh_count++;
    }

    for (const auto& data : meshes_to_load) {
        mesh_name = "data/" + data.first;
        const std::vector<Matrix44>& models = data.second;

        if (models.empty()) continue;

        Material mat;
        EntityCollider* new_entity = nullptr;

        Mesh* mesh = Mesh::Get(mesh_name.c_str());
        if (!mesh) {
            std::cerr << "Mesh loading failed: " << mesh_name << std::endl;
            continue;
        }
        new_entity = new EntityCollider(mesh, mat);

        if (!new_entity) {
            std::cerr << "Entity creation failed: " << data.first << std::endl;
            continue;
        }

        new_entity->name = data.first;
        new_entity->layer = WALL; // Assign appropriate layer here
        if (models.size() > 1) {
            new_entity->isInstanced = true;
            new_entity->models = models;
        }
        else {
            new_entity->model = models[0];
        }

        root->addChild(new_entity);
    }

    std::cout << "Scene [OK] Meshes added: " << mesh_count << std::endl;
    return true;
}

std::vector<std::string> tokenize(const std::string& str, const std::string& delimiters)
{
    std::vector<std::string> tokens;
    size_t prev = 0, pos = 0;
    do {
        pos = str.find_first_of(delimiters, prev);
        if (pos == std::string::npos) pos = str.length();
        std::string token = str.substr(prev, pos - prev);
        if (!token.empty()) tokens.push_back(token);
        prev = pos + 1;
    } while (pos < str.length() && prev < str.length());
    return tokens;
}

void Game::renderQuad(Texture* texture, Vector2 position, Vector2 size, float scale)
{
    Mesh quad;
    quad.vertices.push_back(Vector3(-0.5, 0.5, 0));
    quad.uvs.push_back(Vector2(0, 1));
    quad.vertices.push_back(Vector3(-0.5, -0.5, 0));
    quad.uvs.push_back(Vector2(0, 0));
    quad.vertices.push_back(Vector3(0.5, -0.5, 0));
    quad.uvs.push_back(Vector2(1, 0));
    quad.vertices.push_back(Vector3(-0.5, 0.5, 0));
    quad.uvs.push_back(Vector2(0, 1));
    quad.vertices.push_back(Vector3(0.5, -0.5, 0));
    quad.uvs.push_back(Vector2(1, 0));
    quad.vertices.push_back(Vector3(0.5, 0.5, 0));
    quad.uvs.push_back(Vector2(1, 1));

    Matrix44 model;
    model.setTranslation(position.x, position.y, 0);
    model.scale(size.x * scale, size.y, 1);

    Shader* shader = Shader::Get("data/shaders/basic.vs", "data/shaders/texture.fs");
    shader->enable();
    shader->setUniform("u_texture", texture, 0);
    shader->setUniform("u_model", model);
    shader->setUniform("u_viewprojection", Matrix44::IDENTITY);

    quad.render(GL_TRIANGLES);
    shader->disable();
}


HCHANNEL hSampleChannel; 

HCHANNEL hPunchChannel;

HCHANNEL hHealChannel;

HCHANNEL hWalkChannel;

HCHANNEL hDamageChannel;

HCHANNEL hDeathChannel;

HCHANNEL hVictroyChannel;

void Game::loadAudio() {
    // Load background music stream from disk
    hSampleChannel = BASS_StreamCreateFile(false, "data/audio/background_music.mp3", 0, 0, 0);
    if (hSampleChannel == 0) {
        std::cerr << "Error loading audio stream: background_music.mp3" << std::endl;
        return;
    }
    BASS_ChannelSetAttribute(hSampleChannel, BASS_ATTRIB_VOL, 0.8); // Set background music volume

    // Load punch stream from disk
    hPunchChannel = BASS_StreamCreateFile(false, "data/audio/punch.wav", 0, 0, 0);
    if (hPunchChannel == 0) {
        std::cerr << "Error loading audio stream: punch.wav" << std::endl;
        return;
    }
    BASS_ChannelSetAttribute(hPunchChannel, BASS_ATTRIB_VOL, 0.5); // Set punch sound volume

    // Load heal stream from disk
    hHealChannel = BASS_StreamCreateFile(false, "data/audio/heal.wav", 0, 0, 0);
    if (hHealChannel == 0) {
        std::cerr << "Error loading audio stream: heal.wav" << std::endl;
        return;
    }
    BASS_ChannelSetAttribute(hHealChannel, BASS_ATTRIB_VOL, 1.8); // Set heal sound volume

    // Load walk stream from disk
    hWalkChannel = BASS_StreamCreateFile(false, "data/audio/walk.wav", 0, 0, BASS_SAMPLE_LOOP);
    if (hWalkChannel == 0) {
        std::cerr << "Error loading audio stream: walk.wav" << std::endl;
        return;
    }
    BASS_ChannelSetAttribute(hWalkChannel, BASS_ATTRIB_VOL, 0.2); // Set walk sound volume

    hDamageChannel = BASS_StreamCreateFile(false, "data/audio/damage.wav", 0, 0, 0);
    if (hDamageChannel == 0) {
        std::cerr << "Error loading audio stream: char_death.wav" << std::endl;
        return;
    }
    BASS_ChannelSetAttribute(hDamageChannel, BASS_ATTRIB_VOL, 0.5);


    hDeathChannel = BASS_StreamCreateFile(false, "data/audio/char_death.wav", 0, 0, 0);
    if (hDeathChannel == 0) {
        std::cerr << "Error loading audio stream: char_death.wav" << std::endl;
        return;
    }
    BASS_ChannelSetAttribute(hDeathChannel, BASS_ATTRIB_VOL, 0.5);

    hVictroyChannel = BASS_StreamCreateFile(false, "data/audio/hulda_death.wav", 0, 0, 0);
    if (hVictroyChannel == 0) {
        std::cerr << "Error loading audio stream: char_death.wav" << std::endl;
        return;
    }
    BASS_ChannelSetAttribute(hVictroyChannel, BASS_ATTRIB_VOL, 1.3);
}

void Game::playAudio() {
    // Play channel
    BASS_ChannelPlay(hSampleChannel, true);
}

Game::Game(int window_width, int window_height, SDL_Window* window)
    : window(window), window_width(window_width), window_height(window_height),
    frame(0), time(0.0f), elapsed_time(0.0f), fps(0), must_exit(false),
    mouse_locked(false), game_started(false), death_sound_played(false), hulda_death_sound_played(false)
{
    this->window_width = window_width;
    this->window_height = window_height;
    this->window = window;
    instance = this;
    must_exit = false;

    fps = 0;
    frame = 0;
    time = 0.0f;
    elapsed_time = 0.0f;
    mouse_locked = false;

    // Initialize BASS
    if (BASS_Init(-1, 44100, 0, 0, NULL) == false) {
        std::cerr << "Error initializing BASS" << std::endl;
    }

    // Load audio
    loadAudio();

    // Load HUD textures
    health_empty = Texture::Get("data/textures/health_empty.png");
    health_full = Texture::Get("data/textures/health_full.png");
    stamina_empty = Texture::Get("data/textures/stamina_empty.png");
    stamina_full = Texture::Get("data/textures/stamina_full.png");
    heal_button_0 = Texture::Get("data/textures/heal_button_0.png");
    heal_button_1 = Texture::Get("data/textures/heal_button_1.png");
    heal_button_2 = Texture::Get("data/textures/heal_button_2.png");
    heal_button_3 = Texture::Get("data/textures/heal_button_3.png");

    attack_tut = Texture::Get("data/textures/attack_tut.png");
    heal_tut = Texture::Get("data/textures/heal_tut.png");
    movement_tut = Texture::Get("data/textures/movement_tut.png");

    death_screen_texture = Texture::Get("data/textures/you_died.png");
    victory_screen_texture = Texture::Get("data/textures/slain.png");

    // OpenGL flags
    glEnable(GL_CULL_FACE); // render both sides of every triangle
    glEnable(GL_DEPTH_TEST); // check the occlusions using the Z buffer

    // Create our camera
    camera = new Camera();
    camera->lookAt(Vector3(0.f, 1000.f, 1000.f), Vector3(0.f, 0.f, 0.f), Vector3(0.f, 1.f, 0.f)); // position the camera and point to 0,0,0
    camera->setPerspective(70.f, window_width / (float)window_height, 0.1f, 10000.f); // set the projection, we want to be perspective

    root = new Entity();
    parseScene("data/myscene.scene", root);

    shader = Shader::Get("data/shaders/skinning.vs", "data/shaders/texture.fs");

    character->initialize();
    hulda->initialize();

    // Initialize the main menu
    mainMenu = new MainMenu();
    mainMenu->setActive(true); // Set the main menu as active

    // Set the initial game state to main menu
    currentState = STATE_MAIN_MENU;
}

// what to do when the image has to be drawn
// Include this function to render the game scene
void Game::renderGameScene() {
    // Set the camera as default
    camera->enable();

    // Set flags
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    character->render(camera);
    
    shader->enable();
    root->render(camera);
    shader->disable();

    // Render Hulda before the HUD
    hulda->render(camera);

    // Render the FPS, Draw Calls, etc.
    drawText(2, 2, getGPUStats(), Vector3(1, 1, 1), 2);
}

// Main render function
void Game::render() {
    // Set the clear color (the background color)
    glClearColor(0.0, 0.0, 0.0, 1.0);

    // Clear the window and the depth buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Render based on the current state
    switch (currentState) {
    case STATE_MAIN_MENU:
        renderMainMenu();
        break;
    case STATE_PLAYING:
        renderGameScene();
        renderHUD();
        break;
    case STATE_DEATH:
        renderGameScene(); // Render the scene behind the overlay
        renderDeathOverlay();
        break;
    case STATE_VICTORY:
        renderGameScene(); // Render the scene behind the overlay
        renderVictoryOverlay();
        break;
    default:
        break;
    }

    // Swap between front buffer and back buffer
    SDL_GL_SwapWindow(this->window);
}

// Main menu rendering should only clear and swap buffers once per frame
void Game::renderMainMenu() {
    death_sound_played = false;
    hulda_death_sound_played = false;
    mainMenu->render();

}

// Add functions to render the death and victory overlays
void Game::renderDeathOverlay() {
    // Define position and size for the death overlay
    Vector2 screen_position = Vector2(0.0f, 0.0f); // Centered
    Vector2 screen_size = Vector2(0.8f, 0.4f); // Overlay size in NDC (-1 to 1)

    // Render the death overlay
    renderQuad(death_screen_texture, screen_position, screen_size, 1.0f);
}

void Game::renderVictoryOverlay() {
    // Define position and size for the victory overlay
    Vector2 screen_position = Vector2(0.0f, 0.0f); // Centered
    Vector2 screen_size = Vector2(1.5f, 0.4f); // Overlay size in NDC (-1 to 1)

    // Render the victory overlay
    renderQuad(victory_screen_texture, screen_position, screen_size, 1.0f);
}

void Game::renderHUD() {
    // Disable depth test and enable blending for HUD rendering
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Define positions and sizes for health and stamina bars
    Vector2 health_position = Vector2(-0.6f, 0.9f);
    Vector2 health_size = Vector2(0.6f, 0.1f); // Adjusted size for better visibility
    Vector2 stamina_position = Vector2(-0.6f, 0.75f);
    Vector2 stamina_size = Vector2(0.6f, 0.1f); // Adjusted size for better visibility

    // Render empty bars
    renderQuad(health_empty, health_position, health_size, 1.0f);
    renderQuad(stamina_empty, stamina_position, stamina_size, 1.0f);

    // Assume some variables health and stamina are between 0 and 1
    float health = current_health / max_health;
    float stamina = current_stamina / max_stamina; // Use current_stamina

    // Render full bars with appropriate scaling
    renderQuad(health_full, Vector2(health_position.x - (1.0f - health) * health_size.x * 0.5f, health_position.y), health_size, health);
    renderQuad(stamina_full, Vector2(stamina_position.x - (1.0f - stamina) * stamina_size.x * 0.5f, stamina_position.y), stamina_size, stamina);

    // Define position and size for the heal button
    Vector2 heal_button_position = Vector2(0.8f, -0.8f); // Bottom right corner
    Vector2 heal_button_size = Vector2(0.2f, 0.2f); // Adjust size as needed

    // Choose the appropriate heal button texture based on flask_uses
    Texture* heal_button = heal_button_0;
    if (flask_uses == 1) {
        heal_button = heal_button_1;
    }
    else if (flask_uses == 2) {
        heal_button = heal_button_2;
    }
    else if (flask_uses == 3) {
        heal_button = heal_button_3;
    }

    // Render the heal button
    renderQuad(heal_button, heal_button_position, heal_button_size, 1.0f);

    // Define position and size for the Hulda's health bar
    Vector2 boss_health_position = Vector2(0.0f, -0.9f); // Centered at the bottom of the screen
    Vector2 boss_health_size = Vector2(1.2f, 0.15f); // Larger size for the boss health bar

    // Calculate Hulda's health percentage
    float boss_health = hulda->getHealth() / 100.0f; // Assuming Hulda's max health is 100

    // Render empty boss health bar
    renderQuad(health_empty, boss_health_position, boss_health_size, 1.0f);

    // Render full boss health bar with appropriate scaling
    renderQuad(health_full, Vector2(boss_health_position.x - (1.0f - boss_health) * boss_health_size.x * 0.5f, boss_health_position.y), boss_health_size, boss_health);

    // Define position and size for the Hulda's image
    Vector2 hulda_image_position = Vector2(0.0f, -0.72f); // Slightly above the boss health bar
    Vector2 hulda_image_size = Vector2(0.3f, 0.2f); // Size of the image

    // Load and render the Hulda image
    Texture* hulda_image = Texture::Get("data/textures/hulda_name.png");
    renderQuad(hulda_image, hulda_image_position, hulda_image_size, 1.0f);

    if (tutorial_time <= 10.0f) {
        // Define positions and sizes for tutorial images
        Vector2 movement_tut_size = Vector2(0.9f, 0.7f); // Adjusted size for better visibility
        Vector2 heal_tut_size = Vector2(0.3f, 0.3f); // Adjusted size for better visibility
        Vector2 attack_tut_size = Vector2(0.3f, 0.3f); // Adjusted size for better visibility

        Vector2 attack_tut_position = Vector2(-0.4f, -0.3f);
        Vector2 heal_tut_position = Vector2(0.4f, -0.3f);
        Vector2 movement_tut_position = Vector2(0.0f, 0.3f);

        // Render tutorial images
        renderQuad(attack_tut, attack_tut_position, attack_tut_size, 1.0f);
        renderQuad(heal_tut, heal_tut_position, heal_tut_size, 1.0f);
        renderQuad(movement_tut, movement_tut_position, movement_tut_size, 1.0f);

        // Define positions for the text below the images
        Vector2 attack_text_position = Vector2(attack_tut_position.x + 0.12f, attack_tut_position.y + 0.55f);
        Vector2 heal_text_position = Vector2(heal_tut_position.x - 0.27f, heal_tut_position.y + 0.55f);
        Vector2 movement_text_position = Vector2(movement_tut_position.x, movement_tut_position.y - 0.27f);
        Vector2 shift_text_position = Vector2(attack_tut_position.x + 0.12f, movement_tut_position.y - 0.27f);

        // Render text below tutorial images
        drawText((attack_text_position.x + 0.5f) * window_width, (attack_text_position.y + 0.5f) * window_height, "Attack with Click", Vector3(1, 1, 1), 2);
        drawText((heal_text_position.x + 0.5f) * window_width, (heal_text_position.y + 0.5f) * window_height, "Heal with E", Vector3(1, 1, 1), 2);
        drawText((movement_text_position.x + 0.5f) * window_width, (movement_text_position.y + 0.5f) * window_height, "Move with WASD", Vector3(1, 1, 1), 2);
        drawText((shift_text_position.x + 0.5f) * window_width, (shift_text_position.y + 0.5f) * window_height, "Shift to sprint", Vector3(1, 1, 1), 2);
    }

    glDisable(GL_BLEND);
}

bool is_walking_sound_playing = false;
float walk_speed = 0.5f;

void Game::update(double seconds_elapsed) {
    switch (currentState) {
    case STATE_MAIN_MENU: {
        playAudio();
        mainMenu->update(seconds_elapsed);
        if (!mainMenu->isActive()) {
            currentState = STATE_PLAYING;
            game_started = true;
        }
        break;
    }

    case STATE_PLAYING: {
        if (current_health <= 0) {
            currentState = STATE_DEATH;
            death_sound_played = false;
            break;
        }

        if (hulda->getHealth() <= 0) {
            currentState = STATE_VICTORY;
            hulda_death_sound_played = false;
            break;
        }

        if (game_started && !mouse_locked) {
            mouse_locked = true;
            SDL_ShowCursor(SDL_DISABLE);
            SDL_SetRelativeMouseMode(SDL_TRUE);
        }

        tutorial_time += seconds_elapsed;

        if (camera_pitch + Input::mouse_delta.y * seconds_elapsed < -0.01 && camera_pitch + Input::mouse_delta.y * seconds_elapsed > -1) {
            camera_pitch += Input::mouse_delta.y * seconds_elapsed;
        }

        camera_yaw -= Input::mouse_delta.x * seconds_elapsed;

        Matrix44 pitchmat;
        pitchmat.setRotation(camera_pitch, Vector3(1, 0, 0));

        Matrix44 yawmat;
        yawmat.setRotation(camera_yaw, Vector3(0, 1, 0));

        Matrix44 unified;
        unified = pitchmat * yawmat;
        Vector3 front = unified.frontVector();

        Vector3 viewpoint = character->getPosition();
        viewpoint.y += 10;

        camera->lookAt(character->getPosition() - front * 35, viewpoint, Vector3(0, 1, 0));

        if (hulda->heavyHit() && !character->isImmune()) {
            current_health -= 30;
            character->takeDamage(hulda->getPosition());
            BASS_ChannelPlay(hDamageChannel, true);
        }

        character->update(seconds_elapsed, front, camera_yaw, hulda->getPosition());

        //std::cout << "stamina: " << current_stamina << "," << "speed:" << character->getSpeed() << std::endl;

        // Handle stamina depletion and regeneration
        if (character->getSpeed() == 50.0f && current_stamina > 0) {
            current_stamina -= stamina_depletion_rate * seconds_elapsed;
            if (current_stamina < 0) {
                current_stamina = 0;
            }
        }
        else {
            current_stamina += stamina_regen_rate * seconds_elapsed;
            if (current_stamina > max_stamina) {
                current_stamina = max_stamina;
            }
        }

        // Ensure character speed is reset when stamina is zero
        if (character->getSpeed() == 50.0f && current_stamina <= 0.3f) {
            character->setSpeed(25.0f);  // Reset speed to normal walking speed
        }

        if (character->isRunning()) {
            if (!is_walking_sound_playing) {
                BASS_ChannelPlay(hWalkChannel, true);
                BASS_ChannelSetAttribute(hWalkChannel, BASS_ATTRIB_FREQ, 44100 * 0.5f);
                is_walking_sound_playing = true;
            }
        }
        else {
            BASS_ChannelStop(hWalkChannel);
            is_walking_sound_playing = false;
        }

        if (character->isPunching()) {
            BASS_ChannelPlay(hPunchChannel, true);
            BASS_ChannelStop(hWalkChannel);
        }

        if (character->huldaIsHit() && !hulda->isImmune()) {
            hulda->takeDamage(20.0f);
            hulda->setImmunity();
        }

        hulda->update(seconds_elapsed, character->getPosition());

        break;
    }

    case STATE_DEATH: {
        if (!death_sound_played) {
            BASS_ChannelStop(hSampleChannel);
            BASS_ChannelPlay(hDeathChannel, true);
            death_sound_played = true;
        }
        break;
    }

    case STATE_VICTORY: {
        if (!hulda_death_sound_played) {
            BASS_ChannelStop(hSampleChannel);
            BASS_ChannelPlay(hVictroyChannel, true);
            hulda_death_sound_played = true;
        }
        break;
    }

    default: {
        break;
    }
    }
}

void Game::onKeyUp(SDL_KeyboardEvent event)
{
    if (!game_started) {
        mainMenu->handleInput(event);
        return;
    }

    switch (event.keysym.sym) {
    case SDL_SCANCODE_W:
    case SDL_SCANCODE_S:
    case SDL_SCANCODE_A:
    case SDL_SCANCODE_D:
        // Stop walk sound when movement key is released
        BASS_ChannelStop(hWalkChannel);
        is_walking_sound_playing = false;
        break;
    }
}




// Keyboard event handler (sync input)
void Game::onKeyDown(SDL_KeyboardEvent event) {
    if (currentState == STATE_MAIN_MENU) {
        mainMenu->handleInput(event);
        return;
    }

    switch (event.keysym.sym) {
    case SDLK_ESCAPE:
        currentState = STATE_MAIN_MENU;
        mainMenu->setActive(true); // Reactivate the main menu
        break;
    case SDLK_F1:
        Shader::ReloadAll();
        break;
    case SDLK_e:
        if (flask_uses > 0) {
            if (current_health < max_health) {
                current_health = std::min(max_health, current_health + heal_amount);
                flask_uses--;
                BASS_ChannelPlay(hHealChannel, true);
            }
        }
        break;
    }
}





void Game::onMouseButtonDown(SDL_MouseButtonEvent event)
{

}

void Game::onMouseButtonUp(SDL_MouseButtonEvent event)
{
    // Implementation here
}

void Game::onMouseWheel(SDL_MouseWheelEvent event)
{
    mouse_speed *= event.y > 0 ? 1.1f : 0.9f;
}

void Game::onGamepadButtonDown(SDL_JoyButtonEvent event)
{
    // Implementation here
}

void Game::onGamepadButtonUp(SDL_JoyButtonEvent event)
{
    // Implementation here
}

void Game::onResize(int width, int height)
{
    std::cout << "window resized: " << width << "," << height << std::endl;
    glViewport(0, 0, width, height);
    camera->aspect = width / (float)height;
    window_width = width;
    window_height = height;
}

void Game::setStamina(float new_stamina) {
    current_stamina += new_stamina;
}

float Game::getStamina() const {
    return current_stamina;
}

// ALEX: RENDER COLLISION SPHERES AS DEBUG TO CHECK IF WE ARE DOING STUFF CORRECTLY!
//void Game::renderDebugCollisions()
//{
//    Shader* shader = Shader::Get("data/shaders/basic.vs", "data/shaders/flat.fs");
//    Mesh* mesh = Mesh::Get("data/meshes/sphere.obj");
//
//    shader->enable();
//
//    {
//        Matrix44 m;
//        m.setTranslation(mesh_matrix.getTranslation());
//        m.translate(0.0f, character_height, 0.0f);
//        m.scale(sphere_collision_radius, sphere_collision_radius, sphere_collision_radius);
//
//        shader->setUniform("u_viewprojection", camera->viewprojection_matrix);
//        shader->setUniform("u_color", Vector4(0.0f, 1.0f, 0.0f, 1.0f));
//        shader->setUniform("u_model", m);
//
//        mesh->render(GL_LINES);
//    }
//
//    shader->disable();
//}
