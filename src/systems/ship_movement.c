#include "ship_movement.h"
#include "../ecs.h"
#include "../game_state.h"
#include <math.h>
#include <stdint.h>

void update_ship_movement(float dt) {
    const uint8_t     required_mask = COMP_TRANSFORM | COMP_SHIP_PHYSICS | COMP_SPRITE;
    const uint8_t*    entity_masks  = ecs_get_masks_array();
    c_transform_t*    transforms    = ecs_get_transforms_array();
    c_ship_physics_t* ship_physics  = ecs_get_ship_physics_array();
    c_sprite_t*       sprites       = ecs_get_sprites_array();

    for (int i = 0; i < MAX_ENTITIES; i++) {
        if ((entity_masks[i] & required_mask) != required_mask)
            continue;

        ship_physics[i].rudder_deg += game_state.pd->system->getCrankChange() * 0.1f;
        if (ship_physics[i].rudder_deg > 45.0f)
            ship_physics[i].rudder_deg = 45.0f;
        if (ship_physics[i].rudder_deg < -45.0f)
            ship_physics[i].rudder_deg = -45.0f;

        transforms[i].heading_deg += (ship_physics[i].rudder_deg * TURN_RATE * dt);
        while (transforms[i].heading_deg >= 360.0f)
            transforms[i].heading_deg -= 360.0f;
        while (transforms[i].heading_deg < 0.0f)
            transforms[i].heading_deg += 360.0f;

        float heading_rad = transforms[i].heading_deg * DEG2RAD;
        float x_vel       = sinf(heading_rad) * CRUISE_SPEED * dt;
        float y_vel       = -cosf(heading_rad) * CRUISE_SPEED * dt;

        float                ax, ay;
        int                  len;
        SpriteCollisionInfo* results = game_state.pd->sprite->moveWithCollisions(
            sprites[i].sprite, transforms[i].x + x_vel, transforms[i].y + y_vel, &ax, &ay, &len);

        transforms[i].x = ax;
        transforms[i].y = ay;
        game_state.pd->system->realloc(results, 0);
    }
}
