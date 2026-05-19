#include "ecs.h"
#include "entities/ship.h"
#include "game_state.h"
#include "pd_api.h"
#include "pd_api/pd_api_gfx.h"
#include "pd_api/pd_api_sprite.h"
#include "pd_api/pd_api_sys.h"
#include "systems/ship_movement.h"
#include <stdint.h>
#include <stdio.h>

static int update(void* userdata) {
    update_ship_movement();
    game_state.pd->graphics->clear(kColorWhite);
    game_state.pd->sprite->updateAndDrawSprites();
    game_state.pd->system->drawFPS(10, 10);
    return 1; // 1 = redraw screen
}

int eventHandler(PlaydateAPI* playdate, PDSystemEvent event, uint32_t arg) {
    switch (event) {
        case kEventInit:
            set_playdate_api(playdate);
            ecs_init();
            init_ship(220, 110, "images/PNG/Default_size/Ships/dinghyLarge1");
            game_state.pd->system->setUpdateCallback(update, NULL);
            break;
        case kEventUnlock:
            game_state.pd->system->logToConsole("Unlocked\n");
            break;
        default:
            break;
    };
    return 1;
}
