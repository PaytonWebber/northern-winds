#include "pd_api.h"
#include "pd_api/pd_api_gfx.h"
#include "pd_api/pd_api_lua.h"
#include "pd_api/pd_api_sprite.h"
#include "pd_api/pd_api_sys.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define INV_SQRT2 0.70710678f
#define SPEED 4.0f

const float DIAG = SPEED * INV_SQRT2;

static PlaydateAPI *pd = NULL;
static const struct playdate_graphics *gfx = NULL;

LCDBitmap *player_gfx = NULL;
LCDSprite *player = NULL;
LCDBitmap *block_gfx = NULL;
LCDSprite *block = NULL;

void init_player(void) {
  const char *err = NULL;
  player_gfx = gfx->loadBitmap("images/test1", &err);
  if (player_gfx == NULL) {
    pd->system->logToConsole("loadBitmap failed: %s", err);
    return;
  }
  player = pd->sprite->newSprite();
  pd->sprite->setImage(player, player_gfx, kBitmapUnflipped);
  pd->sprite->moveTo(player, 200, 100);
  pd->sprite->setCollisionsEnabled(player, 1);
  pd->sprite->setCollideRect(player, (PDRect){0, 0, 32, 32});
  pd->sprite->addSprite(player);
}

void update_player(void) {
  PDButtons current;
  pd->system->getButtonState(&current, NULL, NULL);
  float dx = 0, dy = 0, ax = 0, ay = 0;
  int len = 0;
  if (current & kButtonUp)
    dy -= 1.0f;
  if (current & kButtonDown)
    dy += 1.0f;
  if (current & kButtonLeft)
    dx -= 1.0f;
  if (current & kButtonRight)
    dx += 1.0f;
  if (dx && dy) {
    dx *= DIAG;
    dy *= DIAG;
  } else if (dx || dy) {
    dx *= SPEED;
    dy *= SPEED;
  }
  float x, y;
  pd->sprite->getPosition(player, &x, &y);
  SpriteCollisionInfo *results =
      pd->sprite->moveWithCollisions(player, x + dx, y + dy, &ax, &ay, &len);
  free(results);
}

void draw_rect(void) {
  gfx->pushContext(block_gfx);
  {
    gfx->drawRect(0, 0, 32, 32, kColorBlack);
  }
  gfx->popContext();
  block = pd->sprite->newSprite();
  pd->sprite->setImage(block, block_gfx, kBitmapUnflipped);
  pd->sprite->setCollideRect(block, (PDRect){0, 0, 32, 32});
  pd->sprite->setCollisionsEnabled(block, 1);
  pd->sprite->addSprite(block);
}

static int update(void *userdata) {
  gfx->clear(kColorWhite);
  update_player();
  pd->sprite->updateAndDrawSprites();
  draw_rect();
  pd->system->drawFPS(10, 10);
  return 1; // 1 = redraw screen
}

int eventHandler(PlaydateAPI *playdate, PDSystemEvent event, uint32_t arg) {
  switch (event) {
  case kEventInit:
    pd = playdate;
    gfx = pd->graphics;
    init_player();
    pd->system->setUpdateCallback(update, NULL);
    break;
  case kEventUnlock:
    pd->system->logToConsole("Unlocked\n");
    break;
  default:
    break;
  };
  return 1;
}
