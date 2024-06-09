#include "hulda.h"
#include "framework/utils.h"
#include "graphics/EntityMesh.h"
#include "EntityCollider.h"
#include "game.h"
#include <iostream>

Hulda::Hulda() : health(50.0f), character_facing_rad(PI/2), is_running(false), punch_duration(0), chase_threshold(100.0f) {
    mesh_matrix.setIdentity();
}

void Hulda::initialize() {
    mesh = Mesh::Get("data/meshes/hulda.MESH");
    texture = Texture::Get("data/textures/hulda.png");
    shader = Shader::Get("data/shaders/skinning.vs", "data/shaders/texture.fs");

    animator.playAnimation("data/animations/hulda/idle.skanim");

    mesh_matrix.setTranslation(Vector3(200, 0, 0));
    mesh_matrix.scale(0.1f, 0.1f, 0.1f);
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

    if (distance_to_character < chase_threshold && distance_to_character > 12 && punch_duration <= 0) {
        moving = true;

        Vector3 direction = character_pos - position;
        direction.normalize();

        float target_angle = -atan2(direction.x, direction.z);
        character_facing_rad = target_angle;

        float speed = 10.0f;
        position += direction * speed * seconds_elapsed;
    }

    if (punch_duration > 0) {
        punch_duration -= seconds_elapsed;
        is_running = false;
        moving = false;
    }

    if (!is_running && moving) {
        is_running = true;
        animator.playAnimation("data/animations/hulda/running.skanim");
    }

    if (!moving && is_running) {
        is_running = false;
        animator.playAnimation("data/animations/hulda/attack.skanim");
        punch_duration = 2.6f;
    }

    mesh_matrix.setTranslation(position);
    mesh_matrix.rotate(character_facing_rad, Vector3(0, 1, 0));
    mesh_matrix.scale(0.1f, 0.1f, 0.1f);
}


