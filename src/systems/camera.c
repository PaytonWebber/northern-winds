#include "camera.h"
#include "../ecs.h"
#include "../game_state.h"
#include "pd_api.h"
#include <math.h>

static const float S_FOLLOW_RATE = 2.0f;

typedef struct {
    vec2_t pos;
    int    target_entity; // -1 for no follow
} camera_t;

static camera_t camera = {
    (vec2_t) {.x = 0, .y = 0},
     .target_entity = -1
};

vec2_t camera_get_position(void) {
    return camera.pos;
}

void camera_set_follow(int id) {
    if (id >= MAX_ENTITIES || id < 0)
        return;
    camera.target_entity = id;

    const c_transform_t* t = ecs_get_transform_const(id);
    camera.pos             = t->pos;
}

void camera_update_follow(float dt) {
    if (camera.target_entity < 0)
        return;
    const c_transform_t* t = ecs_get_transform_const(camera.target_entity);

    float k    = 1.0f - expf(-S_FOLLOW_RATE * dt);
    camera.pos = vec2_add(camera.pos, vec2_scale(vec2_sub(t->pos, camera.pos), k));
}

void camera_world_to_screen(vec2_t world_pos, int* sx, int* sy) {
    *sx = (int) floorf(world_pos.x - camera.pos.x + SCREEN_WIDTH * 0.5f);
    *sy = (int) floorf(world_pos.y - camera.pos.y + SCREEN_HEIGHT * 0.5f);
}

void camera_apply_draw_offset(void) {
    int ox = (int) (SCREEN_WIDTH * 0.5f) - (int) floorf(camera.pos.x);
    int oy = (int) (SCREEN_HEIGHT * 0.5f) - (int) floorf(camera.pos.y);
    game_state.pd->graphics->setDrawOffset(ox, oy);
}
