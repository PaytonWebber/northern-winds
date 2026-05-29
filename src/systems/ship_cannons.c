#include "ship_cannons.h"
#include "../ecs.h"
#include "cannonballs.h"

static const float S_RELOAD_SECONDS = 1.0f;
static const float S_FIRE_SPEED     = 60.0f;

void update_ship_cannons(float dt) {
    const uint8_t        required_mask = COMP_TRANSFORM | COMP_SHIP_INPUT;
    const uint8_t*       entity_masks  = ecs_get_masks_array();
    const c_transform_t* transforms    = ecs_get_transforms_array();
    c_ship_input_t*      ship_inputs   = ecs_get_ship_inputs_array();

    for (int i = 0; i < MAX_ENTITIES; i++) {
        if ((entity_masks[i] & required_mask) != required_mask)
            continue;

        if (ship_inputs[i].fire_delay > 0.0f)
            ship_inputs[i].fire_delay -= dt;

        if (ship_inputs[i].fire_starboard && ship_inputs[i].fire_delay <= 0.0f) {
            ship_inputs[i].fire_delay = S_RELOAD_SECONDS;
            const vec2_t fire_dir = dir_from_angle(transforms[i].heading_deg * DEG2RAD + PI * 0.5f);
            const vec2_t spawn_vel =
                vec2_add(transforms[i].vel, vec2_scale(fire_dir, S_FIRE_SPEED));
            const vec2_t spawn_pos = vec2_add(transforms[i].pos, vec2_scale(fire_dir, 16.0f));
            cannonballs_fire(spawn_pos, spawn_vel, i);
        }

        if (ship_inputs[i].fire_port && ship_inputs[i].fire_delay <= 0.0f) {
            ship_inputs[i].fire_delay = S_RELOAD_SECONDS;
            const vec2_t fire_dir = dir_from_angle(transforms[i].heading_deg * DEG2RAD + PI * 1.5f);
            const vec2_t spawn_vel =
                vec2_add(transforms[i].vel, vec2_scale(fire_dir, S_FIRE_SPEED));
            const vec2_t spawn_pos = vec2_add(transforms[i].pos, vec2_scale(fire_dir, 16.0f));
            cannonballs_fire(spawn_pos, spawn_vel, i);
        }

        ship_inputs[i].fire_starboard = 0;
        ship_inputs[i].fire_port      = 0;
    }
}
