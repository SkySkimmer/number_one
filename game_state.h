#ifndef GAME_STATE_H_
#define GAME_STATE_H_

#include <cstddef>
#include "player.h"

struct game_state {
  struct player p1;
  struct player p2;
  unsigned char round;  // ranges from 1 to 16
};

// represents the game state as an integer
int game_state_rep(const struct game_state &x);

// constructs the initial state of the game given player types
struct game_state new_game(char type1, char type2);

// calculates the new game state from given state
struct game_state act(const struct game_state &s, char act1, char act2);


#endif  // GAME_STATE_H_
