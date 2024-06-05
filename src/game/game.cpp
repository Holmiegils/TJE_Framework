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
#include <framework/animation.h>
#include <cmath>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>

// some globals
Mesh* mesh = NULL;
Texture* texture = NULL;
Shader* shader = NULL;
// Animation* anim = NULL;
float angle = 0;
float mouse_speed = 100.0f;

bool is_running = false;

Matrix44 mesh_matrix;
Animator animator;

Game* Game::instance = NULL;

float camera_pitch;
float camera_yaw;

float character_x_pos = 0;
float character_y_pos = 0;

float character_facing_rad = 0;

// Declaration of meshes_to_load
std::unordered_map<std::string, std::vector<Matrix44>> meshes_to_load;

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


Game::Game(int window_width, int window_height, SDL_Window* window)
    : window(window), window_width(window_width), window_height(window_height),
    frame(0), time(0.0f), elapsed_time(0.0f), fps(0), must_exit(false), mouse_locked(false), game_started(false)
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

    // OpenGL flags
    glEnable(GL_CULL_FACE); // render both sides of every triangle
    glEnable(GL_DEPTH_TEST); // check the occlusions using the Z buffer

    // Create our camera
    camera = new Camera();
    camera->lookAt(Vector3(0.f, 1000.f, 1000.f), Vector3(0.f, 0.f, 0.f), Vector3(0.f, 1.f, 0.f)); // position the camera and point to 0,0,0
    camera->setPerspective(70.f, window_width / (float)window_height, 0.1f, 10000.f); // set the projection, we want to be perspective

    animator.playAnimation("data/animations/idle.skanim");

    // Load one texture using the Texture Manager
    texture = Texture::Get("data/textures/character/Guard_03__diffuse.png");

    root = new Entity();
    parseScene("data/myscene.scene", root);

    // Example of loading Mesh from Mesh Manager
    mesh = Mesh::Get("data/meshes/character.MESH");

    mesh_matrix.setIdentity();
    mesh_matrix.scale(0.05f, 0.05f, 0.05f);
    //mesh_matrix.rotate(M_PI, Vector3(0.0f, 1.0f, 0.0f));

    // Example of shader loading using the shaders manager
    shader = Shader::Get("data/shaders/skinning.vs", "data/shaders/texture.fs");

    // Initialize the main menu
    mainMenu = new MainMenu();

    // Hide the cursor
    mouse_locked = !mouse_locked;
    SDL_ShowCursor(!mouse_locked);
    SDL_SetRelativeMouseMode((SDL_bool)(mouse_locked));
}

// what to do when the image has to be drawn
void Game::render()
{
    if (!game_started) {
        renderMainMenu();
        return;
    }

    // Set the clear color (the background color)
    glClearColor(0.0, 0.0, 0.0, 1.0);

    // Clear the window and the depth buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set the camera as default
    camera->enable();

    // Set flags
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    shader->enable();

    shader->setUniform("u_color", Vector4(1, 1, 1, 1));
    shader->setUniform("u_viewprojection", camera->viewprojection_matrix);
    shader->setUniform("u_texture", texture, 0);
    shader->setUniform("u_model", mesh_matrix);
    shader->setUniform("u_time", time);

    mesh->renderAnimated(GL_TRIANGLES, &animator.getCurrentSkeleton());

    // Disable shader
    shader->disable();

    shader->enable();
    root->render(camera);
    shader->disable();

    // Draw the floor grid
    drawGrid();

    // Render the FPS, Draw Calls, etc.
    drawText(2, 2, getGPUStats(), Vector3(1, 1, 1), 2);

    // Swap between front buffer and back buffer
    SDL_GL_SwapWindow(this->window);
}

void Game::renderMainMenu() {
    // Set the clear color (the background color)
    glClearColor(0.0, 0.0, 0.0, 1.0);

    // Clear the window and the depth buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    mainMenu->render();

    // Swap between front buffer and back buffer
    SDL_GL_SwapWindow(this->window);
}

void Game::update(double seconds_elapsed) {
    if (!game_started) {
        mainMenu->update(seconds_elapsed);
        if (!mainMenu->isActive()) {
            game_started = true;
        }
        return;
    }

    animator.update(seconds_elapsed);

    float speed = seconds_elapsed * mouse_speed;
    angle += (float)seconds_elapsed * 10.0f;

    if (camera_pitch + Input::mouse_delta.y * seconds_elapsed < -0.01 && camera_pitch + Input::mouse_delta.y * seconds_elapsed > -1) {
        camera_pitch += Input::mouse_delta.y * seconds_elapsed;
    }
    camera_yaw += Input::mouse_delta.x * seconds_elapsed;

    Matrix44 pitchmat;
    pitchmat.setRotation(camera_pitch, Vector3(1, 0, 0));

    Matrix44 yawmat;
    yawmat.setRotation(camera_yaw, Vector3(0, 1, 0));

    Matrix44 unified;
    unified = pitchmat * yawmat;
    Vector3 front = unified.frontVector();

    camera->lookAt(mesh_matrix.getTranslation() - front * 25, mesh_matrix.getTranslation(), Vector3(0, 1, 0));

    bool moving = false;

    if (Input::isKeyPressed(SDL_SCANCODE_W) || Input::isKeyPressed(SDL_SCANCODE_UP)) {
        Vector3 previous_position = mesh_matrix.getTranslation();
        mesh_matrix.translate(yawmat.frontVector() * 20);
        moving = true;

        // Check for collisions
        Vector3 new_position = mesh_matrix.getTranslation();
        Vector3 ray_direction = new_position - previous_position;
        float distance = ray_direction.length();
        ray_direction.normalize();

        Vector3 collision_point, collision_normal;
        bool collision_detected = false;

        for (Entity* child : root->children) {
            EntityCollider* collider = dynamic_cast<EntityCollider*>(child);
            if (collider && (collider->layer & SCENARIO)) {
                if (collider->testCollision(collider->model, previous_position, ray_direction, collision_point, collision_normal, distance)) {
                    collision_detected = true;
                    break;
                }
            }
        }

        if (collision_detected) {
            mesh_matrix.setTranslation(previous_position);
        }
        else {
            if (!is_running) {
                is_running = true;
                animator.playAnimation("data/animations/running.skanim");
            }
        }
    }

    if (!moving && is_running) {
        is_running = false;
        animator.playAnimation("data/animations/idle.skanim");
    }
}


// Keyboard event handler (sync input)
void Game::onKeyDown(SDL_KeyboardEvent event)
{
    if (!game_started) {
        mainMenu->handleInput(event);
        return;
    }

    switch (event.keysym.sym)
    {
    case SDLK_ESCAPE:
        must_exit = true;
        break; // ESC key, kill the app
    case SDLK_F1:
        Shader::ReloadAll();
        break;
    }
}

void Game::onKeyUp(SDL_KeyboardEvent event)
{
    // Implementation here
}

void Game::onMouseButtonDown(SDL_MouseButtonEvent event)
{
    // Implementation here
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
