#include "ship_movement.h"
#include "../core/math.h"
#include "../ecs.h"
#include "../game_state.h"
#include "particles.h"
#include "wind.h"
#include <math.h>
#include <stdint.h>

static const float S_TURN_RATE              = 1.2f;
static const float S_ACCELERATION           = 18.0f;
static const float S_DECELERATION           = 10.0f;
static const float S_RUDDER_DEFLECTION_RATE = 160.0f;
static const float S_RUDDER_RETURN_RATE     = 240.0f;
static const float S_MAX_RUDDER_DEG         = 45.0f;
static const float S_FULL_TURN_SPEED        = 32.0f;
static const float S_MIN_TURN_AUTHORITY     = 0.35f;
static const float S_MAX_RUDDER_DRAG        = 0.15f;
static const float S_WIND_DRIFT_SPEED       = 2.0f;
static const float S_HULL_HALF_HEIGHT       = 16.0f;
static const float S_HULL_HALF_WIDTH        = 8.0f;
static const float S_BOW_SPRAY_OFFSET       = 2.0f;
static const float S_KELVIN_WAKE_ANGLE_DEG  = 20.0f;
static const float S_WAKE_EMIT_INTERVAL     = 1.5f;
static const float S_STERN_WAKE_LIFE        = 1.5f;
static const float S_SPRAY_LIFE             = 1.0f;
static const float S_BOW_SPRAY_ANGLE_DEG    = 40.0f;
static const int   S_STERN_PARTICLES        = 5;
static const int   S_BOW_SPRAY_PARTICLES    = 5;

static float approach(float current, float target, float max_delta) {
    if (current < target)
        return clampf(current + max_delta, current, target);
    return clampf(current - max_delta, target, current);
}

static void emit_bow_spray(vec2_t origin, vec2_t starboard_dir, float forward_speed) {
    const float lateral_speed = forward_speed * tanf(S_BOW_SPRAY_ANGLE_DEG * DEG2RAD);
    for (int p = 0; p < S_BOW_SPRAY_PARTICLES; p++) {
        const float  side = (rand() & 1) ? 1.0f : -1.0f;
        const vec2_t spawn_pos =
            vec2_add(origin, vec2_scale(starboard_dir, side * S_BOW_SPRAY_OFFSET));
        const float  speed_factor = 1.0f + rand_signed() * 0.3f;
        const vec2_t spawn_vel    = vec2_scale(starboard_dir, side * lateral_speed * speed_factor);
        particles_spawn(spawn_pos, spawn_vel, S_SPRAY_LIFE * (0.5f + rand_unit() * 0.5f));
    }
}

static void emit_kelvin_wake(vec2_t origin, vec2_t starboard_dir, float lateral_speed) {
    const float life = 1.5f + rand_signed() * 0.3f;
    for (int i = 0; i < 2; i++) {
        const float  side = i ? -1.0f : 1.0f;
        const vec2_t spawn_pos =
            vec2_add(origin, vec2_scale(starboard_dir, S_HULL_HALF_WIDTH * side));
        const vec2_t spawn_vel = vec2_scale(starboard_dir, lateral_speed * side);
        particles_spawn(spawn_pos, spawn_vel, life);
    }
}

static void emit_wake_foam(vec2_t origin, vec2_t starboard_dir, float lateral_speed) {
    for (int p = 0; p < S_STERN_PARTICLES; p++) {
        const vec2_t spawn_pos =
            vec2_add(origin, vec2_scale(starboard_dir, rand_signed() * S_HULL_HALF_WIDTH));
        const float  speed_factor = rand_signed() * 0.6f;
        const vec2_t spawn_vel    = vec2_scale(starboard_dir, lateral_speed * speed_factor);
        particles_spawn(spawn_pos, spawn_vel, S_STERN_WAKE_LIFE + rand_signed() * 0.5f);
    }
}

