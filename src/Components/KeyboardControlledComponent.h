#ifndef KEYBOARD_CONTROLLED_COMPONENT
#define KEYBOARD_CONTROLLED_COMPONENT

#include "../ECS/ECS.h"
#include <SDL2/SDL.h>
#include <glm/glm.hpp>

struct KeyboardControlledComponent {
    // clockwise order
    glm::vec2 upVelocity;
    glm::vec2 rightVelocity;
    glm::vec2 downVelocity;
    glm::vec2 leftVelocity;

    KeyboardControlledComponent(glm::vec2 upVelocity = glm::vec2(0),
                                glm::vec2 rightVelocity = glm::vec2(0),
                                glm::vec2 downVelocity = glm::vec2(0),
                                glm::vec2 leftVelocity = glm::vec2(0)) {
        this->upVelocity = upVelocity;
        this->rightVelocity = rightVelocity;
        this->downVelocity = downVelocity;
        this->leftVelocity = leftVelocity;
    }
};

#endif // !KEYBOARD_CONTROLLED_COMPONENT
