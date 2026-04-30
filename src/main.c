#include "pd_api.h"
#include "pd_api/pd_api_gfx.h"
#include "pd_api/pd_api_lua.h"
#include "pd_api/pd_api_sprite.h"
#include "pd_api/pd_api_sys.h"
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define INV_SQRT2 0.70710678f
#define SPEED 4.0f

const float DIAG = SPEED * INV_SQRT2;

static PlaydateAPI *pd = NULL;
static const struct playdate_graphics *gfx = NULL;

LCDBitmap *block_gfx = NULL;
LCDSprite *block = NULL;

typedef struct {
  float velocity;
  float heading; // degrees
  LCDSprite *sprite;
  LCDBitmap *gfx;
} Ship;

Ship ship;

typedef struct {
  float startX;
  float startY;
  float endX;
  float endY;
  float angle;
  float length;
} Compass;

Compass compass;

void init_ship(void) {
  const char *err = NULL;
  ship.gfx = gfx->loadBitmap("images/test1", &err);
  if (ship.gfx == NULL) {
    pd->system->logToConsole("loadBitmap failed: %s", err);
    return;
  }
  ship.sprite = pd->sprite->newSprite();
  ship.heading = 0;
  ship.velocity = 0;
  pd->sprite->setImage(ship.sprite, ship.gfx, kBitmapUnflipped);
  pd->sprite->setCollisionsEnabled(ship.sprite, 1);
  pd->sprite->setCollideRect(ship.sprite, (PDRect){0, 0, 32, 32});
  pd->sprite->addSprite(ship.sprite);
  pd->sprite->moveTo(ship.sprite, 220, 100);
}

void update_ship(void) {
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
  pd->sprite->getPosition(ship.sprite, &x, &y);
  SpriteCollisionInfo *results = pd->sprite->moveWithCollisions(
      ship.sprite, x + dx, y + dy, &ax, &ay, &len);
  free(results);
}

void init_comapss(void) {
  compass.length = 15;
  compass.startX = 300;
  compass.startY = 200;
  compass.endX = 300;
  compass.endY = compass.startY - compass.length;
  compass.angle = 0;
}

void draw_compass(void) {
  float newAngle = pd->system->getCrankAngle();
  if (newAngle != compass.angle) {
    compass.angle = newAngle;
    compass.endX = compass.startX + sinf(newAngle) * compass.length;
    compass.endY = compass.startY - cosf(newAngle) * compass.length;
  }
  gfx->drawLine(compass.startX, compass.startY, compass.endX, compass.endY, 2,
                kColorBlack);
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
  update_ship();
  pd->sprite->updateAndDrawSprites();
  draw_rect();
  pd->system->drawFPS(10, 10);
  draw_compass();
  return 1; // 1 = redraw screen
}

int eventHandler(PlaydateAPI *playdate, PDSystemEvent event, uint32_t arg) {
  switch (event) {
  case kEventInit:
    pd = playdate;
    gfx = pd->graphics;
    init_ship();
    init_comapss();
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
