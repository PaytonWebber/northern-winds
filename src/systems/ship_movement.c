#include "ship_movement.h"
#include "../core/math.h"
#include "../ecs.h"
#include "../game_state.h"
#include <math.h>
#include <stdint.h>

static const float S_CRUISE_SPEED = 30.0f;
static const float S_TURN_RATE    = 1.2f;

void update_ship_movement(float dt) {
    const uint8_t         required_mask = COMP_TRANSFORM | COMP_SHIP_INPUT | COMP_SPRITE;
    const uint8_t*        entity_masks  = ecs_get_masks_array();
    const c_ship_input_t* ship_input    = ecs_get_ship_inputs_array();
    c_transform_t*        transforms    = ecs_get_transforms_array();
    c_sprite_t*           sprites       = ecs_get_sprites_array();

    for (int i = 0; i < MAX_ENTITIES; i++) {
        if ((entity_masks[i] & required_mask) != required_mask)
            continue;

        transforms[i].heading_deg += (ship_input[i].rudder_target_deg * S_TURN_RATE * dt);
        while (transforms[i].heading_deg >= 360.0f)
            transforms[i].heading_deg -= 360.0f;
        while (transforms[i].heading_deg < 0.0f)
            transforms[i].heading_deg += 360.0f;

        const float  heading_rad = transforms[i].heading_deg * DEG2RAD;
        const vec2_t forward     = dir_from_angle(heading_rad);
        const vec2_t velocity    = vec2_scale(forward, S_CRUISE_SPEED);

        float                ax, ay;
        int                  len;
        SpriteCollisionInfo* results =
            game_state.pd->sprite->moveWithCollisions(sprites[i].sprite,
                                                      transforms[i].pos.x + velocity.x * dt,
                                                      transforms[i].pos.y + velocity.y * dt,
                                                      &ax,
                                                      &ay,
                                                      &len);

        transforms[i].pos = (vec2_t) {.x = ax, .y = ay};
        game_state.pd->system->realloc(results, 0);
    }
}
