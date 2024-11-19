#include "animation_system.hpp"
#include "tiny_ecs_registry.hpp"

void AnimationSystem::step(float elapsed_ms) {
    for (auto entity : registry.animations.entities) {
        if (!registry.players.has(entity)) continue;
        
        auto& player = registry.players.get(entity);
        auto& animation = registry.animations.get(entity);
        auto& render_request = registry.renderRequests.get(entity);

        if (player.is_moving) {
            animation.frame_time += elapsed_ms;
            if (animation.frame_time >= FRAME_TIME) {
                animation.frame_time = 0.f;
                animation.current_frame = (animation.current_frame + 1) % animation.frames.size();
                render_request.used_texture = animation.frames[animation.current_frame];
            }
        } else {
            // Reset to first frame when not moving
            animation.current_frame = 0;
            animation.frame_time = 0.f;
            render_request.used_texture = animation.frames[0];
        }
    }
} 