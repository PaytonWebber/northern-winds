#include "ship_input.h"
#include "../core/math.h"
#include "../ecs.h"
#include "../game_state.h"
#include "pd_api/pd_api_sys.h"
#include <stdint.h>

static const float S_MAX_RUDDER_DEG         = 45.0f;
static const float S_MAX_SAIL_TRIM_DEG      = 75.0f;
static const float S_SAIL_CRANK_SENSITIVITY = 0.25f;

static float rudder_target_from_buttons(PDButtons held) {
    const int steer_left  = (held & kButtonLeft) != 0;
    const int steer_right = (held & kButtonRight) != 0;

    if (steer_left == steer_right)
        return 0.0f;
    return steer_left ? -S_MAX_RUDDER_DEG : S_MAX_RUDDER_DEG;
}

// TODO: break this up into 'player' controlled and 'ai' controlled helper functions.
void update_ship_input(void) {
    const uint8_t   required_mask = COMP_SHIP_INPUT | COMP_PLAYER_CONTROLLED;
    const uint8_t*  entity_masks  = ecs_get_masks_array();
    c_ship_input_t* ship_input    = ecs_get_ship_inputs_array();

    PDButtons held   = 0;
    PDButtons pushed = 0;
    game_state.pd->system->getButtonState(&held, &pushed, NULL);

    const float rudder_target = rudder_target_from_buttons(held);
    const float sail_trim_change =
        game_state.pd->system->getCrankChange() * S_SAIL_CRANK_SENSITIVITY;

    for (int i = 0; i < MAX_ENTITIES; i++) {
        if ((entity_masks[i] & required_mask) != required_mask)
            continue;

        ship_input[i].rudder_target_deg    = rudder_target;
        ship_input[i].sail_trim_target_deg = clampf(
            ship_input[i].sail_trim_target_deg + sail_trim_change, 0.0f, S_MAX_SAIL_TRIM_DEG);

        if (kButtonA & pushed)
            ship_input[i].fire_starboard = 1;
        if (kButtonB & pushed)
            ship_input[i].fire_port = 1;
    }
}
