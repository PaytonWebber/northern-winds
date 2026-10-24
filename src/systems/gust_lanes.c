#include "gust_lanes.h"
#include "../game_state.h"
#include "camera.h"
#include "pd_api/pd_api_gfx.h"
#include "wind.h"
#include <math.h>

#define MAX_GUST_LANES 3

typedef struct {
    vec2_t center;
    float  half_width;
    float  half_length;
    float  speed_bonus;
} gust_lane_t;

static const float S_MIN_HALF_WIDTH      = 26.0f;
static const float S_MAX_HALF_WIDTH      = 38.0f;
static const float S_MIN_HALF_LENGTH     = 150.0f;
static const float S_MAX_HALF_LENGTH     = 210.0f;
static const float S_MIN_SPEED_BONUS     = 0.20f;
static const float S_MAX_SPEED_BONUS     = 0.28f;
static const float S_LANE_DRIFT_SPEED    = 6.0f;
static const float S_RECYCLE_DISTANCE    = 650.0f;
static const float S_RECYCLE_BEHIND      = 300.0f;
static const float S_SPAWN_AHEAD_MIN     = 240.0f;
static const float S_SPAWN_AHEAD_MAX     = 420.0f;
static const float S_SPAWN_SIDE_DISTANCE = 210.0f;
static const float S_EDGE_FADE_WIDTH     = 12.0f;
static const float S_EDGE_FADE_LENGTH    = 24.0f;
static const float S_STREAK_SPACING      = 30.0f;
static const float S_STREAK_LENGTH       = 12.0f;
static const float S_STREAK_FLOW_SPEED   = 30.0f;
static const int   S_STREAK_COLUMNS      = 4;

