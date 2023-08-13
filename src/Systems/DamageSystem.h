#ifndef DAMAGE_SYSTEM_H
#define DAMAGE_SYSTEM_H

#include "../Components/BoxColliderComponent.h"
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
        Logger::Log(
            "The DamageSystem received an event collision between entities " +
            std::to_string(event.a.GetId()) + " and " +
            std::to_string(event.b.GetId()));

        event.a.Kill();
        event.b.Kill();
    }

    void Update() {
        // TODO: ...
    }
};

#endif // !DAMAGE_SYSTEM_H
