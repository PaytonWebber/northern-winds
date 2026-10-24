#pragma once
#include "../core/math.h"

void  gust_lanes_init(vec2_t origin);
void  gust_lanes_update(float dt);
float gust_lanes_speed_multiplier_at(vec2_t world_pos);
void  gust_lanes_draw(void);
