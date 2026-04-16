#pragma once

#include "engine/core/ecs/world.h"
#include "engine/core/ecs/actor_components.h"
#include "engine/core/ecs/collision_components.h"

namespace urpg {

class CollisionSystem {
public:
    struct AABB {
        Vector3 min;
        Vector3 max;
    };

    /**
     * @brief Very simple O(N^2) AABB collision detection.
     * Note: Optimization (spatial partitioning) should be added later.
     */
    void update(World& world) {
        std::vector<EntityID> entities;
        world.ForEachWith<TransformComponent, CollisionBoxComponent>([&](EntityID id, TransformComponent&, CollisionBoxComponent&) {
            entities.push_back(id);
        });

        for (size_t i = 0; i < entities.size(); ++i) {
            for (size_t j = i + 1; j < entities.size(); ++j) {
                EntityID a = entities[i];
                EntityID b = entities[j];

                auto* transA = world.GetComponent<TransformComponent>(a);
                auto* boxA = world.GetComponent<CollisionBoxComponent>(a);
                auto* transB = world.GetComponent<TransformComponent>(b);
                auto* boxB = world.GetComponent<CollisionBoxComponent>(b);

                AABB aabbA = getWorldAABB(*transA, *boxA);
                AABB aabbB = getWorldAABB(*transB, *boxB);

                if (intersects(aabbA, aabbB)) {
                    // Logic for responding to collisions (e.g., resolving position) goes here.
                    // For now, we'll just stop movement if they collide.
                    if (auto* velA = world.GetComponent<VelocityComponent>(a)) {
                        velA->linear = Vector3::Zero();
                    }
                    if (auto* velB = world.GetComponent<VelocityComponent>(b)) {
                        velB->linear = Vector3::Zero();
                    }
                }
            }
        }
    }

private:
    AABB getWorldAABB(const TransformComponent& transform, const CollisionBoxComponent& box) {
        Vector3 pos = transform.position + box.offset;
        Vector3 halfSize = box.size * Fixed32::FromRaw(32768); // size / 2

        return AABB{
            pos - halfSize,
            pos + halfSize
        };
    }

    bool intersects(const AABB& a, const AABB& b) {
        return (a.min.x < b.max.x && a.max.x > b.min.x) &&
               (a.min.y < b.max.y && a.max.y > b.min.y) &&
               (a.min.z < b.max.z && a.max.z > b.min.z);
    }
};

} // namespace urpg
