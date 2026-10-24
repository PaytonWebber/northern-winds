#include "ship_sails.h"
#include "../core/math.h"
#include "../ecs.h"
#include "gust_lanes.h"
#include "wind.h"
#include <stdint.h>

typedef struct {
    float wind_angle_deg;
    float power;
    float ideal_trim_deg;
} sail_curve_sample_t;

static const sail_curve_sample_t S_SAIL_CURVE[] = {
    {  .wind_angle_deg = 0.0f, .power = 0.75f, .ideal_trim_deg = 75.0f},
    { .wind_angle_deg = 45.0f, .power = 0.90f, .ideal_trim_deg = 65.0f},
    { .wind_angle_deg = 90.0f, .power = 1.00f, .ideal_trim_deg = 45.0f},
    {.wind_angle_deg = 135.0f, .power = 0.80f, .ideal_trim_deg = 20.0f},
    {.wind_angle_deg = 155.0f, .power = 0.55f,  .ideal_trim_deg = 8.0f},
    {.wind_angle_deg = 170.0f, .power = 0.00f,  .ideal_trim_deg = 0.0f},
    {.wind_angle_deg = 180.0f, .power = 0.00f,  .ideal_trim_deg = 0.0f},
};

static const float S_SAIL_DRIVE_SPEED          = 26.0f;
static const float S_FULL_POWER_TRIM_ERROR_DEG = 7.0f;
static const float S_MAX_USEFUL_TRIM_ERROR_DEG = 45.0f;
static const float S_MIN_TRIM_EFFICIENCY       = 0.15f;
static const float S_LUFFING_TRIM_ERROR_DEG    = 15.0f;

static float lerp(float from, float to, float amount) {
    return from + (to - from) * amount;
}

static sail_curve_sample_t sample_sail_curve(float wind_angle_deg) {
    const int sample_count = (int) (sizeof(S_SAIL_CURVE) / sizeof(S_SAIL_CURVE[0]));

    for (int i = 1; i < sample_count; i++) {
        const sail_curve_sample_t previous = S_SAIL_CURVE[i - 1];
        const sail_curve_sample_t current  = S_SAIL_CURVE[i];
        if (wind_angle_deg > current.wind_angle_deg)
            continue;

        const float range  = current.wind_angle_deg - previous.wind_angle_deg;
        const float amount = (wind_angle_deg - previous.wind_angle_deg) / range;
        return (sail_curve_sample_t) {
            .wind_angle_deg = wind_angle_deg,
            .power          = lerp(previous.power, current.power, amount),
            .ideal_trim_deg = lerp(previous.ideal_trim_deg, current.ideal_trim_deg, amount),
        };
    }

    return S_SAIL_CURVE[sample_count - 1];
}

static float trim_efficiency_from_error(float error_deg) {
    if (error_deg <= S_FULL_POWER_TRIM_ERROR_DEG)
        return 1.0f;

    const float error_range = S_MAX_USEFUL_TRIM_ERROR_DEG - S_FULL_POWER_TRIM_ERROR_DEG;
    const float falloff =
        clampf((error_deg - S_FULL_POWER_TRIM_ERROR_DEG) / error_range, 0.0f, 1.0f);
    return lerp(1.0f, S_MIN_TRIM_EFFICIENCY, falloff);
}

void update_ship_sails(void) {
    const uint8_t         required_mask = COMP_TRANSFORM | COMP_SHIP_INPUT | COMP_SAIL;
    const uint8_t*        entity_masks  = ecs_get_masks_array();
    const c_transform_t*  transforms    = ecs_get_transforms_array();
    const c_ship_input_t* ship_inputs   = ecs_get_ship_inputs_array();
    c_sail_t*             sails         = ecs_get_sails_array();
    const wind_state_t*   wind          = wind_get_state();

    for (int i = 0; i < MAX_ENTITIES; i++) {
        if ((entity_masks[i] & required_mask) != required_mask)
            continue;

        const float heading_deg            = transforms[i].heading_deg;
        const float wind_side_deg          = shortest_angle_delta(heading_deg, wind->direction_deg);
        const float wind_angle_deg         = fabsf(wind_side_deg);
        const sail_curve_sample_t response = sample_sail_curve(wind_angle_deg);
        const float               trim_deg = ship_inputs[i].sail_trim_target_deg;
        const float               trim_error_deg  = fabsf(trim_deg - response.ideal_trim_deg);
        const float               trim_efficiency = trim_efficiency_from_error(trim_error_deg);
        const float wind_speed = wind->speed * gust_lanes_speed_multiplier_at(transforms[i].pos);

        sails[i].efficiency     = response.power * trim_efficiency;
        sails[i].drive_speed    = S_SAIL_DRIVE_SPEED * sails[i].efficiency * wind_speed;
        sails[i].wind_angle_deg = wind_angle_deg;
        sails[i].trim_deg       = trim_deg;
        sails[i].ideal_trim_deg = response.ideal_trim_deg;
        sails[i].trim_error_deg = trim_error_deg;
        sails[i].is_luffing = response.power <= 0.1f || trim_error_deg > S_LUFFING_TRIM_ERROR_DEG;
    }
}
