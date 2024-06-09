#ifndef HULDA_H
#define HULDA_H

#pragma once

#include "framework/includes.h"
#include "framework/camera.h"
#include "graphics/mesh.h"
#include "graphics/texture.h"
#include "graphics/shader.h"
#include "framework/entities/entity.h"
#include "graphics/material.h"
#include <framework/animation.h>

class Hulda {
public:
    Hulda();

    void initialize();
    void render(Camera* camera);
    void update(double seconds_elapsed, Vector3 character_pos);

    void setPosition(const Vector3& position);
    Vector3 getPosition() const;

    void setFacing(float facing_rad);
    float getFacing() const;

    void performAttack();
    void takeDamage(float amount);
    bool isDefeated() const;

private:
    Mesh* mesh;
    Texture* texture;
    Shader* shader;
    Animator animator;
    Matrix44 mesh_matrix;
    float character_facing_rad;
    bool is_running;

    float health;
    bool defeated;
    float character_height;
    float sphere_collision_radius;
    float chase_threshold;

    bool right_punch;
    bool is_punching;
    float punch_duration;
    std::string attack_animation;
};

#endif // HULDA_H
