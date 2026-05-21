#include "sprite_render.h"
#include "../ecs.h"
#include "../game_state.h"
#include "camera.h"
#include "pd_api.h"
#include <stdint.h>

void update_sprite_render(void) {
    const uint8_t        required_mask = COMP_SPRITE | COMP_TRANSFORM;
    const uint8_t*       masks         = ecs_get_masks_array();
    const c_transform_t* transforms    = ecs_get_transforms_array();
    const c_sprite_t*    sprites       = ecs_get_sprites_array();

    for (int i = 0; i < MAX_ENTITIES; i++) {
        if ((masks[i] & required_mask) != required_mask)
            continue;

        int sx, sy;
        camera_world_to_screen(transforms[i].x, transforms[i].y, &sx, &sy);

        game_state.pd->graphics->drawRotatedBitmap(
            sprites[i].gfx, sx, sy, transforms[i].heading_deg, 0.5f, 0.5f, 1.0f, 1.0f);
    }
}
