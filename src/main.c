#include "pd_api.h"
#include "pd_api/pd_api_gfx.h"
#include "pd_api/pd_api_lua.h"
#include "pd_api/pd_api_sprite.h"
#include "pd_api/pd_api_sys.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define PI 3.1415926535897932384626433832f
#define DEG2RAD (PI / 180.0f)

static const PlaydateAPI *pd = NULL;
static const struct playdate_graphics *gfx = NULL;

LCDBitmap *block_gfx = NULL;
LCDSprite *block = NULL;

typedef struct {
  float x;
  float y;
  float speed;
  float heading;      // degrees
  float rudder_angle; // degrees
  LCDSprite *sprite;
  LCDBitmap *gfx;
} Ship;

Ship ship;

void init_ship(void) {
  const char *err = NULL;
  ship.gfx =
      gfx->loadBitmap("images/PNG/Default_size/Ships/dinghyLarge1", &err);
  if (ship.gfx == NULL) {
    pd->system->logToConsole("loadBitmap failed: %s", err);
    return;
  }
  ship.sprite = pd->sprite->newSprite();
  ship.heading = 0;
  ship.speed = 1;
  ship.rudder_angle = 0;
  ship.x = 220;
  ship.y = 100;
  pd->sprite->setImage(ship.sprite, ship.gfx, kBitmapUnflipped);
  pd->sprite->setCollisionsEnabled(ship.sprite, 1);
  pd->sprite->setCollideRect(ship.sprite, (PDRect){0, 0, 32, 32});
  pd->sprite->addSprite(ship.sprite);
  pd->sprite->moveTo(ship.sprite, 220, 100);
}

void update_ship_movement(void) {
  float crank_change = pd->system->getCrankChange();
  ship.rudder_angle += (crank_change * 0.1f);

  if (ship.rudder_angle > 45.0f)
    ship.rudder_angle = 45.0f;
  if (ship.rudder_angle < -45.0f)
    ship.rudder_angle = -45.0f;

  ship.heading += (ship.rudder_angle * ship.speed * 0.02f);
  while (ship.heading >= 360.0f)
    ship.heading -= 360.0f;
  while (ship.heading < 0.0f) {
    ship.heading += 360.0f;
  }

  float heading_rad = ship.heading * DEG2RAD;
  float vel_x = sinf(heading_rad) * ship.speed;
  float vel_y = -cosf(heading_rad) * ship.speed;
  float ax, ay;
  int len;
  SpriteCollisionInfo *results = pd->sprite->moveWithCollisions(
      ship.sprite, ship.x + vel_x, ship.y + vel_y, &ax, &ay, &len);
  ship.x = ax;
  ship.y = ay;
  // pd->system->logToConsole("Postion: {%f, %f}\nHeading: %f", ship.x, ship.y,
  //                          ship.heading);
  free(results);
}

void update_ship_sprite(void) {
  gfx->drawRotatedBitmap(ship.gfx, (int)ship.x, (int)ship.y, ship.heading, 0.5f,
                         0.5f, 1.0f, 1.0f);
}

void draw_rect(void) {
  block_gfx = gfx->newBitmap(32, 32, kColorWhite);
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
  update_ship_movement();
  update_ship_sprite();
  // pd->sprite->updateAndDrawSprites();
  pd->system->drawFPS(10, 10);
  return 1; // 1 = redraw screen
}

int eventHandler(PlaydateAPI *playdate, PDSystemEvent event, uint32_t arg) {
  switch (event) {
  case kEventInit:
    pd = playdate;
    gfx = pd->graphics;
    init_ship();
    draw_rect();
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