static void
update_ship_wake(c_transform_t* t, float heading_rad, float moved, float forward_speed) {
    t->wake_accumulator += moved;

    const vec2_t forward_dir   = dir_from_angle(heading_rad);
    const vec2_t backward_dir  = dir_from_angle(heading_rad + PI);
    const vec2_t starboard_dir = dir_from_angle(heading_rad + PI * 0.5f);
    const float  lateral_speed = forward_speed * tanf(S_KELVIN_WAKE_ANGLE_DEG * DEG2RAD);

    while (t->wake_accumulator >= S_WAKE_EMIT_INTERVAL) {
        t->wake_accumulator -= S_WAKE_EMIT_INTERVAL;
        const vec2_t stern_pos = vec2_add(t->pos, vec2_scale(backward_dir, S_HULL_HALF_HEIGHT));
        const vec2_t bow_pos   = vec2_add(t->pos, vec2_scale(forward_dir, S_HULL_HALF_HEIGHT));
        emit_kelvin_wake(stern_pos, starboard_dir, lateral_speed);
        emit_wake_foam(stern_pos, starboard_dir, lateral_speed);
        emit_bow_spray(bow_pos, starboard_dir, forward_speed);
    }
}

static void update_ship_motion(c_ship_motion_t*      motion,
                               const c_ship_input_t* input,
                               const c_sail_t*       sail,
                               float                 dt) {
    const float rudder_rate =
        input->rudder_target_deg == 0.0f ? S_RUDDER_RETURN_RATE : S_RUDDER_DEFLECTION_RATE;
    motion->rudder_deg = approach(motion->rudder_deg, input->rudder_target_deg, rudder_rate * dt);

    const float rudder_amount = fabsf(motion->rudder_deg) / S_MAX_RUDDER_DEG;
    const float target_speed  = sail->drive_speed * (1.0f - rudder_amount * S_MAX_RUDDER_DRAG);
    const float speed_rate = target_speed > motion->forward_speed ? S_ACCELERATION : S_DECELERATION;
    motion->forward_speed  = approach(motion->forward_speed, target_speed, speed_rate * dt);
}

void update_ship_movement(float dt) {
    const uint8_t required_mask =
        COMP_TRANSFORM | COMP_SHIP_INPUT | COMP_SHIP_MOTION | COMP_SPRITE | COMP_SAIL;
    const uint8_t*        entity_masks = ecs_get_masks_array();
    const c_ship_input_t* ship_inputs  = ecs_get_ship_inputs_array();
    const c_sail_t*       sails        = ecs_get_sails_array();
    c_ship_motion_t*      motions      = ecs_get_ship_motions_array();
    c_transform_t*        transforms   = ecs_get_transforms_array();
    c_sprite_t*           sprites      = ecs_get_sprites_array();
    const wind_state_t*   wind         = wind_get_state();
    const vec2_t          wind_dir     = wind_get_direction();

    for (int i = 0; i < MAX_ENTITIES; i++) {
        if ((entity_masks[i] & required_mask) != required_mask)
            continue;

        update_ship_motion(&motions[i], &ship_inputs[i], &sails[i], dt);

        const float speed_amount = clampf(motions[i].forward_speed / S_FULL_TURN_SPEED, 0.0f, 1.0f);
        const float turn_authority =
            S_MIN_TURN_AUTHORITY + (1.0f - S_MIN_TURN_AUTHORITY) * speed_amount;
        transforms[i].heading_deg += motions[i].rudder_deg * S_TURN_RATE * turn_authority * dt;
        transforms[i].heading_deg = wrap_degrees(transforms[i].heading_deg);

        const float  heading_rad = transforms[i].heading_deg * DEG2RAD;
        const vec2_t forward     = dir_from_angle(heading_rad);
        const float  drift_speed =
            S_WIND_DRIFT_SPEED * wind->speed * (1.0f - sails[i].efficiency * 0.4f);
        const vec2_t forward_velocity = vec2_scale(forward, motions[i].forward_speed);
        const vec2_t drift_velocity   = vec2_scale(wind_dir, drift_speed);
        const vec2_t velocity         = vec2_add(forward_velocity, drift_velocity);
        transforms[i].vel             = velocity;

        float                ax, ay;
        int                  len;
        SpriteCollisionInfo* results =
            game_state.pd->sprite->moveWithCollisions(sprites[i].sprite,
                                                      transforms[i].pos.x + velocity.x * dt,
                                                      transforms[i].pos.y + velocity.y * dt,
                                                      &ax,
                                                      &ay,
                                                      &len);

        const float moved         = hypotf(ax - transforms[i].pos.x, ay - transforms[i].pos.y);
        const float wake_distance = fminf(moved, motions[i].forward_speed * dt);
        transforms[i].pos         = (vec2_t) {.x = ax, .y = ay};
        game_state.pd->system->realloc(results, 0);

        update_ship_wake(&transforms[i], heading_rad, wake_distance, motions[i].forward_speed);
    }
}
