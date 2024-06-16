#ifndef ENTITYMESH_H
#define ENTITYMESH_H

#include "framework/entities/entity.h"
#include "graphics/mesh.h"
#include "graphics/material.h"
#include "graphics/shader.h"

class EntityMesh : public Entity {
public:
    Mesh* mesh;
    Material material;
    std::vector<Matrix44> models; // For instanced rendering
    bool isInstanced;

    EntityMesh(Mesh* mesh, const Material& material) : mesh(mesh), material(material), isInstanced(false) {}

    void render(Camera* camera) {
        if (!mesh) return;

        Shader* shader = Shader::Get("data/shaders/basic.vs", "data/shaders/texture.fs");  // Ensure shader is used here

        shader->enable();


        shader->setUniform("u_color", Vector4(1.0));
        shader->setUniform("u_viewprojection", camera->viewprojection_matrix);

        if (isInstanced) {
            for (const auto& model : models) {
                shader->setUniform("u_model", model);
                shader->setUniform("u_tiling", 4.0f);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                mesh->render(GL_TRIANGLES);
            }
        }
        else {
            shader->setUniform("u_model", model);
            shader->setUniform("u_tiling", 4.0f);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            mesh->render(GL_TRIANGLES);
        }

        shader->disable();
    }
};

#endif
