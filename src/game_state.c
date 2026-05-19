#include "game_state.h"
#include "pd_api.h"

game_state_t game_state = {0};

void set_playdate_api(PlaydateAPI* pd_api) {
    game_state.pd = pd_api;
}
