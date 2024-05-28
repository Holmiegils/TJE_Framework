#ifndef ENTITYCOLLIDER_H
#define ENTITYCOLLIDER_H

#include "graphics/EntityMesh.h"
#include <framework/extra/coldet/coldetimpl.h>

class EntityCollider : public EntityMesh {
public:
    bool is_dynamic = false;
    int layer = 0; // Layer for collision filtering
    CollisionModel3D* collision_model;

    EntityCollider(Mesh* mesh, const Material& material, int layer = 0)
        : EntityMesh(mesh, material), layer(layer) {
        collision_model = newCollisionModel3D();  // Initialize Coldet collision model
        createCollisionModel();
    }

    ~EntityCollider() {
        delete collision_model;
    }

    void createCollisionModel() {
        collision_model->setTriangleNumber(mesh->num_triangles);
        for (int i = 0; i < mesh->num_triangles; ++i) {
            Vector3 v1 = mesh->getVertex(mesh->indices[i * 3]);
            Vector3 v2 = mesh->getVertex(mesh->indices[i * 3 + 1]);
            Vector3 v3 = mesh->getVertex(mesh->indices[i * 3 + 2]);
            collision_model->addTriangle(v1.ptr(), v2.ptr(), v3.ptr());
        }
        collision_model->finalize();
    }

    bool testSphereCollision(const Vector3& center, float radius, Vector3& collision, Vector3& normal) {
        return collision_model->sphereCollision(center.ptr(), radius, collision.ptr(), normal.ptr());
    }
};

#endif
