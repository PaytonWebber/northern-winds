#pragma once
#include "../core/math.h"

#define SCREEN_WIDTH  400.0f
#define SCREEN_HEIGHT 240.0f

vec2_t camera_get_position(void);
void   camera_set_follow(int entity_id);
void   camera_update_follow(float dt);
void   camera_world_to_screen(vec2_t world_pos, int* sx, int* sy);
void   camera_apply_draw_offset(void);
