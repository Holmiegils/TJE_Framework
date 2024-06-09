//#ifndef CHARACTER_H
//#define CHARACTER_H
//
//#include "framework/includes.h"
//#include "framework/camera.h"
//#include "graphics/mesh.h"
//#include "graphics/texture.h"
//#include "graphics/shader.h"
//#include "framework/entities/entity.h"
//#include "graphics/material.h"
//#include <framework/animation.h>
//
//class Character {
//public:
//    Character();
//    ~Character();
//
//    void initialize(const std::string& mesh_file, const std::string& texture_file, const std::string& idle_animation, const std::string& run_animation);
//    void render(Camera* camera, Shader* shader);
//    void update(double seconds_elapsed);
//
//    void setPosition(const Vector3& position);
//    Vector3 getPosition() const;
//
//    void setFacing(float facing_rad);
//    float getFacing() const;
//
//    void playAnimation(const std::string& animation);
//
//private:
//    Mesh* mesh;
//    Texture* texture;
//    Shader* shader;
//    Animator animator;
//    Matrix44 mesh_matrix;
//    float character_facing_rad;
//    bool is_running;
//
//    float character_height;
//    float sphere_collision_radius;
//
//    bool right_punch;
//    bool is_punching;
//    float punch_duration;
//};
//
//#endif // CHARACTER_H
