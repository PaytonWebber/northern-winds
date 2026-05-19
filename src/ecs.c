#include "ecs.h"
#include <assert.h>
#include <stdint.h>

static uint8_t          s_entity_masks[MAX_ENTITIES];
static c_transform_t    s_transforms[MAX_ENTITIES];
static c_sprite_t       s_sprites[MAX_ENTITIES];
static c_ship_physics_t s_ship_physics[MAX_ENTITIES];
static uint8_t          s_free_stack[MAX_ENTITIES];
static int              s_free_top = -1;

/*
   Initializes the arrays, so s_free_stack will contain indices
   the following order: MAX_ENTITIES, MAX_ENTITIES-1, ..., 1, 0
   s_free_top will be set to: MAX_ENTITIES-1.
*/
static void init_ecs(void) {
    s_free_top = -1;
    for (int i = MAX_ENTITIES - 1; i >= 0; i--) {
        s_free_stack[++s_free_top] = i;
        s_entity_masks[i]          = COMP_NONE;
    }
}

static int create_entity(void) {
    return s_free_stack[s_free_top--];
}

static void destroy_entity(int id) {
    s_entity_masks[id]         = COMP_NONE;
    s_free_stack[++s_free_top] = id;
};

static c_transform_t* add_transform(int id) {
    // assert id < MAX_ENTITIES
    s_entity_masks[id] |= COMP_TRANSFORM;
    return &s_transforms[id];
}

static c_sprite_t* add_sprite(int id) {
    // assert id < MAX_ENTITIES
    s_entity_masks[id] |= COMP_SPRITE;
    s_sprites[id].entity_id = id;
    return &s_sprites[id];
}

static c_ship_physics_t* add_ship_physics(int id) {
    // assert id < MAX_ENTITIES
    s_entity_masks[id] |= COMP_SHIP_PHYSICS;
    return &s_ship_physics[id];
}

static const c_transform_t* get_transform_const(int id) {
    return &s_transforms[id];
}

static const c_sprite_t* get_sprite_const(int id) {
    return &s_sprites[id];
}

static const c_ship_physics_t* get_ship_physics_const(int id) {
    return &s_ship_physics[id];
}

static const uint8_t* get_masks_array(void) {
    return s_entity_masks;
}

static c_transform_t* get_transforms_array(void) {
    return s_transforms;
}

static c_sprite_t* get_sprites_array(void) {
    return s_sprites;
}

static c_ship_physics_t* get_ship_physics_array(void) {
    return s_ship_physics;
}

const ecs_api_t ecs = {.init_ecs               = init_ecs,
                       .create_entity          = create_entity,
                       .destroy_entity         = destroy_entity,
                       .add_transform          = add_transform,
                       .add_sprite             = add_sprite,
                       .add_ship_physics       = add_ship_physics,
                       .get_transform_const    = get_transform_const,
                       .get_sprite_const       = get_sprite_const,
                       .get_ship_physics_const = get_ship_physics_const,
                       .get_transforms_array   = get_transforms_array,
                       .get_sprites_array      = get_sprites_array,
                       .get_ship_physics_array = get_ship_physics_array,
                       .get_masks_array        = get_masks_array};
