#include "sail_render.h"
#include "../core/math.h"
#include "../ecs.h"
#include "../game_state.h"
#include "camera.h"
#include "pd_api/pd_api_gfx.h"
#include "wind.h"
#include <stdint.h>

static void draw_luff_mark(const c_transform_t* transform, const c_sail_t* sail) {
    const float wind_side =
        shortest_angle_delta(transform->heading_deg, wind_get_state()->direction_deg);
    const float  sail_side       = wind_side < 0.0f ? -1.0f : 1.0f;
    const float  sail_angle_deg  = transform->heading_deg + sail->trim_deg * sail_side;
    const vec2_t sail_direction  = dir_from_angle(sail_angle_deg * DEG2RAD);
    const vec2_t cross_direction = (vec2_t) {-sail_direction.y, sail_direction.x};

    const vec2_t base   = vec2_add(transform->pos, vec2_scale(sail_direction, 12.0f));
    const vec2_t middle = vec2_add(vec2_add(base, vec2_scale(sail_direction, 3.0f)),
                                   vec2_scale(cross_direction, 2.0f));
    const vec2_t tip    = vec2_add(vec2_add(base, vec2_scale(sail_direction, 6.0f)),
                                vec2_scale(cross_direction, -2.0f));

    int base_x, base_y;
    int middle_x, middle_y;
    int tip_x, tip_y;
    camera_world_to_screen(base, &base_x, &base_y);
    camera_world_to_screen(middle, &middle_x, &middle_y);
    camera_world_to_screen(tip, &tip_x, &tip_y);

    game_state.pd->graphics->drawLine(base_x, base_y, middle_x, middle_y, 1, kColorXOR);
    game_state.pd->graphics->drawLine(middle_x, middle_y, tip_x, tip_y, 1, kColorXOR);
}

void sail_render_draw(void) {
    const uint8_t        required_mask = COMP_TRANSFORM | COMP_SAIL;
    const uint8_t*       entity_masks  = ecs_get_masks_array();
    const c_transform_t* transforms    = ecs_get_transforms_array();
    const c_sail_t*      sails         = ecs_get_sails_array();

    for (int i = 0; i < MAX_ENTITIES; i++) {
        if ((entity_masks[i] & required_mask) != required_mask)
            continue;
        if (!sails[i].is_luffing || ((game_state.frame_counter / 3 + i) & 1) != 0)
            continue;

        draw_luff_mark(&transforms[i], &sails[i]);
    }
}
