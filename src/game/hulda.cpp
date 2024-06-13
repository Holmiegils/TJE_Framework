#include "hulda.h"
#include "framework/utils.h"
#include "graphics/EntityMesh.h"
#include "EntityCollider.h"
#include "game.h"
#include <iostream>

Hulda::Hulda() : health(50.0f), character_facing_rad(PI / 2), is_running(false), attack_duration(0), 
is_punching(false), hit_character(false), chase_threshold(100.0f), immunity(0) {
    mesh_matrix.setIdentity();
}

void Hulda::initialize() {
    mesh = Mesh::Get("data/meshes/hulda.MESH");
    texture = Texture::Get("data/textures/hulda.png");
    shader = Shader::Get("data/shaders/skinning.vs", "data/shaders/texture.fs");

    animator.playAnimation("data/animations/hulda/idle.skanim");

    mesh_matrix.setTranslation(Vector3(200, 0, 0));
}

void Hulda::render(Camera* camera) {
    shader->enable();

    shader->setUniform("u_color", Vector4(1, 1, 1, 1));
    shader->setUniform("u_viewprojection", camera->viewprojection_matrix);
    shader->setUniform("u_texture", texture, 0);
    shader->setUniform("u_model", mesh_matrix);
    shader->setUniform("u_time", Game::instance->time);

    mesh->renderAnimated(GL_TRIANGLES, &animator.getCurrentSkeleton());

    shader->disable();
}

void Hulda::update(double seconds_elapsed, Vector3 character_pos) {
    animator.update(seconds_elapsed);

    Vector3 position = mesh_matrix.getTranslation();
    float distance_to_character = (character_pos - position).length();

    bool moving = false;
    Vector3 direction = character_pos - position;
    direction.normalize();

    float target_angle = -atan2(direction.x, direction.z);

    if (distance_to_character < chase_threshold && distance_to_character > 15) {
        moving = true;

        if (attack_duration <= 0) {
            character_facing_rad = target_angle;

            float speed = 10.0f;
            position += direction * speed * seconds_elapsed;
        }
    }

    if (attack_duration > 0) {
        attack_duration -= seconds_elapsed;
    }

    //std::cout << is_running << ", " << moving << std::endl;
    if (!is_running && moving && attack_duration <= 0) {
        is_running = true;
        animator.playAnimation("data/animations/hulda/running.skanim");
    }                                    

    /*if (distance_to_character < 5) {
        if (!is_punching) {
            is_punching = true;
            character_facing_rad = target_angle;
            animator.playAnimation("data/animations/hulda/punch.skanim");
            attack_duration = 0.33f;
        }
    }*/
    //else is_punching = false;

    if (!moving) {
        if (is_running) {
            is_running = false;
            animator.playAnimation("data/animations/hulda/heavy.skanim");
        }
        if (attack_duration <= 0 && distance_to_character < chase_threshold) {
            attack_duration = 2.6f;
            character_facing_rad = target_angle;
        }
    }

    mesh_matrix.setTranslation(position);
    mesh_matrix.rotate(character_facing_rad, Vector3(0, 1, 0));
    mesh_matrix.scale(0.1f, 0.1f, 0.1f);

    if (immunity > 0) {
        immunity -= seconds_elapsed;
    }

    if (distance_to_character <= 15 && attack_duration > 1.1f && attack_duration < 1.2f) {
        // Check if the character is in front of Hulda
        Vector3 forward = mesh_matrix.frontVector();
        Vector3 to_character = (character_pos - position).normalize();
        float dot_product = forward.dot(to_character);

        // If the dot product is close to 1, it means the character is in front of Hulda
        if (dot_product > 0.09f) { // Adjust the threshold as needed
            hit_character = true;
        }
        else {
            hit_character = false;
        }
    }
    else hit_character = false;
}

Vector3 Hulda::getPosition() const {
    return mesh_matrix.getTranslation();
}

bool Hulda::heavyHit() const {
    return hit_character;
}

bool Hulda::isImmune() const {
    return immunity > 0;
}

void Hulda::setImmunity() {
    immunity = 0.2;
}

void Hulda::takeDamage(float damage) {
    health -= damage;
    std::cout << "health is: " << health << std::endl;
}
