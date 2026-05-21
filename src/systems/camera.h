#pragma once

#define SCREEN_WIDTH  400.0f
#define SCREEN_HEIGHT 240.0f

void camera_get_position(float* cx, float* cy);
void camera_set_follow(int entity_id);
void camera_update_follow(float dt);
void camera_world_to_screen(float wx, float wy, int* sx, int* sy);
