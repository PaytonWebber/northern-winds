#include "pd_api.h"
#include "pd_api/pd_api_gfx.h"

static PlaydateAPI *pd = NULL;
int x, y, textHeight, textWidth;
int stepX = 1;
int stepY = 2;

static int update(void *userdata) {
  x += stepX;
  y += stepY;
  if (x < 0 || x > LCD_COLUMNS - textWidth) {
    stepX = -stepX;
  }
  if (y < 0 || y > LCD_ROWS - textHeight) {
    stepY = -stepY;
  }
  pd->graphics->clear(kColorWhite);
  pd->system->drawFPS(10, 10);
  return 1; // 1 = redraw screen
}

int eventHandler(PlaydateAPI *playdate, PDSystemEvent event, uint32_t arg) {
  switch (event) {
  case kEventInit:
    pd = playdate;
    pd->system->setUpdateCallback(update, NULL);
    textHeight = 14;
  default:
    break;
  };
  return 1;
}
