#include "wind.h"
#include "../core/math.h"

static const float S_PREVAILING_DIRECTION_DEG = 180.0f;
static const float S_BASE_SPEED               = 1.0f;
static const float S_MIN_SPEED                = 0.6f;
static const float S_MAX_SPEED                = 1.5f;
static const float S_MAX_TARGET_SHIFT_DEG     = 18.0f;
static const float S_MAX_PREVAILING_SHIFT_DEG = 70.0f;
static const float S_MIN_DIRECTION_SECONDS    = 35.0f;
static const float S_MAX_DIRECTION_SECONDS    = 70.0f;
static const float S_MIN_SPEED_SECONDS        = 25.0f;
static const float S_MAX_SPEED_SECONDS        = 50.0f;
static const float S_MIN_GUST_WAIT_SECONDS    = 6.0f;
static const float S_MAX_GUST_WAIT_SECONDS    = 14.0f;
static const float S_MIN_GUST_DURATION        = 2.0f;
static const float S_MAX_GUST_DURATION        = 4.0f;
static const float S_DIRECTION_CHANGE_RATE    = 3.0f;
static const float S_SPEED_CHANGE_RATE        = 0.04f;
static const float S_GUST_SPEED_CHANGE_RATE   = 0.18f;
static const float S_GUST_DIRECTION_RATE      = 3.0f;

static wind_state_t s_wind;
static float        s_base_direction_deg;
static float        s_base_speed;
static float        s_gust_speed_offset;
static float        s_gust_direction_offset_deg;
static float        s_target_gust_speed_offset;
static float        s_target_gust_direction_offset_deg;
static float        s_direction_timer;
static float        s_speed_timer;
static float        s_gust_timer;
static int          s_gust_active;

static float random_range(float min, float max) {
    return min + rand_unit() * (max - min);
}

static float clamp_to_prevailing(float direction_deg) {
    float offset = shortest_angle_delta(S_PREVAILING_DIRECTION_DEG, direction_deg);
    offset       = clampf(offset, -S_MAX_PREVAILING_SHIFT_DEG, S_MAX_PREVAILING_SHIFT_DEG);
    return wrap_degrees(S_PREVAILING_DIRECTION_DEG + offset);
}

static void pick_direction_target(void) {
    s_wind.target_direction_deg =
        clamp_to_prevailing(s_wind.target_direction_deg + rand_signed() * S_MAX_TARGET_SHIFT_DEG);
    s_direction_timer = random_range(S_MIN_DIRECTION_SECONDS, S_MAX_DIRECTION_SECONDS);
}

static void pick_speed_target(void) {
    s_wind.target_speed =
        clampf(s_wind.target_speed + rand_signed() * 0.15f, S_MIN_SPEED, S_MAX_SPEED);
    s_speed_timer = random_range(S_MIN_SPEED_SECONDS, S_MAX_SPEED_SECONDS);
}

static void start_gust(void) {
    s_target_gust_speed_offset         = random_range(-0.12f, 0.22f);
    s_target_gust_direction_offset_deg = random_range(-4.0f, 4.0f);
    s_gust_timer                       = random_range(S_MIN_GUST_DURATION, S_MAX_GUST_DURATION);
    s_gust_active                      = 1;
}

static void finish_gust(void) {
    s_target_gust_speed_offset         = 0.0f;
    s_target_gust_direction_offset_deg = 0.0f;
    s_gust_timer  = random_range(S_MIN_GUST_WAIT_SECONDS, S_MAX_GUST_WAIT_SECONDS);
    s_gust_active = 0;
}

static float approach(float current, float target, float max_delta) {
    if (current < target)
        return clampf(current + max_delta, current, target);
    return clampf(current - max_delta, target, current);
}

static float approach_angle(float current_deg, float target_deg, float max_delta) {
    const float delta = shortest_angle_delta(current_deg, target_deg);
    return wrap_degrees(current_deg + clampf(delta, -max_delta, max_delta));
}

void wind_init(void) {
    s_wind.direction_deg               = S_PREVAILING_DIRECTION_DEG;
    s_wind.speed                       = S_BASE_SPEED;
    s_wind.target_direction_deg        = S_PREVAILING_DIRECTION_DEG;
    s_wind.target_speed                = S_BASE_SPEED;
    s_base_direction_deg               = S_PREVAILING_DIRECTION_DEG;
    s_base_speed                       = S_BASE_SPEED;
    s_gust_speed_offset                = 0.0f;
    s_gust_direction_offset_deg        = 0.0f;
    s_target_gust_speed_offset         = 0.0f;
    s_target_gust_direction_offset_deg = 0.0f;
    s_direction_timer = random_range(S_MIN_DIRECTION_SECONDS, S_MAX_DIRECTION_SECONDS);
    s_speed_timer     = random_range(S_MIN_SPEED_SECONDS, S_MAX_SPEED_SECONDS);
    s_gust_timer      = random_range(S_MIN_GUST_WAIT_SECONDS, S_MAX_GUST_WAIT_SECONDS);
    s_gust_active     = 0;
}

void wind_update(float dt) {
    s_direction_timer -= dt;
    if (s_direction_timer <= 0.0f)
        pick_direction_target();

    s_speed_timer -= dt;
    if (s_speed_timer <= 0.0f)
        pick_speed_target();

    s_gust_timer -= dt;
    if (s_gust_timer <= 0.0f) {
        if (s_gust_active)
            finish_gust();
        else
            start_gust();
    }

    s_base_direction_deg = approach_angle(
        s_base_direction_deg, s_wind.target_direction_deg, S_DIRECTION_CHANGE_RATE * dt);
    s_base_speed = approach(s_base_speed, s_wind.target_speed, S_SPEED_CHANGE_RATE * dt);
    s_gust_speed_offset =
        approach(s_gust_speed_offset, s_target_gust_speed_offset, S_GUST_SPEED_CHANGE_RATE * dt);
    s_gust_direction_offset_deg = approach(s_gust_direction_offset_deg,
                                           s_target_gust_direction_offset_deg,
                                           S_GUST_DIRECTION_RATE * dt);

    s_wind.direction_deg = wrap_degrees(s_base_direction_deg + s_gust_direction_offset_deg);
    s_wind.speed         = clampf(s_base_speed + s_gust_speed_offset, S_MIN_SPEED, S_MAX_SPEED);
}

const wind_state_t* wind_get_state(void) {
    return &s_wind;
}

vec2_t wind_get_direction(void) {
    return dir_from_angle(s_wind.direction_deg * DEG2RAD);
}
