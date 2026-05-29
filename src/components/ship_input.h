#pragma once

typedef struct {
    float rudder_target_deg;
    float fire_delay;
    int   fire_starboard;
    int   fire_port;
} c_ship_input_t;
