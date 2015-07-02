#ifndef PLAYER_H_
#define PLAYER_H_

enum player_type {
  PREPARATION = 0,
  CATCH = 1,
  PIERCE = 2,
  AURA = 3
};

struct player {
  char hp;  // ranges from -1 to 5
  unsigned char bullets;  // ranges from 0 to 6
  unsigned char fatigue;  // ranges from 0 to 3
  unsigned char type;  // ranges from 0 to 3
};

int player_rep(const struct player &p);

#endif  // PLAYER_H_
