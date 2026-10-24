#include "hud.h"
#include "../core/math.h"
#include "../ecs.h"
#include "../game_state.h"
#include "pd_api/pd_api_gfx.h"
#include "wind.h"
#include <math.h>
#include <stdint.h>

static const int S_PANEL_X      = 284;
static const int S_PANEL_Y      = 8;
static const int S_PANEL_WIDTH  = 108;
static const int S_PANEL_HEIGHT = 60;
static const int S_DIVIDER_X    = 337;

static const int S_WIND_CENTER_X  = 310;
static const int S_SAIL_CENTER_X  = 365;
static const int S_INSTRUMENT_Y   = 42;
static const int S_COMPASS_RADIUS = 13;

static const int S_METER_Y              = 60;
static const int S_METER_SEGMENTS       = 5;
static const int S_METER_SEGMENT_WIDTH  = 6;
static const int S_METER_SEGMENT_HEIGHT = 4;
static const int S_METER_GAP            = 2;

static const float S_MAX_WIND_SPEED = 1.5f;

static int find_player_entity(void) {
    const uint8_t  required_mask = COMP_PLAYER_CONTROLLED | COMP_SAIL | COMP_TRANSFORM;
    const uint8_t* entity_masks  = ecs_get_masks_array();

    for (int i = 0; i < MAX_ENTITIES; i++) {
        if ((entity_masks[i] & required_mask) == required_mask)
            return i;
    }

    return -1;
}

static void draw_label(const char* text, int x, int y, size_t len) {
    game_state.pd->graphics->drawText(text, len, kASCIIEncoding, x, y);
}

static void draw_segment_meter(int x, float amount) {
    const float normalized      = clampf(amount, 0.0f, 1.0f);
    const int   filled_segments = (int) roundf(normalized * (float) S_METER_SEGMENTS);

    for (int i = 0; i < S_METER_SEGMENTS; i++) {
        const int segment_x = x + i * (S_METER_SEGMENT_WIDTH + S_METER_GAP);
        game_state.pd->graphics->drawRect(
            segment_x, S_METER_Y, S_METER_SEGMENT_WIDTH, S_METER_SEGMENT_HEIGHT, kColorBlack);
        if (i < filled_segments) {
            game_state.pd->graphics->fillRect(
                segment_x, S_METER_Y, S_METER_SEGMENT_WIDTH, S_METER_SEGMENT_HEIGHT, kColorBlack);
        }
    }
}

static void draw_direction_arrow(int x, int y, vec2_t direction, int length) {
    const int tip_x = x + (int) roundf(direction.x * (float) length);
    const int tip_y = y + (int) roundf(direction.y * (float) length);

    game_state.pd->graphics->drawLine(x, y, tip_x, tip_y, 2, kColorBlack);
    game_state.pd->graphics->fillRect(tip_x - 1, tip_y - 1, 3, 3, kColorBlack);
}

static void draw_wind_instrument(const wind_state_t* wind) {
    const int diameter = S_COMPASS_RADIUS * 2 + 1;
    const int left     = S_WIND_CENTER_X - S_COMPASS_RADIUS;
    const int top      = S_INSTRUMENT_Y - S_COMPASS_RADIUS;

    game_state.pd->graphics->drawEllipse(
        left, top, diameter, diameter, 1, 0.0f, 360.0f, kColorBlack);
    game_state.pd->graphics->drawLine(
        S_WIND_CENTER_X, top, S_WIND_CENTER_X, top + 3, 1, kColorBlack);
    draw_direction_arrow(
        S_WIND_CENTER_X, S_INSTRUMENT_Y, wind_get_direction(), S_COMPASS_RADIUS - 4);
    draw_segment_meter(S_PANEL_X + 8, wind->speed / S_MAX_WIND_SPEED);
}

