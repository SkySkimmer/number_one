#include "game_state.h"
#include <cassert>

int game_state_rep(const struct game_state &x) {
  return
    (player_rep(x.p1) << 14) ^
    (player_rep(x.p2) << 4) ^
    (x.round - 1);
}

// types: 0 = prep, 1 = catch, 2 = pierce, 3 = aura
struct game_state new_game(char type1, char type2) {
  struct game_state ret;
  ret.p1.hp = 5;
  ret.p1.bullets = 1;
  ret.p1.fatigue = 0;
  ret.p1.type = type1;

  ret.p2.hp = 5;
  ret.p2.bullets = 1;
  ret.p2.fatigue = 0;
  ret.p2.type = type2;

  if (type1 == PREPARATION) {
    ret.p1.bullets++;
  }
  if (type2 == PREPARATION) {
    ret.p2.bullets++;
  }
  if (type1 == AURA) {
    ret.p2.hp -= 2;
  }
  if (type2 == AURA) {
    ret.p1.hp -= 2;
  }

  ret.round = 1;
  return ret;
}

// calculates the game state resulting from the given actions from the given
// game state
// -1 = reload
// 0 = block
// +n = fire n
struct game_state act(const struct game_state &s, char act1, char act2) {
  struct game_state new_state = s;

  // Resolve round change and HP damage due to round 10+
  if (new_state.round ++>= 10) {
    new_state.p1.hp--;
    new_state.p2.hp--;
  }

  // Resolve bullet count change
  new_state.p1.bullets -= act1;
  new_state.p2.bullets -= act2;
  // should add some asserts here really

  // Resolve fatigue and effects thereof
  if (act1 || (act2 > 0)) {
    // player 1 did not block, or blocked a shot
    new_state.p1.fatigue = 0;
  } else if (act2 <= 0) {
    switch (new_state.p1.fatigue) {
    case 0:
      new_state.p1.fatigue = 1;
      break;
    case 1:
      new_state.p1.fatigue = 2;
      break;
    case 2:
      new_state.p1.fatigue = 3;
      // fall through
    case 3:
      new_state.p1.hp--;
      break;
    }
  }

  if (act2 || (act1 > 0)) {
    // player 2 did not block, or blocked a shot
    new_state.p2.fatigue = 0;
  } else if (act1 <= 0) {
    switch (new_state.p2.fatigue) {
    case 0:
      new_state.p2.fatigue = 1;
      break;
    case 1:
      new_state.p2.fatigue = 2;
      break;
    case 2:
      new_state.p2.fatigue = 3;
      // fall through
    case 3:
      new_state.p2.hp--;
      break;
    }
  }

  // Resolve results of shooting
  if (act1 > 0) {
    if (act2 > 0) {
      // both players shot
      if (act1 > act2) {
        // p2 loses
        new_state.p2.hp = -10;
      } else if (act1 < act2 ||
                 (new_state.p2.type == PIERCE &&
                  new_state.p1.type != PIERCE)) {
        // p1 loses
        new_state.p1.hp = -10;
      } else if (new_state.p1.type == PIERCE &&
                 new_state.p2.type != PIERCE) {
        // p2 loses
        new_state.p2.hp = -10;
      }
    } else if (act2) {
      // p2 reloaded and therefore loses
      new_state.p2.hp = -10;
    } else {
      // p2 blocked
      new_state.p2.hp -= (act1 - 1);
      if (new_state.p2.type == CATCH) {
        // bullet was caught
        new_state.p2.bullets++;
      }
    }
  } else if (act2 > 0) {
    if (act1) {
      // p1 reloaded and loses
      new_state.p1.hp = -10;
    } else {
      // p1 blocked
      new_state.p1.hp -= (act2 - 1);
      if (new_state.p1.type == CATCH) {
        new_state.p1.bullets++;
      }
    }
  }
  // cap bullet count
  if (new_state.p1.bullets > 6) {
    new_state.p1.bullets = 6;
  }
  if (new_state.p2.bullets > 6) {
    new_state.p2.bullets = 6;
  }
  // cap hp count
  if (new_state.p1.hp < 0) {
    new_state.p1.hp = -1;
  }
  if (new_state.p2.hp < 0) {
    new_state.p2.hp = -1;
  }
  return new_state;
}
