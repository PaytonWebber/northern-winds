#include "camera.h"
#include "../ecs.h"
#include "../game_state.h"
#include "pd_api.h"
#include <math.h>

static const float S_FOLLOW_RATE = 2.0f;

typedef struct {
    float x;
    float y;
    int   target_entity; // -1 for no follow
} camera_t;

static camera_t camera = {.x = 0, .y = 0, .target_entity = -1};

void camera_set_follow(int id) {
    if (id >= MAX_ENTITIES || id < 0)
        return;
    camera.target_entity = id;

    const c_transform_t* t = ecs_get_transform_const(id);
    camera.x               = t->x;
    camera.y               = t->y;
}

void camera_update_follow(float dt) {
    if (camera.target_entity < 0)
        return;
    const c_transform_t* t = ecs_get_transform_const(camera.target_entity);

    float k = 1.0f - expf(-S_FOLLOW_RATE * dt);
    camera.x += (t->x - camera.x) * k;
    camera.y += (t->y - camera.y) * k;
}

void camera_world_to_screen(float wx, float wy, int* sx, int* sy) {
    *sx = (int) floorf(wx - camera.x + SCREEN_WIDTH * 0.5f);
    *sy = (int) floorf(wy - camera.y + SCREEN_HEIGHT * 0.5f);
}
