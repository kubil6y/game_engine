#ifndef MOVEMENT_SYSTEM_H
#define MOVEMENT_SYSTEM_H

#include "../Components/RigidBodyComponent.h"
#include "../Components/TransformComponent.h"
#include "../ECS/ECS.h"

class MovementSystem : public System {
public:
    MovementSystem() {
        RequireComponent<TransformComponent>();
        RequireComponent<RigidBodyComponent>();
    }

    void Update(double deltaTime) {
        // Loop all entities that the system is interested in
        for (auto entity : GetSystemEntities()) {
            // Update entity position based on its velocity
            auto&      tf = entity.GetComponent<TransformComponent>();
            const auto rb = entity.GetComponent<RigidBodyComponent>();

            tf.position += rb.velocity * static_cast<float>(deltaTime);
        }
    }
};

#endif // !MOVEMENT_SYSTEM_H
