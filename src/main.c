#include "ecs.h"
#include "game_state.h"
#include "pd_api.h"
#include "pd_api/pd_api_gfx.h"
#include "pd_api/pd_api_sprite.h"
#include "pd_api/pd_api_sys.h"
#include <stdint.h>
#include <stdio.h>

void init_ship(void) {
    int            id = ecs.create_entity();
    c_transform_t* t  = ecs.add_transform(id);
    t->x              = 220;
    t->y              = 100;
    t->heading_deg    = 0;

    c_ship_physics_t* sp = ecs.add_ship_physics(id);
    sp->rudder_deg       = 0;

    c_sprite_t* s   = ecs.add_sprite(id);
    const char* err = NULL;
    s->gfx =
        game_state.pd->graphics->loadBitmap("images/PNG/Default_size/Ships/dinghyLarge1", &err);
    if (s->gfx == NULL) {
        game_state.pd->system->logToConsole("loadBitmap failed: %s", err);
        return;
    }
    s->sprite = game_state.pd->sprite->newSprite();
    game_state.pd->sprite->setUserdata(s->sprite, s);
    game_state.pd->sprite->setImage(s->sprite, s->gfx, kBitmapUnflipped);
    game_state.pd->sprite->setDrawFunction(s->sprite, draw_ship_callback);
    game_state.pd->sprite->setBounds(s->sprite, (PDRect) {0, 0, 48, 48});
    game_state.pd->sprite->setZIndex(s->sprite, 10);
    game_state.pd->sprite->setCollisionsEnabled(s->sprite, 1);
    game_state.pd->sprite->setCollideRect(s->sprite, (PDRect) {8, 8, 32, 32});
    game_state.pd->sprite->addSprite(s->sprite);
    game_state.pd->sprite->moveTo(s->sprite, t->x, t->y);
}

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
            ecs.init_ecs();
            init_ship();
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
