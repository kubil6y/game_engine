#ifndef DAMAGE_SYSTEM_H
#define DAMAGE_SYSTEM_H

#include "../Components/BoxColliderComponent.h"
#include "../Components/HealthComponent.h"
#include "../Components/ProjectileComponent.h"
#include "../ECS/ECS.h"
#include "../EventBus/EventBus.h"
#include "../Events/CollisionEvent.h"
#include "../Logger/Logger.h"

class DamageSystem : public System {
public:
    DamageSystem() { RequireComponent<BoxColliderComponent>(); }

    void SubscribeToEvents(std::unique_ptr<EventBus>& eventBus) {
        eventBus->SubscribeToEvent<CollisionEvent>(this,
                                                   &DamageSystem::onCollision);
    }

    void onCollision(CollisionEvent& event) {
        Entity a = event.a;
        Entity b = event.b;

        Logger::Log(
            "The DamageSystem received an event collision between entities " +
            std::to_string(a.GetId()) + " and " + std::to_string(b.GetId()));

        if (a.BelongsToGroup("projectiles") && b.HasTag("player")) {
            OnProjectileHitsPlayer(a, b);
        }

        if (b.BelongsToGroup("projectiles") && a.HasTag("player")) {
            OnProjectileHitsPlayer(b, a);
        }

        if (a.BelongsToGroup("projectiles") && b.BelongsToGroup("enemies")) {
            OnProjectileHitsEnemy(a, b);
        }

        if (b.BelongsToGroup("projectiles") && a.BelongsToGroup("enemies")) {
            OnProjectileHitsEnemy(b, a);
        }
    }

    void OnProjectileHitsPlayer(Entity projectile, Entity player) {
        auto projectileComponent =
            projectile.GetComponent<ProjectileComponent>();

        if (!projectileComponent.isFriendly) {
            auto& playerHealth = player.GetComponent<HealthComponent>();
            playerHealth.healthPercentage -=
                projectileComponent.hitPercentDamage;

            // FIXME: this should be an event!
            if (playerHealth.healthPercentage <= 0) {
                player.Kill();
            }

            // Kill the projectile
            projectile.Kill();
        }
    }

    void OnProjectileHitsEnemy(Entity projectile, Entity enemy) {
        auto projectileComponent =
            projectile.GetComponent<ProjectileComponent>();


        if (projectileComponent.isFriendly) {
            auto& enemyHealth = enemy.GetComponent<HealthComponent>();
            enemyHealth.healthPercentage -= projectileComponent.hitPercentDamage;

            // FIXME: this should be an event!
            if (enemyHealth.healthPercentage <= 0) {
                enemy.Kill();
            }

            // Kill the projectile
            projectile.Kill();
        }
    }

    void Update() {
        // TODO: ...
    }
};

#endif // !DAMAGE_SYSTEM_H
