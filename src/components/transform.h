#pragma once
#include "../core/math.h"

typedef struct {
    vec2_t pos;
    float  heading_deg;
    float  wake_accumulator;
} c_transform_t;