static const LCDPattern S_GUST_WATER_PATTERN = {
    0x77, 0xDD, 0x77, 0xDD, 0x77, 0xDD, 0x77, 0xDD, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

static gust_lane_t s_lanes[MAX_GUST_LANES];
static vec2_t      s_last_camera_pos;
static vec2_t      s_travel_direction = {.x = 1.0f, .y = 0.0f};
static float       s_streak_offset;

static float random_range(float min, float max) {
    return min + rand_unit() * (max - min);
}

static vec2_t perpendicular(vec2_t direction) {
    return (vec2_t) {-direction.y, direction.x};
}

static void configure_lane(gust_lane_t* lane, vec2_t center) {
    lane->center      = center;
    lane->half_width  = random_range(S_MIN_HALF_WIDTH, S_MAX_HALF_WIDTH);
    lane->half_length = random_range(S_MIN_HALF_LENGTH, S_MAX_HALF_LENGTH);
    lane->speed_bonus = random_range(S_MIN_SPEED_BONUS, S_MAX_SPEED_BONUS);
}

static void recycle_lane(gust_lane_t* lane, vec2_t camera_pos) {
    const vec2_t side        = perpendicular(s_travel_direction);
    const float  ahead       = random_range(S_SPAWN_AHEAD_MIN, S_SPAWN_AHEAD_MAX);
    const float  side_offset = random_range(-S_SPAWN_SIDE_DISTANCE, S_SPAWN_SIDE_DISTANCE);
    const vec2_t center      = vec2_add(
        camera_pos, vec2_add(vec2_scale(s_travel_direction, ahead), vec2_scale(side, side_offset)));
    configure_lane(lane, center);
}

void gust_lanes_init(vec2_t origin) {
    const vec2_t side = perpendicular(s_travel_direction);

    configure_lane(&s_lanes[0], vec2_add(origin, vec2_scale(s_travel_direction, 50.0f)));
    configure_lane(
        &s_lanes[1],
        vec2_add(origin,
                 vec2_add(vec2_scale(s_travel_direction, 260.0f), vec2_scale(side, -170.0f))));
    configure_lane(
        &s_lanes[2],
        vec2_add(origin,
                 vec2_add(vec2_scale(s_travel_direction, 440.0f), vec2_scale(side, 130.0f))));

    s_last_camera_pos = origin;
    s_streak_offset   = 0.0f;
}

void gust_lanes_update(float dt) {
    const vec2_t camera_pos   = camera_get_position();
    const vec2_t camera_delta = vec2_sub(camera_pos, s_last_camera_pos);
    const float  camera_moved = hypotf(camera_delta.x, camera_delta.y);

    if (camera_moved > 0.01f)
        s_travel_direction = vec2_scale(camera_delta, 1.0f / camera_moved);
    s_last_camera_pos = camera_pos;

    const vec2_t lane_velocity = vec2_scale(wind_get_direction(), S_LANE_DRIFT_SPEED * dt);
    for (int i = 0; i < MAX_GUST_LANES; i++) {
        s_lanes[i].center = vec2_add(s_lanes[i].center, lane_velocity);

        const vec2_t relative       = vec2_sub(s_lanes[i].center, camera_pos);
        const float  distance       = hypotf(relative.x, relative.y);
        const float  distance_ahead = vec2_dot(relative, s_travel_direction);
        if (distance > S_RECYCLE_DISTANCE || distance_ahead < -S_RECYCLE_BEHIND)
            recycle_lane(&s_lanes[i], camera_pos);
    }

    s_streak_offset = fmodf(s_streak_offset + S_STREAK_FLOW_SPEED * dt, S_STREAK_SPACING);
}

static float lane_strength_at(const gust_lane_t* lane, vec2_t world_pos) {
    const vec2_t lane_direction  = wind_get_direction();
    const vec2_t cross_direction = perpendicular(lane_direction);
    const vec2_t relative        = vec2_sub(world_pos, lane->center);
    const float  along           = fabsf(vec2_dot(relative, lane_direction));
    const float  across          = fabsf(vec2_dot(relative, cross_direction));

    if (along >= lane->half_length || across >= lane->half_width)
        return 0.0f;

    const float width_strength =
        clampf((lane->half_width - across) / S_EDGE_FADE_WIDTH, 0.0f, 1.0f);
    const float length_strength =
        clampf((lane->half_length - along) / S_EDGE_FADE_LENGTH, 0.0f, 1.0f);
    return fminf(width_strength, length_strength);
}

float gust_lanes_speed_multiplier_at(vec2_t world_pos) {
    float multiplier = 1.0f;

    for (int i = 0; i < MAX_GUST_LANES; i++) {
        const float lane_multiplier =
            1.0f + s_lanes[i].speed_bonus * lane_strength_at(&s_lanes[i], world_pos);
        multiplier = fmaxf(multiplier, lane_multiplier);
    }

    return multiplier;
}

static void draw_streak(vec2_t start, vec2_t lane_direction) {
    const vec2_t end = vec2_add(start, vec2_scale(lane_direction, S_STREAK_LENGTH));
    int          start_x, start_y;
    int          end_x, end_y;
    camera_world_to_screen(start, &start_x, &start_y);
    camera_world_to_screen(end, &end_x, &end_y);

    const int margin = 12;
    if ((start_x < -margin && end_x < -margin) ||
        (start_x > (int) SCREEN_WIDTH + margin && end_x > (int) SCREEN_WIDTH + margin) ||
        (start_y < -margin && end_y < -margin) ||
        (start_y > (int) SCREEN_HEIGHT + margin && end_y > (int) SCREEN_HEIGHT + margin)) {
        return;
    }

    game_state.pd->graphics->drawLine(start_x, start_y, end_x, end_y, 2, kColorBlack);
    game_state.pd->graphics->drawLine(start_x, start_y, end_x, end_y, 1, kColorWhite);
}

static void fill_lane(const gust_lane_t* lane, vec2_t lane_direction, vec2_t cross_direction) {
    const vec2_t along      = vec2_scale(lane_direction, lane->half_length);
    const vec2_t across     = vec2_scale(cross_direction, lane->half_width);
    const vec2_t corners[4] = {
        vec2_sub(vec2_sub(lane->center, along), across),
        vec2_add(vec2_sub(lane->center, across), along),
        vec2_add(vec2_add(lane->center, along), across),
        vec2_sub(vec2_add(lane->center, across), along),
    };
    int coordinates[8];
    int min_x, max_x;
    int min_y, max_y;

    for (int i = 0; i < 4; i++) {
        camera_world_to_screen(corners[i], &coordinates[i * 2], &coordinates[i * 2 + 1]);
        if (i == 0) {
            min_x = max_x = coordinates[0];
            min_y = max_y = coordinates[1];
            continue;
        }
        min_x = coordinates[i * 2] < min_x ? coordinates[i * 2] : min_x;
        max_x = coordinates[i * 2] > max_x ? coordinates[i * 2] : max_x;
        min_y = coordinates[i * 2 + 1] < min_y ? coordinates[i * 2 + 1] : min_y;
        max_y = coordinates[i * 2 + 1] > max_y ? coordinates[i * 2 + 1] : max_y;
    }

    if (max_x < 0 || min_x >= (int) SCREEN_WIDTH || max_y < 0 || min_y >= (int) SCREEN_HEIGHT)
        return;

    game_state.pd->graphics->fillPolygon(
        4, coordinates, (LCDColor) S_GUST_WATER_PATTERN, kPolygonFillNonZero);
}

static void draw_lane(const gust_lane_t* lane, int lane_index) {
    const vec2_t lane_direction  = wind_get_direction();
    const vec2_t cross_direction = perpendicular(lane_direction);
    const float  usable_width    = lane->half_width - 5.0f;

    fill_lane(lane, lane_direction, cross_direction);

    for (int column = 0; column < S_STREAK_COLUMNS; column++) {
        const float column_amount = (float) column / (float) (S_STREAK_COLUMNS - 1);
        const float cross_offset  = -usable_width + column_amount * usable_width * 2.0f;
        const float column_offset = fmodf((float) (column * 7 + lane_index * 11), S_STREAK_SPACING);
        const float first_along   = -lane->half_length + s_streak_offset + column_offset;

        for (float along = first_along; along < lane->half_length; along += S_STREAK_SPACING) {
            const vec2_t start = vec2_add(lane->center,
                                          vec2_add(vec2_scale(cross_direction, cross_offset),
                                                   vec2_scale(lane_direction, along)));
            draw_streak(start, lane_direction);
        }
    }
}

void gust_lanes_draw(void) {
    for (int i = 0; i < MAX_GUST_LANES; i++)
        draw_lane(&s_lanes[i], i);
}
