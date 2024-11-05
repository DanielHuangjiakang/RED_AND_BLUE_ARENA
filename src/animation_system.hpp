#pragma once

#include "common.hpp"

class AnimationSystem {
public:
    void step(float elapsed_ms);
private:
    const float FRAME_TIME = 200.f;
}; 