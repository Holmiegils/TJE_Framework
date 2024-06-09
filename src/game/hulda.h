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
    float getHealth() const { return health; }

private:
    Mesh* mesh;
    Texture* texture;
    Shader* shader;
    Animator animator;
    Matrix44 mesh_matrix;
    float character_facing_rad;
    bool is_running;

    float health;
    float chase_threshold;

    float punch_duration;
};

#endif // HULDA_H
