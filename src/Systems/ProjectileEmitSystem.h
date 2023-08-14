#ifndef PROJECTILE_EMIT_SYSTEM_H
#define PROJECTILE_EMIT_SYSTEM_H

#include "../Components/BoxColliderComponent.h"
#include "../Components/CameraFollowComponent.h"
#include "../Components/ProjectileComponent.h"
#include "../Components/ProjectileEmitterComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/SpriteComponent.h"
#include "../Components/TransformComponent.h"
#include "../ECS/ECS.h"
#include "../EventBus/EventBus.h"
#include "../Events/KeyPressedEvent.h"
#include <SDL2/SDL.h>
#include <glm/glm.hpp>

class ProjectileEmitSystem : public System {
public:
    ProjectileEmitSystem() {
        RequireComponent<ProjectileEmitterComponent>();
        RequireComponent<TransformComponent>();
    }

    void SubscribeToEvents(std::unique_ptr<EventBus>& eventBus) {
        eventBus->SubscribeToEvent<KeyPressedEvent>(
            this, &ProjectileEmitSystem::OnKeyPressed);
    }

    void OnKeyPressed(KeyPressedEvent& event) {
        if (event.symbol == SDLK_SPACE) {
            for (auto entity : GetSystemEntities()) {
                if (entity.HasComponent<CameraFollowComponent>()) {
                    auto& projectileEmitter =
                        entity.GetComponent<ProjectileEmitterComponent>();
                    const auto& tf = entity.GetComponent<TransformComponent>();
                    const auto& rb = entity.GetComponent<RigidBodyComponent>();

                    // Calculate projectile position
                    glm::vec2 projectilePos = tf.position;
                    if (entity.HasComponent<SpriteComponent>()) {
                        const auto& sprite =
                            entity.GetComponent<SpriteComponent>();
                        projectilePos = glm::vec2(
                            tf.position.x +
                                static_cast<int>(sprite.width / 2) * tf.scale.x,
                            tf.position.y +
                                static_cast<int>(sprite.height / 2) *
                                    tf.scale.y);
                    }

                    // If parent entity direction is controlled by the keyboard
                    // keys, modify
                    glm::vec2 projectileVelocity =
                        projectileEmitter.projectileVelocity;
                    // clang-format off
                    int directionX = 0;
                    int directionY = 0;
                    if (rb.velocity.x > 0) directionX = +1;
                    if (rb.velocity.x < 0) directionX = -1;
                    if (rb.velocity.y > 0) directionY = +1;
                    if (rb.velocity.y < 0) directionY = -1;
                    // clang-format on

                    projectileVelocity.x =
                        projectileEmitter.projectileVelocity.x * directionX;
                    projectileVelocity.y =
                        projectileEmitter.projectileVelocity.y * directionY;

                    // TODO: chopper stands still, starts direction top
                    if (projectileVelocity == glm::vec2(0)) {
                        projectileVelocity = glm::vec2(
                            0.0, -projectileEmitter.projectileVelocity.y);
                    }

                    Entity projectile = entity.registry->CreateEntity();
                    projectile.AddComponent<TransformComponent>(
                        projectilePos, glm::vec2(1.0, 1.0), 0.0);
                    projectile.AddComponent<RigidBodyComponent>(
                        projectileVelocity);
                    projectile.AddComponent<SpriteComponent>("bullet-image", 4,
                                                             4, 4);
                    projectile.AddComponent<BoxColliderComponent>(4, 4);
                    projectile.AddComponent<ProjectileComponent>(
                        projectileEmitter.isFriendly,
                        projectileEmitter.hitPercentDamage,
                        projectileEmitter.projectileDuration);
                }
            }
        }
    }

    void Update(std::unique_ptr<Registry>& registry) {
        for (auto entity : GetSystemEntities()) {
            const auto& tf = entity.GetComponent<TransformComponent>();
            auto&       projectileEmitter =
                entity.GetComponent<ProjectileEmitterComponent>();

            if (projectileEmitter.repeatFrequency == 0) {
                continue;
            }

            // Check if its time to re-emit a new projectile
            if (static_cast<int>(SDL_GetTicks()) -
                    projectileEmitter.lastEmissionTime >
                projectileEmitter.repeatFrequency) {
                // Calculate projectile position
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
                projectile.AddComponent<ProjectileComponent>(
                    projectileEmitter.isFriendly,
                    projectileEmitter.hitPercentDamage,
                    projectileEmitter.projectileDuration);

                projectileEmitter.lastEmissionTime = SDL_GetTicks();
            }
        }
    }
};

#endif // !PROJECTILE_EMIT_SYSTEM_H
