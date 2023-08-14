#ifndef PROJECTILE_EMIT_SYSTEM_H
#define PROJECTILE_EMIT_SYSTEM_H

#include "../Components/BoxColliderComponent.h"
#include "../Components/ProjectileEmitterComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/SpriteComponent.h"
#include "../Components/TransformComponent.h"
#include "../ECS/ECS.h"
#include <SDL2/SDL.h>
#include <glm/glm.hpp>

class ProjectileEmitSystem : public System {
public:
    ProjectileEmitSystem() {
        RequireComponent<ProjectileEmitterComponent>();
        RequireComponent<TransformComponent>();
    }

    void Update(std::unique_ptr<Registry>& registry) {
        for (auto entity : GetSystemEntities()) {
            auto& projectileEmitter =
                entity.GetComponent<ProjectileEmitterComponent>();
            const auto& tf = entity.GetComponent<TransformComponent>();

            // Check if its time to re-emit a new projectile
            if (static_cast<int>(SDL_GetTicks()) -
                    projectileEmitter.lastEmissionTime >
                projectileEmitter.repeatFrequency) {
                glm::vec2 projectilePos = tf.position;
                if (entity.HasComponent<SpriteComponent>()) {
                    const auto& sprite = entity.GetComponent<SpriteComponent>();

                    projectilePos = glm::vec2(
                        tf.position.x +
                            static_cast<int>(sprite.width / 2) * tf.scale.x,
                        tf.position.y +
                            static_cast<int>(sprite.height / 2) * tf.scale.y);
                }

                Entity projectile = registry->CreateEntity();
                projectile.AddComponent<TransformComponent>(
                    projectilePos, glm::vec2(1.0, 1.0));
                projectile.AddComponent<RigidBodyComponent>(
                    projectileEmitter.projectileVelocity);
                projectile.AddComponent<SpriteComponent>("bullet-image", 4, 4,
                                                         4);
                projectile.AddComponent<BoxColliderComponent>(4, 4);

                projectileEmitter.lastEmissionTime = SDL_GetTicks();
            }
        }
    }
};

#endif // !PROJECTILE_EMIT_SYSTEM_H
