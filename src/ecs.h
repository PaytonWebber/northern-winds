#pragma once
#include "components/ship_physics.h"
#include "components/sprite.h"
#include "components/transform.h"
#include "entities/ship.h"
#include "systems/render.h"
#include "systems/ship_movement.h"
#include <stdint.h>

#define MAX_ENTITIES 128

#define COMP_NONE         0
#define COMP_TRANSFORM    (1 << 0)
#define COMP_SPRITE       (1 << 1)
#define COMP_SHIP_PHYSICS (1 << 2)

typedef struct {
    void (*init_ecs)(void);
    int (*create_entity)(void);
    void (*destroy_entity)(int entity_id);

    c_transform_t* (*add_transform)(int entity_id);
    c_sprite_t* (*add_sprite)(int entity_id);
    c_ship_physics_t* (*add_ship_physics)(int entity_id);

    const c_transform_t* (*get_transform_const)(int entity_id);
    const c_sprite_t* (*get_sprite_const)(int entity_id);
    const c_ship_physics_t* (*get_ship_physics_const)(int entity_id);

    const uint8_t* (*get_masks_array)(void);
    c_transform_t* (*get_transforms_array)(void);
    c_sprite_t* (*get_sprites_array)(void);
    c_ship_physics_t* (*get_ship_physics_array)(void);
} ecs_api_t;

extern const ecs_api_t ecs;
