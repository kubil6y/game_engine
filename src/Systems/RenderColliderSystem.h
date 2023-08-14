#ifndef RENDER_COLLIDER_SYSTEM_H
#define RENDER_COLLIDER_SYSTEM_H

#include "../Components/BoxColliderComponent.h"
#include "../Components/TransformComponent.h"
#include "../ECS/ECS.h"
#include <SDL2/SDL.h>

class RenderColliderSystem : public System {
public:
    RenderColliderSystem() {
        RequireComponent<TransformComponent>();
        RequireComponent<BoxColliderComponent>();
    }

    void Update(SDL_Renderer* renderer, const SDL_Rect& camera) {
        for (auto entity : GetSystemEntities()) {
            const auto& collider = entity.GetComponent<BoxColliderComponent>();
            const auto& tf = entity.GetComponent<TransformComponent>();

            SDL_Rect rect = {
                static_cast<int>(tf.position.x + collider.offset.x - camera.x),
                static_cast<int>(tf.position.y + collider.offset.y - camera.y),
                static_cast<int>(collider.width * tf.scale.x),
                static_cast<int>(collider.height * tf.scale.y)};

            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            SDL_RenderDrawRect(renderer, &rect);
        }
    }
};

#endif // !RENDER_COLLIDER_SYSTEM_H
