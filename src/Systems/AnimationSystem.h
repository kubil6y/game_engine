#ifndef ANIMATION_SYSTEM_H
#define ANIMATION_SYSTEM_H

#include "../Components/AnimationComponent.h"
#include "../Components/SpriteComponent.h"
#include "../ECS/ECS.h"
#include <SDL2/SDL.h>

class AnimationSystem : public System {
public:
    AnimationSystem() {
        RequireComponent<AnimationComponent>();
        RequireComponent<SpriteComponent>();
    }

    void Update() {
        for (auto entity : GetSystemEntities()) {
            auto& animation = entity.GetComponent<AnimationComponent>();
            auto& sprite = entity.GetComponent<SpriteComponent>();

            animation.currentFrame = ((SDL_GetTicks() - animation.startTime) *
                                      animation.frameRateSpeed / 1000) %
                                     animation.numFrames;
            sprite.srcRect.x = animation.currentFrame * sprite.width;
        }
    }
};

#endif // !ANIMATION_SYSTEM_H
