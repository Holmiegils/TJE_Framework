#ifndef CHARACTER_H
#define CHARACTER_H

#include "framework/includes.h"
#include "framework/camera.h"
#include "graphics/mesh.h"
#include "graphics/texture.h"
#include "graphics/shader.h"
#include "framework/entities/entity.h"
#include "graphics/material.h"
#include <framework/animation.h>

class Character {
public:
    Character();

    void initialize();
    void render(Camera* camera);
    void update(double seconds_elapsed, const Vector3& camera_front, float camera_yaw, Vector3 hulda_pos);


    Vector3 getPosition() const;
    bool isRunning() const;
    bool isPunching() const;
    bool isImmune() const;
    void setImmunity();
    bool huldaIsHit() const;

    void setSpeed(float new_speed);
    float getSpeed() const;
    /*void setPosition(const Vector3& position);

    float getFacing() const;
    void setFacing(float facing_rad);

    bool isPunching() const;*/

private:
    Mesh* mesh;
    Texture* texture;
    Shader* shader;
    Animator animator;
    Matrix44 mesh_matrix;
    
    bool is_running;
    float character_facing_rad;
    bool right_punch;
    bool is_punching;
    float punch_duration;
    float character_height;
    float sphere_collision_radius;
    float immunity;
    bool hit_hulda;

    float speed;
};

#endif // CHARACTER_H
