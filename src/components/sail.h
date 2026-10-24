#pragma once

typedef struct {
    float efficiency;
    float drive_speed;
    float wind_angle_deg;
    float trim_deg;
    float ideal_trim_deg;
    float trim_error_deg;
    int   is_luffing;
} c_sail_t;
