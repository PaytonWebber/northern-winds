#include "ecs.h"
#include <assert.h>
#include <stdint.h>

static uint8_t        s_entity_masks[MAX_ENTITIES];
static c_transform_t  s_transforms[MAX_ENTITIES];
static c_sprite_t     s_sprites[MAX_ENTITIES];
static c_ship_input_t s_ship_inputs[MAX_ENTITIES];
static uint8_t        s_free_stack[MAX_ENTITIES];
static int            s_free_top = -1;

/*
   Initializes the arrays, so s_free_stack will contain indices
   the following order: MAX_ENTITIES, MAX_ENTITIES-1, ..., 1, 0
   s_free_top will be set to: MAX_ENTITIES-1.
*/
void ecs_init(void) {
    s_free_top = -1;
    for (int i = MAX_ENTITIES - 1; i >= 0; i--) {
        s_free_stack[++s_free_top] = (uint8_t) i;
        s_entity_masks[i]          = COMP_NONE;
    }
}

int ecs_create_entity(void) {
    if (s_free_top < 0)
        return -1;
    return s_free_stack[s_free_top--];
}

void ecs_destroy_entity(int id) {
    s_entity_masks[id]         = COMP_NONE;
    s_free_stack[++s_free_top] = (uint8_t) id;
}

c_transform_t* ecs_add_transform(int id) {
    // assert id < MAX_ENTITIES
    s_entity_masks[id] |= COMP_TRANSFORM;
    return &s_transforms[id];
}

c_sprite_t* ecs_add_sprite(int id) {
    // assert id < MAX_ENTITIES
    s_entity_masks[id] |= COMP_SPRITE;
    s_sprites[id].entity_id = id;
    return &s_sprites[id];
}

void ecs_add_player_controlled(int id) {
    // assert id < MAX_ENTITIES
    s_entity_masks[id] |= COMP_PLAYER_CONTROLLED;
};

c_ship_input_t* ecs_add_ship_input(int id) {
    // assert id < MAX_ENTITIES
    s_entity_masks[id] |= COMP_SHIP_INPUT;
    return &s_ship_inputs[id];
}

const c_transform_t* ecs_get_transform_const(int id) {
    return &s_transforms[id];
}

const c_sprite_t* ecs_get_sprite_const(int id) {
    return &s_sprites[id];
}

const c_ship_input_t* ecs_get_ship_input_const(int id) {
    return &s_ship_inputs[id];
}

const uint8_t* ecs_get_masks_array(void) {
    return s_entity_masks;
}

c_transform_t* ecs_get_transforms_array(void) {
    return s_transforms;
}

c_sprite_t* ecs_get_sprites_array(void) {
    return s_sprites;
}

c_ship_input_t* ecs_get_ship_inputs_array(void) {
    return s_ship_inputs;
}
