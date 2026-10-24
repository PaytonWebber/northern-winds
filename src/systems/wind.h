#pragma once
#include "../core/math.h"

typedef struct {
    float direction_deg;
    float speed;
    float target_direction_deg;
    float target_speed;
} wind_state_t;

void wind_init(void);
void wind_update(float dt);
const wind_state_t* wind_get_state(void);
vec2_t wind_get_direction(void);
