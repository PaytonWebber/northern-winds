#pragma once
#include "pd_api.h"

typedef struct {
    PlaydateAPI* pd;
    int          frame_counter;
} game_state_t;

extern game_state_t game_state;
void                set_playdate_api(PlaydateAPI* pd_api);
