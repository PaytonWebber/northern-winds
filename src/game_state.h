#include "pd_api.h"
#include "pd_api/pd_api_gfx.h"

typedef struct {
    PlaydateAPI* pd;
} game_state_t;

extern game_state_t game_state;
void                set_playdate_api(PlaydateAPI* pd_api);
