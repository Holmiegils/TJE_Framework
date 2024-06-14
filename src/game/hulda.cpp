#include "hulda.h"
#include "framework/utils.h"
#include "graphics/EntityMesh.h"
#include "EntityCollider.h"
#include "game.h"
#include <iostream>
#include <bass.h>

Hulda::Hulda() : health(50.0f), character_facing_rad(PI / 2), target_facing_rad(PI / 2), is_running(false), attack_duration(0),
is_punching(false), hit_character(false), chase_threshold(100.0f), immunity(0) {
    mesh_matrix.setIdentity();
}

void Hulda::initialize() {
    mesh = Mesh::Get("data/meshes/hulda.MESH");
    texture = Texture::Get("data/textures/hulda.png");
    shader = Shader::Get("data/shaders/skinning.vs", "data/shaders/texture.fs");

    animator.playAnimation("data/animations/hulda/idle.skanim");

    mesh_matrix.setTranslation(Vector3(200, 0, 0));

    loadAudio();
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

    target_facing_rad = -atan2(direction.x, direction.z);

    if (distance_to_character < chase_threshold && distance_to_character > 15) {
        moving = true;

        if (attack_duration <= 0) {
            float speed = 10.0f;
            position += direction * speed * seconds_elapsed;
        }
    }

    if (attack_duration > 0) {
        attack_duration -= seconds_elapsed;
    }

    if (!is_running && moving && attack_duration <= 0) {
        is_running = true;
        animator.playAnimation("data/animations/hulda/running.skanim");
        BASS_ChannelPlay(hHuldaIdleChannel, true);
        BASS_ChannelStop(hHuldaPunchChannel);
    }

    if (!moving) {
        if (is_running) {
            is_running = false;
            animator.playAnimation("data/animations/hulda/heavy.skanim");
            BASS_ChannelPlay(hHuldaPunchChannel, true);
            BASS_ChannelSetAttribute(hHuldaPunchChannel, BASS_ATTRIB_FREQ, 44100 * 0.38f);
        }
        if (attack_duration <= 0 && distance_to_character < chase_threshold) {
            attack_duration = 2.6f;
        }
    }

    // Smoothly rotate towards the target facing angle
    float rotation_speed = 2.0f; // Adjust this value to change the rotation speed
    character_facing_rad = lerpAngle(character_facing_rad, target_facing_rad, rotation_speed * seconds_elapsed);

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
        if (dot_product > 0.08f) { // Adjust the threshold as needed
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
    immunity = 0.2f;
}

void Hulda::takeDamage(float damage) {
    health -= damage;
    std::cout << "health is: " << health << std::endl;
}

void Hulda::loadAudio() {
    hHuldaIdleChannel = BASS_StreamCreateFile(false, "data/audio/hulda_idle.wav", 0, 0, 0);
    if (hHuldaIdleChannel == 0) {
        std::cerr << "Error loading audio stream: char_death.wav" << std::endl;
        return;
    }
    BASS_ChannelSetAttribute(hHuldaIdleChannel, BASS_ATTRIB_VOL, 1.0);

    hHuldaPunchChannel = BASS_StreamCreateFile(false, "data/audio/hulda_punch.wav", 0, 0, BASS_SAMPLE_LOOP);
    if (hHuldaPunchChannel == 0) {
        std::cerr << "Error loading audio stream: char_death.wav" << std::endl;
        return;
    }
    BASS_ChannelSetAttribute(hHuldaPunchChannel, BASS_ATTRIB_VOL, 1.3);
}

// Linear interpolation function for angles
float Hulda::lerpAngle(float a, float b, float t) {
    float delta = b - a;
    while (delta < -PI) delta += 2 * PI;
    while (delta > PI) delta -= 2 * PI;
    return a + delta * t;
}
