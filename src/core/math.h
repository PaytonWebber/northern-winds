#pragma once
#include <math.h>
#include <stdlib.h>
#define PI      3.1415926535897932384626433832f
#define DEG2RAD (PI / 180.0f)

typedef struct {
    float x;
    float y;
} vec2_t;

static inline vec2_t dir_from_angle(float angle_rad) {
    return (vec2_t) {sinf(angle_rad), -cosf(angle_rad)};
}

static inline vec2_t vec2_scale(vec2_t v, float scale) {
    return (vec2_t) {v.x * scale, v.y * scale};
}

static inline vec2_t vec2_add(vec2_t a, vec2_t b) {
    return (vec2_t) {a.x + b.x, a.y + b.y};
}

static inline vec2_t vec2_sub(vec2_t a, vec2_t b) {
    return (vec2_t) {a.x - b.x, a.y - b.y};
}

static inline float vec2_dot(vec2_t a, vec2_t b) {
    return a.x * b.x + a.y * b.y;
}

static inline float clampf(float value, float min, float max) {
    if (value < min)
        return min;
    if (value > max)
        return max;
    return value;
}

static inline float wrap_degrees(float deg) {
    while (deg >= 360.0f)
        deg -= 360.0f;
    while (deg < 0.0f)
        deg += 360.0f;
    return deg;
}

static inline float shortest_angle_delta(float from_deg, float to_deg) {
    float delta = wrap_degrees(to_deg) - wrap_degrees(from_deg);
    if (delta > 180.0f)
        delta -= 360.0f;
    if (delta < -180.0f)
        delta += 360.0f;
    return delta;
}

static inline float rand_unit(void) {
    return (float) rand() / (float) RAND_MAX; // [0, 1]
}

static inline float rand_signed(void) {
    return rand_unit() * 2.0f - 1.0f; // [-1, 1]
}