static void draw_ship_hull(int x, int y, vec2_t forward) {
    const vec2_t starboard = (vec2_t) {-forward.y, forward.x};
    const vec2_t bow       = vec2_add((vec2_t) {(float) x, (float) y}, vec2_scale(forward, 11.0f));
    const vec2_t stern     = vec2_add((vec2_t) {(float) x, (float) y}, vec2_scale(forward, -9.0f));
    const vec2_t hull_mid  = vec2_add((vec2_t) {(float) x, (float) y}, vec2_scale(forward, -3.0f));
    const vec2_t port      = vec2_add(hull_mid, vec2_scale(starboard, -4.0f));
    const vec2_t starboard_edge = vec2_add(hull_mid, vec2_scale(starboard, 4.0f));

    game_state.pd->graphics->drawLine(
        (int) bow.x, (int) bow.y, (int) port.x, (int) port.y, 1, kColorBlack);
    game_state.pd->graphics->drawLine(
        (int) port.x, (int) port.y, (int) stern.x, (int) stern.y, 1, kColorBlack);
    game_state.pd->graphics->drawLine((int) stern.x,
                                      (int) stern.y,
                                      (int) starboard_edge.x,
                                      (int) starboard_edge.y,
                                      1,
                                      kColorBlack);
    game_state.pd->graphics->drawLine(
        (int) starboard_edge.x, (int) starboard_edge.y, (int) bow.x, (int) bow.y, 1, kColorBlack);
}

static void draw_sail_instrument(const c_transform_t* transform, const c_sail_t* sail) {
    const float  heading_deg = transform->heading_deg;
    const vec2_t forward     = dir_from_angle(heading_deg * DEG2RAD);
    const float  wind_side   = shortest_angle_delta(heading_deg, wind_get_state()->direction_deg);
    const float  sail_side   = wind_side < 0.0f ? -1.0f : 1.0f;
    const float  flutter_deg =
        sail->is_luffing ? (((game_state.frame_counter / 3) & 1) ? -3.0f : 3.0f) : 0.0f;
    const vec2_t sail_direction =
        dir_from_angle((heading_deg + (sail->trim_deg + flutter_deg) * sail_side) * DEG2RAD);
    const vec2_t ideal_direction =
        dir_from_angle((heading_deg + sail->ideal_trim_deg * sail_side) * DEG2RAD);
    const int ideal_x = S_SAIL_CENTER_X + (int) roundf(ideal_direction.x * 13.0f);
    const int ideal_y = S_INSTRUMENT_Y + (int) roundf(ideal_direction.y * 13.0f);

    draw_ship_hull(S_SAIL_CENTER_X, S_INSTRUMENT_Y, forward);
    game_state.pd->graphics->drawLine(S_SAIL_CENTER_X,
                                      S_INSTRUMENT_Y,
                                      S_SAIL_CENTER_X + (int) roundf(sail_direction.x * 10.0f),
                                      S_INSTRUMENT_Y + (int) roundf(sail_direction.y * 10.0f),
                                      2,
                                      kColorBlack);
    game_state.pd->graphics->fillRect(S_SAIL_CENTER_X - 1, S_INSTRUMENT_Y - 1, 3, 3, kColorBlack);
    game_state.pd->graphics->drawRect(ideal_x - 1, ideal_y - 1, 3, 3, kColorBlack);
    draw_segment_meter(S_DIVIDER_X + 8, sail->efficiency);
}

static void draw_panel(void) {
    game_state.pd->graphics->fillRect(
        S_PANEL_X + 2, S_PANEL_Y + 2, S_PANEL_WIDTH, S_PANEL_HEIGHT, kColorBlack);
    game_state.pd->graphics->fillRect(
        S_PANEL_X, S_PANEL_Y, S_PANEL_WIDTH, S_PANEL_HEIGHT, kColorWhite);
    game_state.pd->graphics->drawRect(
        S_PANEL_X, S_PANEL_Y, S_PANEL_WIDTH, S_PANEL_HEIGHT, kColorBlack);
    game_state.pd->graphics->drawLine(
        S_DIVIDER_X, S_PANEL_Y, S_DIVIDER_X, S_PANEL_Y + S_PANEL_HEIGHT - 1, 1, kColorBlack);

    draw_label("WIND", S_PANEL_X + 12, S_PANEL_Y + 4, 4);
    draw_label("SAIL", S_DIVIDER_X + 13, S_PANEL_Y + 4, 4);
}

void hud_draw(void) {
    const wind_state_t* wind      = wind_get_state();
    const int           player_id = find_player_entity();

    draw_panel();
    draw_wind_instrument(wind);

    if (player_id < 0) {
        draw_segment_meter(S_DIVIDER_X + 8, 0.0f);
        return;
    }

    draw_sail_instrument(ecs_get_transform_const(player_id), ecs_get_sail_const(player_id));
}
