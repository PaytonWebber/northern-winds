#include "render.h"
#include "../ecs.h"
#include "../game_state.h"

void draw_ship_callback(LCDSprite* sprite, PDRect bounds, PDRect drawrect) {
    c_sprite_t*          sprite_comp    = game_state.pd->sprite->getUserdata(sprite);
    const c_transform_t* transform_comp = ecs.get_transform_const(sprite_comp->entity_id);
    game_state.pd->graphics->drawRotatedBitmap(sprite_comp->gfx,
                                               transform_comp->x,
                                               transform_comp->y,
                                               transform_comp->heading_deg,
                                               0.5f,
                                               0.5f,
                                               1.0f,
                                               1.0f);
}
