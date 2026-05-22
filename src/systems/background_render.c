#include "background_render.h"
#include "../ecs.h"
#include "../game_state.h"
#include "camera.h"
#include "pd_api/pd_api_gfx.h"
#include <math.h>

static const uint8_t ocean_dither[16] = {
    0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

void update_background_render(void) {
    float cx, cy;
    camera_get_position(&cx, &cy);

    // https://stackoverflow.com/questions/14997165/fastest-way-to-get-a-positive-modulo-in-c-c
    int shift_x = (((int) floorf(cx) % 8) + 8) % 8;
    int shift_y = (((int) floorf(cy) % 8) + 8) % 8;

    game_state.pd->graphics->setDrawOffset(-shift_x, -shift_y);
    game_state.pd->graphics->fillRect(
        0, 0, (int) SCREEN_WIDTH + 16, (int) SCREEN_HEIGHT + 16, (LCDColor) ocean_dither);
    game_state.pd->graphics->setDrawOffset(0, 0);
}
