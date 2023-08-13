#ifndef RENDER_SYSTEM_H
#define RENDER_SYSTEM_H

#include "../AssetStore/AssetStore.h"
#include "../Components/SpriteComponent.h"
#include "../Components/TransformComponent.h"
#include "../ECS/ECS.h"
#include <SDL2/SDL.h>
#include <bits/stdc++.h>

class RenderSystem : public System {
public:
    RenderSystem() {
        RequireComponent<TransformComponent>();
        RequireComponent<SpriteComponent>();
    }

    void Update(SDL_Renderer* renderer, std::unique_ptr<AssetStore>& assetStore,
                const SDL_Rect& camera) {
        // Create a vector with both Sprite and Transform components of all
        // entities
        struct RenderableEntity {
            TransformComponent transformComponent;
            SpriteComponent    spriteComponent;
        };

        std::vector<RenderableEntity> renderableEntities;

        for (auto entity : GetSystemEntities()) {
            RenderableEntity renderableEntity;
            renderableEntity.spriteComponent =
                entity.GetComponent<SpriteComponent>();
            renderableEntity.transformComponent =
                entity.GetComponent<TransformComponent>();
            renderableEntities.emplace_back(renderableEntity);
        }

        // Sort the vector by the z-index value
        std::sort(
            renderableEntities.begin(), renderableEntities.end(),
            [](const RenderableEntity& first, const RenderableEntity& second) {
                return first.spriteComponent.zIndex <
                       second.spriteComponent.zIndex;
            });

        // Loop all entities that the system is interested in
        for (auto entity : renderableEntities) {
            const auto tf = entity.transformComponent;
            const auto sprite = entity.spriteComponent;

            // Set the source rectangle of our original sprite texture
            SDL_Rect srcRect = sprite.srcRect;

            // Set the destination rectangle with the x,y position to be
            // rendered
            SDL_Rect dstRect = {static_cast<int>(tf.position.x - camera.x),
                                static_cast<int>(tf.position.y - camera.y),
                                static_cast<int>(sprite.width * tf.scale.x),
                                static_cast<int>(sprite.height * tf.scale.y)};

            // Draw the texture on the destination renderer
            SDL_RenderCopyEx(renderer, assetStore->GetTexture(sprite.assetId),
                             &srcRect, &dstRect, tf.rotation, NULL,
                             SDL_FLIP_NONE);
        }
    }
};

#endif // !RENDER_SYSTEM_H
