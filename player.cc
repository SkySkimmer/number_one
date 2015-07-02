#include "player.h"

int player_rep(const struct player &p) {
  int ret = p.hp + 2;
  ret |= (static_cast<int>(p.bullets)) << 3;
  ret |= (static_cast<int>(p.fatigue)) << 6;
  ret |= (static_cast<int>(p.type)) << 8;
  return ret;
}
