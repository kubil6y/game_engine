#ifndef CAMERA_MOVEMENT_SYSTEM_H
#define CAMERA_MOVEMENT_SYSTEM_H

#include "../Components/CameraFollowComponent.h"
#include "../Components/TransformComponent.h"
#include "../ECS/ECS.h"
#include "../EventBus/EventBus.h"
#include "../Game/Game.h"
#include "../Logger/Logger.h"
#include <SDL2/SDL.h>
#include <glm/glm.hpp>

class CameraMovementSystem : public System {
public:
    CameraMovementSystem() {
        RequireComponent<TransformComponent>();
        RequireComponent<CameraFollowComponent>();
    }

    void Update(SDL_Rect& camera) {
        for (auto entity : GetSystemEntities()) {
            const auto& tf = entity.GetComponent<TransformComponent>();

            if (tf.position.x + static_cast<int>(camera.w / 2) <
                Game::s_mapWidth) {
                camera.x =
                    tf.position.x - static_cast<int>(Game::s_windowWidth / 2);
            }

            if (tf.position.y + static_cast<int>(camera.h / 2) <
                Game::s_mapHeight) {
                camera.y =
                    tf.position.y - static_cast<int>(Game::s_windowHeight / 2);
            }

            camera.x = glm::clamp(camera.x, 0, camera.w);
            camera.y = glm::clamp(camera.y, 0, camera.h);
        }
    }
};

#endif // !CAMERA_MOVEMENT_SYSTEM_H
