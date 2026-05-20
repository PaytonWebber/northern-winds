#pragma once
#include "components/ship_input.h"
#include "components/sprite.h"
#include "components/transform.h"
#include <stdint.h>

#define MAX_ENTITIES 128

#define COMP_NONE              0
#define COMP_TRANSFORM         (1 << 0)
#define COMP_SPRITE            (1 << 1)
#define COMP_SHIP_INPUT        (1 << 2)
#define COMP_PLAYER_CONTROLLED (1 << 3)

void ecs_init(void);
int  ecs_create_entity(void);
void ecs_destroy_entity(int entity_id);

c_transform_t*  ecs_add_transform(int entity_id);
c_sprite_t*     ecs_add_sprite(int entity_id);
c_ship_input_t* ecs_add_ship_input(int entity_id);
void            ecs_add_player_controlled(int entity_id);

const c_transform_t*  ecs_get_transform_const(int entity_id);
const c_sprite_t*     ecs_get_sprite_const(int entity_id);
const c_ship_input_t* ecs_get_ship_input_const(int entity_id);

const uint8_t*  ecs_get_masks_array(void);
c_transform_t*  ecs_get_transforms_array(void);
c_sprite_t*     ecs_get_sprites_array(void);
c_ship_input_t* ecs_get_ship_inputs_array(void);
