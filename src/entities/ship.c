#include "ship.h"
#include "../ecs.h"
#include "../game_state.h"

int init_ship(float x, float y, const char* bitmap) {
    int            id = ecs.create_entity();
    c_transform_t* t  = ecs.add_transform(id);
    t->x              = x;
    t->y              = y;
    t->heading_deg    = 0;

    c_ship_physics_t* sp = ecs.add_ship_physics(id);
    sp->rudder_deg       = 0;

    c_sprite_t* s   = ecs.add_sprite(id);
    const char* err = NULL;
    s->gfx          = game_state.pd->graphics->loadBitmap(bitmap, &err);
    if (s->gfx == NULL) {
        game_state.pd->system->logToConsole("loadBitmap failed: %s", err);
        return -1;
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

    return id;
}
