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
#include <framework/extra/bass.h>

class Hulda {
public:
    Hulda();

    void initialize();
    void render(Camera* camera);
    void update(double seconds_elapsed, Vector3 character_pos);

    Vector3 getPosition() const;
    float getHealth() const { return health; }
    bool heavyHit() const;
    bool isImmune() const;
    void setImmunity();
    void takeDamage(float damage);

    void loadAudio();

    void set_chase_threshold() {
        chase_threshold = 100.f;
    }

    void stopAudio();

    void setHealth();

private:
    Mesh* mesh;
    Texture* texture;
    Shader* shader;
    Animator animator;
    Matrix44 mesh_matrix;
    float character_facing_rad;
    float target_facing_rad; // Added member variable
    bool is_running;

    float health;
    float chase_threshold;

    float attack_duration;
    bool is_punching;
    bool hit_character;
    float immunity;

    HCHANNEL hHuldaIdleChannel;
    HCHANNEL hHuldaPunchChannel;

    // Added method for angle interpolation
    float lerpAngle(float a, float b, float t);
    
};

#endif // HULDA_H
