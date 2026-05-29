#include "particles.h"
#include "../game_state.h"
#include "camera.h"
#include "pd_api/pd_api_gfx.h"

typedef struct {
    vec2_t pos;
    vec2_t vel;
    float  life_remaining;
} particle_t;

static particle_t       s_particles[MAX_PARTICLES];
static int              s_active_count   = 0;
static const float      S_WATER_FRICTION = 0.95f;
static const float      S_LIFE_MID       = 1.0f;
static const float      S_LIFE_DISSOLVE  = 0.5f;
static const int        S_SIZE_DENSE     = 3;
static const int        S_SIZE_MID       = 2;
static const int        S_SIZE_DISSOLVE  = 1;
static const LCDPattern S_FOAM_DENSE     = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xEE, 0xFF, 0xBB, 0xFF, 0xEE, 0xFF, 0xBB};
static const LCDPattern S_FOAM_MID = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55};

void particles_spawn(vec2_t pos, vec2_t vel, float life) {
    if (s_active_count >= MAX_PARTICLES)
        return;
    s_particles[s_active_count++] = (particle_t) {.pos = pos, .vel = vel, .life_remaining = life};
}

typedef struct {
    int      size;
    LCDColor color;
} foam_style_t;

static foam_style_t pick_foam_style(float life) {
    if (life < S_LIFE_DISSOLVE)
        return (foam_style_t) {S_SIZE_DISSOLVE, kColorWhite};
    if (life < S_LIFE_MID)
        return (foam_style_t) {S_SIZE_MID, (LCDColor) S_FOAM_MID};
    return (foam_style_t) {S_SIZE_DENSE, (LCDColor) S_FOAM_DENSE};
}

void particles_update(float dt) {
    for (int i = s_active_count - 1; i >= 0; i--) {
        s_particles[i].life_remaining -= dt;
        if (s_particles[i].life_remaining <= 0.0f) {
            s_particles[i] = s_particles[s_active_count - 1];
            s_active_count--;
            continue;
        }
        s_particles[i].pos    = vec2_add(s_particles[i].pos, vec2_scale(s_particles[i].vel, dt));
        const float retention = powf(S_WATER_FRICTION, dt);
        s_particles[i].vel    = vec2_scale(s_particles[i].vel, retention);
    }
}

void particles_draw(void) {
    for (int i = s_active_count - 1; i >= 0; i--) {
        const foam_style_t style = pick_foam_style(s_particles[i].life_remaining);
        int                sx, sy;
        camera_world_to_screen(s_particles[i].pos, &sx, &sy);
        if (sx >= 0 && (float) sx < SCREEN_WIDTH && sy >= 0 && (float) sy < SCREEN_HEIGHT) {
            game_state.pd->graphics->fillRect(sx, sy, style.size, style.size, style.color);
        }
    }
}
