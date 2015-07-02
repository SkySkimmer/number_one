#include "player.h"

int player_rep(const struct player &p) {
  int ret = p.hp + 2;
  ret |= ((int)p.bullets) << 3;
  ret |= ((int)p.fatigue) << 6;
  ret |= ((int)p.type) << 8;
  return ret;
}
