#include "character.h"
#include "framework/utils.h"
#include "graphics/EntityMesh.h"
#include "EntityCollider.h"
#include "game.h"
#include <iostream>
#include "framework/bass.h"'


Character::Character() : is_running(false), character_facing_rad(0), right_punch(true), is_punching(false), punch_duration(0),
character_height(5.0f), immunity(0), sphere_collision_radius(4.0f) {
    mesh_matrix.setIdentity();
}

void Character::initialize() {
    mesh = Mesh::Get("data/meshes/character.MESH");
    texture = Texture::Get("data/textures/character.png");
    shader = Shader::Get("data/shaders/skinning.vs", "data/shaders/texture.fs");

    animator.playAnimation("data/animations/character/idle.skanim");

    mesh_matrix.setIdentity();
    mesh_matrix.scale(0.05f, 0.05f, 0.05f);
}

void Character::render(Camera* camera) {
    shader->enable();

    shader->setUniform("u_color", Vector4(1, 1, 1, 1));
    shader->setUniform("u_viewprojection", camera->viewprojection_matrix);
    shader->setUniform("u_texture", texture, 0);
    shader->setUniform("u_model", mesh_matrix);
    shader->setUniform("u_time", Game::instance->time);

    mesh->renderAnimated(GL_TRIANGLES, &animator.getCurrentSkeleton());

    shader->disable();
}

void Character::update(double seconds_elapsed, const Vector3& camera_front, float camera_yaw) {
    animator.update(seconds_elapsed);

    float speed = 25.0f;
    float sprint_speed = 50.0f;

    if (Input::isKeyPressed(SDL_SCANCODE_LSHIFT)) {
        speed = sprint_speed;
    }

    Vector3 front = camera_front;
    front.y = 0.0f; // DISCARD HEIGHT IN THE DIRECTION

    Vector3 position = mesh_matrix.getTranslation();
    bool moving = false;

    if (Input::isKeyPressed(SDL_SCANCODE_W)) {
        moving = true;
        character_facing_rad = camera_yaw;
        position += front * seconds_elapsed * speed;
    }

    if (Input::isKeyPressed(SDL_SCANCODE_S)) {
        moving = true;
        character_facing_rad = camera_yaw - PI;
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
        else {
            character_facing_rad = camera_yaw - PI / 2;
        }

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
        else {
            character_facing_rad = camera_yaw + PI / 2;
        }

        position.x -= front.z * seconds_elapsed * speed;
        position.z += front.x * seconds_elapsed * speed;
    }

    if (punch_duration > 0) {
        punch_duration -= seconds_elapsed;
        is_running = false;
        moving = false;
    }

    if (Input::isMousePressed(SDL_BUTTON_LEFT)) {
         if (punch_duration < 0.5f) {
            character_facing_rad = camera_yaw;
            if (right_punch) animator.playAnimation("data/animations/character/right_punch.skanim");
            else animator.playAnimation("data/animations/character/left_punch.skanim");
            right_punch = !right_punch;
            is_punching = true;
            punch_duration = 1;
        }
    }

    if (moving) {
        if (!is_running) {
            is_running = true;
            animator.playAnimation("data/animations/character/running.skanim");
        }

        Vector3 collision_point, collision_normal;
        bool collision_detected = false;

        for (Entity* child : Game::instance->root->children) {
            EntityCollider* collider = dynamic_cast<EntityCollider*>(child);
            if (collider && (collider->layer & SCENARIO)) {
                if (collider->testSphereCollision(collider->model, position + Vector3(0.f, character_height, 0.0f), sphere_collision_radius, collision_point, collision_normal)) {
                    collision_detected = true;
                    break;
                }
            }
        }

        if (!collision_detected) {
            mesh_matrix.setTranslation(position);
            mesh_matrix.rotate(character_facing_rad, Vector3(0, 1, 0));
            mesh_matrix.scale(0.05f, 0.05f, 0.05f);
        }
    }

    if (!moving && is_running || is_punching && punch_duration <= 0) {
        is_running = false;
        is_punching = false;
        animator.playAnimation("data/animations/character/idle.skanim");
    }

    if (immunity > 0) {
        immunity -= seconds_elapsed;
    }
}

Vector3 Character::getPosition() const {
    return mesh_matrix.getTranslation();
}

bool Character::isRunning() const {
    return is_running;
}

bool Character::isPunching() const {
    return punch_duration == 1;
}

bool Character::isImmune() const {
    return immunity > 0;
}

void Character::setImmunity() {
    immunity = 0.2;
}

//void Character::setPosition(const Vector3& position) {
//    mesh_matrix.setTranslation(position);
//}
//
//float Character::getFacing() const {
//    return character_facing_rad;
//}
//
//void Character::setFacing(float facing_rad) {
//    character_facing_rad = facing_rad;
//    mesh_matrix.rotate(character_facing_rad, Vector3(0, 1, 0));
//    mesh_matrix.scale(0.05f, 0.05f, 0.05f);
//}
//
//bool Character::isPunching() const {
//    return is_punching;
//}
