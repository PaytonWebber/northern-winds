#pragma once
#include "../core/math.h"

#define MAX_PARTICLES 2048

void particles_spawn(vec2_t pos, vec2_t vel, float life);
void particles_update(float dt);
void particles_draw(void);
