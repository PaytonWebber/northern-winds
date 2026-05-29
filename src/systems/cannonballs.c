#include "cannonballs.h"
#include "../game_state.h"
#include "camera.h"
#include "particles.h"
#include "pd_api/pd_api_gfx.h"
#include <math.h>

typedef struct {
    vec2_t pos;
    vec2_t vel;
    float  range_remaining;
    int    owner_id;
} cannonball_t;

static cannonball_t s_cannonballs[MAX_CANNONBALLS];
static int          s_active_count     = 0;
static const float  S_STARTING_RANGE   = 100.0f;
static const int    S_SPLASH_PARTICLES = 25;
static const float  S_SPLASH_SPEED     = 25.0f;
static const float  S_SPLASH_LIFE      = 0.8f;

static void spawn_splash(vec2_t pos) {
    for (int p = 0; p < S_SPLASH_PARTICLES; p++) {
        const float  angle = rand_unit() * 2.0f * PI;
        const vec2_t dir   = dir_from_angle(angle);
        const float  speed = S_SPLASH_SPEED * (0.5f + rand_unit() * 0.5f);
        const vec2_t vel   = vec2_scale(dir, speed);
        const float  life  = S_SPLASH_LIFE * (0.5f + rand_unit() * 0.5f);
        particles_spawn(pos, vel, life);
    }
}

void cannonballs_fire(vec2_t pos, vec2_t vel, int owner_id) {
    if (s_active_count >= MAX_CANNONBALLS)
        return;
    s_cannonballs[s_active_count++] = (cannonball_t) {
        .pos = pos, .vel = vel, .range_remaining = S_STARTING_RANGE, .owner_id = owner_id};
};
void cannonballs_update(float dt) {
    for (int i = s_active_count - 1; i >= 0; i--) {
        s_cannonballs[i].range_remaining -=
            hypotf(s_cannonballs[i].vel.x, s_cannonballs[i].vel.y) * dt;
        if (s_cannonballs[i].range_remaining <= 0.0f) {
            spawn_splash(s_cannonballs[i].pos);
            s_cannonballs[i] = s_cannonballs[s_active_count - 1];
            s_active_count--;
            continue;
        }
        s_cannonballs[i].pos = vec2_add(s_cannonballs[i].pos, vec2_scale(s_cannonballs[i].vel, dt));
    }
};
void cannonballs_draw(void) {
    for (int i = s_active_count - 1; i >= 0; i--) {
        int sx, sy;
        camera_world_to_screen(s_cannonballs[i].pos, &sx, &sy);
        if (sx >= 0 && (float) sx < SCREEN_WIDTH && sy >= 0 && (float) sy < SCREEN_HEIGHT) {
            game_state.pd->graphics->fillRect(sx, sy, 5, 5, kColorBlack);
        }
    }
};
