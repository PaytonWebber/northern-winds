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

static inline float rand_unit(void) {
    return (float) rand() / (float) RAND_MAX; // [0, 1]
}

static inline float rand_signed(void) {
    return rand_unit() * 2.0f - 1.0f; // [-1, 1]
}
