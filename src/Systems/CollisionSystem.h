#ifndef COLLISION_SYSTEM_H
#define COLLISION_SYSTEM_H

#include "../Components/BoxColliderComponent.h"
#include "../Components/TransformComponent.h"
#include "../ECS/ECS.h"
#include "../EventBus/EventBus.h"
#include "../Events/CollisionEvent.h"
#include "../Logger/Logger.h"

class CollisionSystem : public System {
public:
    CollisionSystem() {
        RequireComponent<BoxColliderComponent>();
        RequireComponent<TransformComponent>();
    }

    void Update(std::unique_ptr<EventBus>& eventBus) {
        auto entities = GetSystemEntities();

        for (auto i = entities.begin(); i != entities.end(); i++) {
            Entity a = *i;
            auto   aTf = a.GetComponent<TransformComponent>();
            auto   aCollider = a.GetComponent<BoxColliderComponent>();

            // Loop all the entities need to be checked (to the right of the i)
            for (auto j = i; j != entities.end(); j++) {
                // Check the collision of A and B
                Entity b = *j;
                // By pass if we are trying to test the same entity
                if (a == b) { // == operator is overloaded with id checks
                    continue;
                }
                auto bTf = b.GetComponent<TransformComponent>();
                auto bCollider = b.GetComponent<BoxColliderComponent>();

                bool hasCollision = CheckAABCCollision(
                    aTf.position.x + aCollider.offset.x * aTf.scale.x,
                    aTf.position.y + aCollider.offset.y * aTf.scale.y,
                    aCollider.width * aTf.scale.x,
                    aCollider.height * aTf.scale.y,
                    bTf.position.x + bCollider.offset.x * bTf.scale.x,
                    bTf.position.y + bCollider.offset.y * bTf.scale.y,
                    bCollider.width * bTf.scale.x,
                    bCollider.height * bTf.scale.y);

                if (hasCollision) {
                    Logger::Log("Entity " + std::to_string(a.GetId()) +
                                " is colliding with entity " +
                                std::to_string(b.GetId()));

                    eventBus->EmitEvent<CollisionEvent>(a, b);
                }
            }
        }
    }

    bool CheckAABCCollision(double aX, double aY, double aW, double aH,
                            double bX, double bY, double bW, double bH) {
        return aX < bX + bW && aX + aW > bX && aY < bY + bH && aY + aH > bY;
    }
};

#endif // !COLLISION_SYSTEM_H
