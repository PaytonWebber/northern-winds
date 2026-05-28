#include "ship.h"
#include "../ecs.h"
#include "../game_state.h"

int init_ship(vec2_t pos, const char* bitmap) {
    int            id   = ecs_create_entity();
    c_transform_t* t    = ecs_add_transform(id);
    t->pos              = pos;
    t->heading_deg      = 0;
    t->wake_accumulator = 0;

    c_ship_input_t* si    = ecs_add_ship_input(id);
    si->rudder_target_deg = 0;

    c_sprite_t* s   = ecs_add_sprite(id);
    const char* err = NULL;
    s->gfx          = game_state.pd->graphics->loadBitmap(bitmap, &err);
    if (s->gfx == NULL) {
        game_state.pd->system->logToConsole("loadBitmap failed: %s", err);
        return -1;
    }
    s->sprite = game_state.pd->sprite->newSprite();
    game_state.pd->sprite->setBounds(s->sprite, (PDRect) {0, 0, 48, 48});
    game_state.pd->sprite->setCollisionsEnabled(s->sprite, 1);
    game_state.pd->sprite->setCollideRect(s->sprite, (PDRect) {8, 8, 32, 32});
    game_state.pd->sprite->addSprite(s->sprite);
    game_state.pd->sprite->moveTo(s->sprite, t->pos.x, t->pos.y);

    return id;
}
