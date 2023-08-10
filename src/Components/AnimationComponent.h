#ifndef ANIMATION_COMPONENT_H
#define ANIMATION_COMPONENT_H

struct AnimationComponent {
    int  numFrames;
    int  currentFrame;
    int  frameRateSpeed;
    bool shouldLoop;

    AnimationComponent(int numFrames = 1, int frameRateSpeed = 10,
                       bool shouldLoop = false) {
        this->numFrames = numFrames;
        this->currentFrame = 1;
        this->frameRateSpeed = frameRateSpeed;
        this->shouldLoop = shouldLoop;
    }
};

#endif // !ANIMATION_COMPONENT_H
