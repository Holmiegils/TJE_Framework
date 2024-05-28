#include "game.h"
#include "framework/utils.h"
#include "graphics/mesh.h"
#include "graphics/texture.h"
#include "graphics/fbo.h"
#include "graphics/shader.h"
#include "framework/input.h"
#include "framework/animation.h"

#include <cmath>

// some globals
Mesh* mesh = NULL;
Texture* texture = NULL;
Shader* shader = NULL;
// Animation* anim = NULL;
float angle = 0;
float mouse_speed = 100.0f;

Matrix44 mesh_matrix;
Animator animator;

Game* Game::instance = NULL;

float camera_pitch;
float camera_yaw;

bool is_running = false;

Game::Game(int window_width, int window_height, SDL_Window* window)
    : window(window), window_width(window_width), window_height(window_height),
    frame(0), time(0.0f), elapsed_time(0.0f), fps(0), must_exit(false), mouse_locked(false)
{
    instance = this;

    // OpenGL flags
    glEnable(GL_CULL_FACE); // render both sides of every triangle
    glEnable(GL_DEPTH_TEST); // check the occlusions using the Z buffer

    // Create our camera
    camera = new Camera();
    camera->lookAt(Vector3(0.f, 1000.f, 1000.f), Vector3(0.f, 0.f, 0.f), Vector3(0.f, 1.f, 0.f)); // position the camera and point to 0,0,0
    camera->setPerspective(70.f, window_width / (float)window_height, 0.1f, 10000.f); // set the projection, we want to be perspective

    // Load one texture using the Texture Manager
    texture = Texture::Get("data/textures/character/Guard_03__diffuse.png");

    // Example of loading Mesh from Mesh Manager
    mesh = Mesh::Get("data/meshes/character.MESH");

    mesh_matrix.setIdentity();
    mesh_matrix.rotate(M_PI, Vector3(0.0f, 1.0f, 0.0f));

    // Example of shader loading using the shaders manager
    shader = Shader::Get("data/shaders/skinning.vs", "data/shaders/texture.fs");

    // Hide the cursor
    SDL_ShowCursor(mouse_locked); // hide or show the mouse
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

    if (shader)
    {
        // Enable shader
        shader->enable();

        // Upload uniforms
        shader->setUniform("u_color", Vector4(1, 1, 1, 1));
        shader->setUniform("u_viewprojection", camera->viewprojection_matrix);
        shader->setUniform("u_texture", texture, 0);
        shader->setUniform("u_model", mesh_matrix);
        shader->setUniform("u_time", time);

        mesh->renderAnimated(GL_TRIANGLES, &animator.getCurrentSkeleton());

        // Disable shader
        shader->disable();
    }

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

    camera_pitch += Input::mouse_delta.y * seconds_elapsed;
    camera_yaw += Input::mouse_delta.x * seconds_elapsed;

    Matrix44 pitchmat;
    pitchmat.setRotation(camera_pitch, Vector3(1, 0, 0));

    Matrix44 yawmat;
    yawmat.setRotation(camera_yaw, Vector3(0, 1, 0));

    mesh_matrix.setRotation(camera_yaw, Vector3(0, 1, 0));

    Matrix44 unified;
    unified = pitchmat * yawmat;
    Vector3 front = unified.frontVector();

    camera->lookAt(mesh_matrix.getTranslation() - front * 500, mesh_matrix.getTranslation(), Vector3(0, 1, 0));

    bool moving = false;

    if (Input::isKeyPressed(SDL_SCANCODE_W) || Input::isKeyPressed(SDL_SCANCODE_UP)) {
        moving = true;
        if (!is_running) {
            is_running = true;
            animator.playAnimation("data/animations/running.skanim");
        }
    }

    if (!moving) {
        animator.playAnimation("data/animations/idle.skanim");
        is_running = false;
    }
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
    if (event.button == SDL_BUTTON_MIDDLE) // middle mouse
    {
        mouse_locked = !mouse_locked;
        SDL_ShowCursor(!mouse_locked);
        SDL_SetRelativeMouseMode((SDL_bool)(mouse_locked));
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
