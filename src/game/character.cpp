////#include "character.h"
//#include "framework/utils.h"
//#include "graphics/EntityMesh.h"
//#include "EntityCollider.h"
//#include <cmath>
//#include <iostream>
//
//Character::Character() : mesh(nullptr), texture(nullptr), shader(nullptr), character_facing_rad(0), is_running(false),
//character_height(5.0f), sphere_collision_radius(4.0f), right_punch(true), is_punching(false), punch_duration(0) {
//    mesh_matrix.setIdentity();
//}
//
//Character::~Character() {
//    // Clean up resources if necessary
//}
//
//void Character::initialize(const std::string& mesh_file, const std::string& texture_file, const std::string& idle_animation, const std::string& run_animation) {
//    mesh = Mesh::Get(mesh_file.c_str());
//    texture = Texture::Get(texture_file.c_str());
//    shader = Shader::Get("data/shaders/skinning.vs", "data/shaders/texture.fs");
//
//    animator.playAnimation(idle_animation);
//
//    mesh_matrix.scale(0.05f, 0.05f, 0.05f);
//}
//
//void Character::render(Camera* camera, Shader* shader) {
//    shader->enable();
//
//    shader->setUniform("u_color", Vector4(1, 1, 1, 1));
//    shader->setUniform("u_viewprojection", camera->viewprojection_matrix);
//    shader->setUniform("u_texture", texture, 0);
//    shader->setUniform("u_model", mesh_matrix);
//    shader->setUniform("u_time", Game::instance->time);
//
//    mesh->renderAnimated(GL_TRIANGLES, &animator.getCurrentSkeleton());
//
//    shader->disable();
//}
//
//void Character::update(double seconds_elapsed) {
//    animator.update(seconds_elapsed);
//
//    // Update position, facing direction, and animation state based on input
//    // Similar to the logic in Game::update, but now encapsulated in the Character class
//}
//
//void Character::setPosition(const Vector3& position) {
//    mesh_matrix.setTranslation(position);
//}
//
//Vector3 Character::getPosition() const {
//    return mesh_matrix.getTranslation();
//}
//
//void Character::setFacing(float facing_rad) {
//    character_facing_rad = facing_rad;
//    mesh_matrix.rotate(character_facing_rad, Vector3(0, 1, 0));
//    mesh_matrix.scale(0.05f, 0.05f, 0.05f);
//}
//
//float Character::getFacing() const {
//    return character_facing_rad;
//}
//
//void Character::playAnimation(const std::string& animation) {
//    animator.playAnimation(animation);
//}
