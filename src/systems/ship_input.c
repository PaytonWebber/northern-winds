#include "ship_input.h"
#include "../ecs.h"
#include "../game_state.h"
#include "pd_api/pd_api_sys.h"
#include <stdint.h>

static const float S_RUDDER_SENSITIVITY = 0.1f; // deg of target per deg of crank

void update_ship_input(void) {
    const uint8_t   required_mask = COMP_SHIP_INPUT | COMP_PLAYER_CONTROLLED;
    const uint8_t*  entity_masks  = ecs_get_masks_array();
    c_ship_input_t* ship_input    = ecs_get_ship_inputs_array();

    PDButtons pushed = 0;
    game_state.pd->system->getButtonState(NULL, &pushed, NULL);
    for (int i = 0; i < MAX_ENTITIES; i++) {
        if ((entity_masks[i] & required_mask) != required_mask)
            continue;
        // Crank delta is per-frame from the SDK, so no dt scaling here.
        ship_input[i].rudder_target_deg +=
            game_state.pd->system->getCrankChange() * S_RUDDER_SENSITIVITY;
        if (ship_input[i].rudder_target_deg > 45.0f)
            ship_input[i].rudder_target_deg = 45.0f;
        if (ship_input[i].rudder_target_deg < -45.0f)
            ship_input[i].rudder_target_deg = -45.0f;

        if (kButtonA & pushed)
            ship_input[i].fire_starboard = 1;
        if (kButtonB & pushed)
            ship_input[i].fire_port = 1;
    }
}
