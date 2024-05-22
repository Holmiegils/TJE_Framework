#include "game.h"
#include "framework/utils.h"
#include "graphics/mesh.h"
#include "graphics/texture.h"
#include "graphics/fbo.h"
#include "graphics/shader.h"
#include "framework/input.h"
#include <framework/animation.h>

#include <cmath>

//some globals
Mesh* mesh = NULL;
Texture* texture = NULL;
Shader* shader = NULL;
//Animation* anim = NULL;
float angle = 0;
float mouse_speed = 100.0f;

Matrix44 mesh_matrix;
Animator animator;

Game* Game::instance = NULL;

Game::Game(int window_width, int window_height, SDL_Window* window)
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
	glEnable( GL_CULL_FACE ); //render both sides of every triangle
	glEnable( GL_DEPTH_TEST ); //check the occlusions using the Z buffer

	// Create our camera
	camera = new Camera();
	camera->lookAt(Vector3(0.f,100.f, 100.f),Vector3(0.f,0.f,0.f), Vector3(0.f,1.f,0.f)); //position the camera and point to 0,0,0
	camera->setPerspective(70.f,window_width/(float)window_height,0.1f,10000.f); //set the projection, we want to be perspective
	
	animator.playAnimation("data/animations/idle.skanim");

	// Load one texture using the Texture Manager
	texture = Texture::Get("data/textures/character/Guard_03__diffuse.png");

	// Example of loading Mesh from Mesh Manager
	mesh = Mesh::Get("data/meshes/character.MESH");

	mesh_matrix.setIdentity();
	mesh_matrix.scale(0.2f, 0.2f, 0.2f);

	// Example of shader loading using the shaders manager
	shader = Shader::Get("data/shaders/basic.vs", "data/shaders/texture.fs");

	// Hide the cursor
	SDL_ShowCursor(!mouse_locked); //hide or show the mouse
}

//what to do when the image has to be draw
void Game::render(void)
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

	if(shader)
	{
		// Enable shader
		shader->enable();

		// Upload uniforms
		shader->setUniform("u_color", Vector4(1,1,1,1));
		shader->setUniform("u_viewprojection", camera->viewprojection_matrix );
		shader->setUniform("u_texture", texture, 0);
		shader->setUniform("u_model", mesh_matrix);
		shader->setUniform("u_time", time);

		mesh->renderAnimated(GL_TRIANGLES, &animator.getCurrentSkeleton());

		// Do the draw call
		//mesh->render( GL_TRIANGLES );

		// Disable shader
		shader->disable();
	}
		
	// Draw the floor grid
	drawGrid();

	// Render the FPS, Draw Calls, etc
	drawText(2, 2, getGPUStats(), Vector3(1, 1, 1), 2);

	// Swap between front buffer and back buffer
	SDL_GL_SwapWindow(this->window);
}

void Game::update(double seconds_elapsed)
{
	animator.update(seconds_elapsed);

	float speed = seconds_elapsed * mouse_speed; //the speed is defined by the seconds_elapsed so it goes constant

	// Example
	angle += (float)seconds_elapsed * 10.0f;

	// Mouse input to rotate the cam
	if (Input::isMousePressed(SDL_BUTTON_LEFT) || mouse_locked) //is left button pressed?
	{
		camera->rotate(Input::mouse_delta.x * 0.005f, Vector3(0.0f,-1.0f,0.0f));
		camera->rotate(Input::mouse_delta.y * 0.005f, camera->getLocalVector( Vector3(-1.0f,0.0f,0.0f)));
	}

	// Async input to move the camera around
	if (Input::isKeyPressed(SDL_SCANCODE_LSHIFT) ) speed *= 10; //move faster with left shift
	if (Input::isKeyPressed(SDL_SCANCODE_W) || Input::isKeyPressed(SDL_SCANCODE_UP)) {
		camera->move(Vector3(0.0f, -0.7f, 0.7f) * speed);
		mesh_matrix.translate(Vector3(0.0f, 0.0f, -1.0f) * speed * 5);
	}
	if (Input::isKeyPressed(SDL_SCANCODE_S) || Input::isKeyPressed(SDL_SCANCODE_DOWN)) {
		camera->move(Vector3(0.0f, 0.7f, -0.7f) * speed);
		mesh_matrix.translate(Vector3(0.0f, 0.0f, 1.0f) * speed * 5);
	}
	if (Input::isKeyPressed(SDL_SCANCODE_A) || Input::isKeyPressed(SDL_SCANCODE_LEFT)) {
		camera->move(Vector3(1.0f, 0.0f, 0.0f) * speed);
		mesh_matrix.translate(Vector3(-1.0f, 0.0f, 0.0f) * speed * 5);
	}
	if (Input::isKeyPressed(SDL_SCANCODE_D) || Input::isKeyPressed(SDL_SCANCODE_RIGHT)) {
		camera->move(Vector3(-1.0f, 0.0f, 0.0f) * speed);
		mesh_matrix.translate(Vector3(1.0f, 0.0f, 0.0f) * speed * 5);
	}
}

//Keyboard event handler (sync input)
void Game::onKeyDown( SDL_KeyboardEvent event )
{
	switch(event.keysym.sym)
	{
		case SDLK_ESCAPE: must_exit = true; break; //ESC key, kill the app
		case SDLK_F1: Shader::ReloadAll(); break; 
	}
}

void Game::onKeyUp(SDL_KeyboardEvent event)
{

}

void Game::onMouseButtonDown( SDL_MouseButtonEvent event )
{
	if (event.button == SDL_BUTTON_MIDDLE) //middle mouse
	{
		mouse_locked = !mouse_locked;
		SDL_ShowCursor(!mouse_locked);
		SDL_SetRelativeMouseMode((SDL_bool)(mouse_locked));
	}
}

void Game::onMouseButtonUp(SDL_MouseButtonEvent event)
{

}

void Game::onMouseWheel(SDL_MouseWheelEvent event)
{
	mouse_speed *= event.y > 0 ? 1.1f : 0.9f;
}

void Game::onGamepadButtonDown(SDL_JoyButtonEvent event)
{

}

void Game::onGamepadButtonUp(SDL_JoyButtonEvent event)
{

}

void Game::onResize(int width, int height)
{
    std::cout << "window resized: " << width << "," << height << std::endl;
	glViewport( 0,0, width, height );
	camera->aspect =  width / (float)height;
	window_width = width;
	window_height = height;
}

