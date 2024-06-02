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

// Declaration of meshes_to_load
std::unordered_map<std::string, std::vector<Matrix44>> meshes_to_load;

bool parseScene(const char* filename, Entity* root)
{
    std::cout << " + Scene loading: " << filename << "..." << std::endl;

    std::ifstream file(filename);

    if (!file.good()) {
        std::cerr << "Scene [ERROR] File not found!" << std::endl;
        return false;
    }

    std::string scene_info, mesh_name, model_data;
    file >> scene_info >> scene_info;
    int mesh_count = 0;

    // Read file line by line and store mesh path and model info in separated variables
    while (file >> mesh_name >> model_data)
    {
        if (mesh_name[0] == '#')
            continue;

        // Get all 16 matrix floats
        std::vector<std::string> tokens = tokenize(model_data, ",");

        // Fill matrix converting chars to floats
        Matrix44 model;
        for (int t = 0; t < tokens.size(); ++t) {
            model.m[t] = (float)atof(tokens[t].c_str());
        }

        // Add model to mesh list (might be instanced!)
        meshes_to_load[mesh_name].push_back(model);
        mesh_count++;
    }

    // Iterate through meshes loaded and create corresponding entities
    for (const auto& data : meshes_to_load) {
        mesh_name = "data/" + data.first;
        const std::vector<Matrix44>& models = data.second;

        // No transforms, nothing to do here
        if (models.empty())
            continue;

        Material mat;
        EntityMesh* new_entity = nullptr;

        size_t tag = data.first.find("@tag");

        if (tag != std::string::npos) {
            Mesh* mesh = Mesh::Get("...");
            if (!mesh) {
                std::cerr << "Mesh loading failed (tag): " << mesh_name << std::endl;
                continue;
            }
            // Create a different type of entity
            // new_entity = new ...
        }
        else {
            Mesh* mesh = Mesh::Get(mesh_name.c_str());
            if (!mesh) {
                std::cerr << "Mesh loading failed: " << mesh_name << std::endl;
                continue;
            }
            new_entity = new EntityMesh(mesh, mat);
        }

        if (!new_entity) {
            std::cerr << "Entity creation failed: " << data.first << std::endl;
            continue;
        }

        new_entity->name = data.first;

        // Create instanced entity
        if (models.size() > 1) {
            new_entity->isInstanced = true;
            new_entity->models = models; // Add all instances
        }
        // Create normal entity
        else {
            new_entity->model = models[0];
        }

        // Add entity to scene root
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
    frame(0), time(0.0f), elapsed_time(0.0f), fps(0), must_exit(false), mouse_locked(false)
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
    mesh_matrix.scale(0.1f, 0.1f, 0.1f);
    //mesh_matrix.rotate(M_PI, Vector3(0.0f, 1.0f, 0.0f));

    // Example of shader loading using the shaders manager
    shader = Shader::Get("data/shaders/skinning.vs", "data/shaders/texture.fs");

    // Hide the cursor
    mouse_locked = !mouse_locked;
    SDL_ShowCursor(!mouse_locked);
    SDL_SetRelativeMouseMode((SDL_bool)(mouse_locked));
}

// what to do when the image has to be drawn
void Game::render()
{
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

    /*shader->enable();
    root->render(camera);

    shader->disable();*/
    // Draw the floor grid
    drawGrid();

    // Render the FPS, Draw Calls, etc.
    drawText(2, 2, getGPUStats(), Vector3(1, 1, 1), 2);

    // Swap between front buffer and back buffer
    SDL_GL_SwapWindow(this->window);
}

void Game::update(double seconds_elapsed)
{
    animator.update(seconds_elapsed);

    float speed = seconds_elapsed * mouse_speed; // the speed is defined by the seconds_elapsed so it goes constant

    // Example
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

    camera->lookAt(mesh_matrix.getTranslation() - front * 50, mesh_matrix.getTranslation(), Vector3(0, 1, 0));

    bool moving = false;

    if (Input::isKeyPressed(SDL_SCANCODE_W) || Input::isKeyPressed(SDL_SCANCODE_UP)) {
        mesh_matrix.translate(yawmat.frontVector());

        moving = true;
        if (!is_running) {
            is_running = true;
            animator.playAnimation("data/animations/running.skanim");
        }
    }

    if (!moving && is_running) {
        is_running = false;
        animator.playAnimation("data/animations/idle.skanim");
    }

    //mesh_matrix.setRotation(camera_yaw, Vector3(0, 1, 0));
}

// Keyboard event handler (sync input)
void Game::onKeyDown(SDL_KeyboardEvent event)
{
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
