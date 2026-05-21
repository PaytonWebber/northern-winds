#include "background_render.h"
#include "../ecs.h"
#include "../game_state.h"
#include "camera.h"
#include "pd_api/pd_api_gfx.h"

static const uint8_t ocean_dither[16] = {
    0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

void draw_ocean_background(void) {
    float cx, cy;
    camera_get_position(&cx, &cy);
    int shift_x = (int) cx % 8;
    int shift_y = (int) cy % 8;
    if (shift_x < 0)
        shift_x += 8;
    if (shift_y < 0)
        shift_y += 8;

    game_state.pd->graphics->setDrawOffset(-shift_x, -shift_y);
    game_state.pd->graphics->fillRect(0, 0, 416, 256, (LCDColor) ocean_dither);
    game_state.pd->graphics->setDrawOffset(0, 0);
}
