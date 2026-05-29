#pragma once
#include "../core/math.h"

#define MAX_CANNONBALLS 100

void cannonballs_fire(vec2_t pos, vec2_t dir, int owner_id);
void cannonballs_update(float dt);
void cannonballs_draw(void);
