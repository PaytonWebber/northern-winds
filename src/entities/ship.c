#include "ship.h"
#include "../ecs.h"
#include "../game_state.h"

int init_ship(vec2_t pos, float heading_deg, const char* bitmap) {
    int            id = ecs_create_entity();
    c_transform_t* t  = ecs_add_transform(id);
    *t                = (c_transform_t) {0};
    t->pos            = pos;
    t->heading_deg    = heading_deg;

    c_ship_input_t* si       = ecs_add_ship_input(id);
    *si                      = (c_ship_input_t) {0};
    si->sail_trim_target_deg = 45.0f;

    c_sail_t* sail = ecs_add_sail(id);
    *sail          = (c_sail_t) {0};

    c_ship_motion_t* motion = ecs_add_ship_motion(id);
    *motion                 = (c_ship_motion_t) {0};

    c_sprite_t* s   = ecs_add_sprite(id);
    *s              = (c_sprite_t) {0};
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
