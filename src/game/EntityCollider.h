#include "framework/entities/entity.h"
#include "graphics/mesh.h"
#include "framework/utils.h"
#include "graphics/EntityMesh.h"
#include <vector>

enum CollisionLayer {
    NONE = 0,
    FLOOR = 1 << 0,
    WALL = 1 << 1,
    PLAYER = 1 << 2,
    ENEMY = 1 << 3,
    SCENARIO = WALL | FLOOR,
    ALL = 0xFF
};

class EntityCollider : public EntityMesh {
public:
    bool is_dynamic = false;
    int layer = NONE;

    // Adjust the constructor to match EntityMesh parameters
    EntityCollider(Mesh* mesh, Material material) : EntityMesh(mesh, material) {
        mesh->createCollisionModel(!is_dynamic); // Initialize collision model
    }

    bool testCollision(Matrix44 model, Vector3 ray_origin, Vector3 ray_direction, Vector3& collision, Vector3& normal, float max_ray_dist) {
        return mesh->testRayCollision(model, ray_origin, ray_direction, collision, normal, max_ray_dist, false);
    }

    bool testSphereCollision(Matrix44 model, Vector3 center, float radius, Vector3& collision, Vector3& normal) {
        return mesh->testSphereCollision(model, center, radius, collision, normal);
    }
};
