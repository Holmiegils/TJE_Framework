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

float character_facing_rad = 0;

// ALEX: WE DON'T NEED IT
// Vector3 character_pos;

// ALEX: USE THIS TO INCREASE THE POSITION IN Y AND AVOID DOING STUFF ON THE CHARACTER FEET.
float character_height = 5.0f;
// ALEX: SIZE OF THE SPHERE OF THE COLLISION TEST (TWEAK THIS VALUE AS YOU LIKE!)
float sphere_collision_radius = 4.0f;

bool right_punch = true;
bool is_punching = false;
float punch_duration = 0;

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

    // ALEX: THIS IS DEBUG CODE, COMMENT IT WHEN NOT NEEDED
    //renderDebugCollisions();
    // 

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

    float speed = 15.0f;

    if (camera_pitch + Input::mouse_delta.y * seconds_elapsed < -0.01 && camera_pitch + Input::mouse_delta.y * seconds_elapsed > -1) {
        camera_pitch += Input::mouse_delta.y * seconds_elapsed;
    }

    // ALEX: I HAVE FLIPPED THE DIRECTION SO IT IS MORE INTUITIVE, BUT FEEL FREE TO CHANGE IT AGAIN IF YOU LIKE THE OTHER WAY!
    camera_yaw -= Input::mouse_delta.x * seconds_elapsed;

    Matrix44 pitchmat;
    pitchmat.setRotation(camera_pitch, Vector3(1, 0, 0));

    Matrix44 yawmat;
    yawmat.setRotation(camera_yaw, Vector3(0, 1, 0));

    Matrix44 unified;
    unified = pitchmat * yawmat;
    Vector3 front = unified.frontVector();

    camera->lookAt(mesh_matrix.getTranslation() - front * 25, mesh_matrix.getTranslation(), Vector3(0, 1, 0));

    bool moving = false;

    Vector3 position = mesh_matrix.getTranslation();

    if (Input::isKeyPressed(SDL_SCANCODE_W)) {

        moving = true;
        character_facing_rad = camera_yaw;
        front.y = 0.0f; // DISCARD HEIGHT IN THE DIRECTION
        position += front * seconds_elapsed * speed;
    }

    if (Input::isKeyPressed(SDL_SCANCODE_S)) {
        moving = true;
        character_facing_rad = camera_yaw - PI;
        front.y = 0.0f;
        position -= front * seconds_elapsed * speed;
    }

    if (Input::isKeyPressed(SDL_SCANCODE_A)) {
        moving = true;

        if (Input::isKeyPressed(SDL_SCANCODE_W)) {
            character_facing_rad = camera_yaw - PI / 4;
        }
        else if (Input::isKeyPressed(SDL_SCANCODE_S)) {
            character_facing_rad = camera_yaw - 3 * PI / 4;
        }
        else character_facing_rad = camera_yaw - PI / 2;

        position.x += front.z * seconds_elapsed * speed;
        position.z -= front.x * seconds_elapsed * speed;
    }

    if (Input::isKeyPressed(SDL_SCANCODE_D)) {
        moving = true;

        if (Input::isKeyPressed(SDL_SCANCODE_W)) {
            character_facing_rad = camera_yaw + PI / 4;
        }
        else if (Input::isKeyPressed(SDL_SCANCODE_S)) {
            character_facing_rad = camera_yaw + 3 * PI / 4;
        }
        else character_facing_rad = camera_yaw + PI / 2;

        position.x -= front.z * seconds_elapsed * speed;
        position.z += front.x * seconds_elapsed * speed;
    }

    if (moving) {
        if (!is_running) {
            is_running = true;
            animator.playAnimation("data/animations/running.skanim");
        }

        Vector3 collision_point, collision_normal;
        bool collision_detected = false;

        for (Entity* child : root->children) {
            EntityCollider* collider = dynamic_cast<EntityCollider*>(child);
            if (collider && (collider->layer & SCENARIO)) {
                // ALEX: CHECK FOR COLLISIONS USING A SPHERE IN NEW CHARACTER POSITION + HEIGHT OFFSET
                // (BASICALLY, CHECK I CAN GO TO THAT POSITION)
                if (collider->testSphereCollision(collider->model, position + Vector3(0.f, character_height, 0.0f), sphere_collision_radius, collision_point, collision_normal)) {
                    collision_detected = true;
                    break;
                }
            }
        }



        if (!collision_detected) {

            //  IF  NOT COLLIDED, APPLY NEW POSITION

            mesh_matrix.setTranslation(position);
            mesh_matrix.rotate(character_facing_rad, Vector3(0, 1, 0));
            mesh_matrix.scale(0.05f, 0.05f, 0.05f);
        }
    }

    if (!moving && is_running || !moving && is_punching && punch_duration <= 0) {
        is_running = false;
        is_punching = false;
        animator.playAnimation("data/animations/idle.skanim");
    }

    if (punch_duration > 0) {
        punch_duration -= seconds_elapsed;
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
    if (punch_duration < 0.5f) {
        character_facing_rad = camera_yaw;
        if (right_punch) animator.playAnimation("data/animations/right_punch.skanim");
        else animator.playAnimation("data/animations/left_punch.skanim");
        right_punch = !right_punch;
        is_punching = true;
        punch_duration = 1;
    }
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

// ALEX: RENDER COLLISION SPHERES AS DEBUG TO CHECK IF WE ARE DOING STUFF CORRECTLY!
void Game::renderDebugCollisions()
{
    Shader* shader = Shader::Get("data/shaders/basic.vs", "data/shaders/flat.fs");
    Mesh* mesh = Mesh::Get("data/meshes/sphere.obj");

    shader->enable();

    {
        Matrix44 m;
        m.setTranslation(mesh_matrix.getTranslation());
        m.translate(0.0f, character_height, 0.0f);
        m.scale(sphere_collision_radius, sphere_collision_radius, sphere_collision_radius);

        shader->setUniform("u_viewprojection", camera->viewprojection_matrix);
        shader->setUniform("u_color", Vector4(0.0f, 1.0f, 0.0f, 1.0f));
        shader->setUniform("u_model", m);

        mesh->render(GL_LINES);
    }

    shader->disable();
}