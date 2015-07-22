#include "eval_state.h"

#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <unordered_map>
#include <vector>
#include <cassert>
#include <cstdlib>

#include "player.h"
#include "zs_wrap.h"

std::unique_ptr<std::ofstream> dout;
std::unordered_map<int, double> memoization;

struct game_state rev_state(const struct game_state &s) {
  auto s_(s);
  std::swap(s_.p1, s_.p2);
  return s_;
}

int game_state_rev_rep(const struct game_state &s) {
  return game_state_rep(rev_state(s));
}

// Computes the probability that the first player will win
double eval_state(const struct game_state &s) {
  int state_rep = game_state_rep(s);
  {
    auto s_rev = rev_state(s);
    if (state_rep > game_state_rep(s_rev)) {
      return 1.0 - eval_state(s_rev);
    }
  }
  if (memoization.count(state_rep)) {
    // already calculated; return memoized result
    return memoization[state_rep];
  }

  // Check if game is already over (that is, whether either player has
  // negative HP)
  if ((s.p1.hp <= 0) || (s.p2.hp <= 0)) {
    // the player with less HP loses
    double result;
    if (s.p1.hp < s.p2.hp) {
      result = 0;
    } else if (s.p1.hp > s.p2.hp) {
      result = 1;
    } else {
      result = 0.5;
    }
    return result;
  }

  // Autoload if neither player has a bullet
  if (s.p1.bullets == 0 && s.p2.bullets == 0) {
    double res = eval_state(act(s, -1, -1));
    return res;
  }

  // Otherwise we need to do the recursive step; we call eval_state on
  // every position which can be reached from the current position and then
  // call zs_solve on it

  // we have s.p1.bullets + 2 columns and s.p2.bullets + 2 rows
  double *payoffMatrix = reinterpret_cast<double *>(malloc((s.p1.bullets + 2) *
                                                           (s.p2.bullets + 2) *
                                                           sizeof(double)));
  for (int p2Move = -1; p2Move <= s.p2.bullets; p2Move++) {
    int offset = (p2Move + 1) * (s.p1.bullets + 2);
    for (int p1Move = -1; p1Move <= s.p1.bullets; p1Move++) {
      payoffMatrix[offset + (p1Move + 1)] = 1 +
        eval_state(act(s, p1Move, p2Move));
      // adding offset of 1 to avoid some rounding problems
    }
  }

  double *results = zs_solve(s.p2.bullets + 2,
                             s.p1.bullets + 2,
                             payoffMatrix);
  free(payoffMatrix);
  if (results == NULL) {
    std::cerr << "Unsolved position?!" << std::endl;
    // It'd be weird, because zero-sum games are always solvable
    exit(-1);
  }
  results[0] -= 1;

  if (results[0] < 0.00000001) {
    results[0] = 0;
  }
  if (results[0] > 1) {
    results[0] = 1;
  }
  double value = results[0];
  free(results);
  if (dout) {
    dout->write((const char *)&state_rep, sizeof(state_rep));
    dout->write((const char *)&value, sizeof(value));
  }
  memoization[state_rep] = value;
  return value;
}

namespace {
  std::random_device rd;
  std::default_random_engine generator(rd());
  std::uniform_real_distribution<double> distribution(0.0, 1.0);
}

// Prints a move generated according to spe distribution
void get_strat(const struct game_state &s) {
  using std::cout;
  using std::endl;
  // Autoload
  if (s.p1.bullets == 0 && s.p2.bullets == 0) {
    cout << "Autoload! ";
    get_strat(act(s, -1, -1));
    return;
  }

  // Check if game is already over (that is, whether either player has
  // negative HP)
  if ((s.p1.hp <= 0) || (s.p2.hp <= 0)) {
    cout << "Reload (Game is already over)" << endl;
    return;
  }

  double *payoffMatrix = reinterpret_cast<double *>(malloc((s.p1.bullets + 2) *
                                                           (s.p2.bullets + 2) *
                                                           sizeof(double)));
  for (int p2Move = -1; p2Move <= s.p2.bullets; p2Move++) {
    int offset = (p2Move + 1) * (s.p1.bullets + 2);
    for (int p1Move = -1; p1Move <= s.p1.bullets; p1Move++) {
      payoffMatrix[offset + (p1Move + 1)] = 1 +
        eval_state(act(s, p1Move, p2Move));
    }
  }

  double *results = zs_solve(s.p2.bullets + 2, s.p1.bullets + 2, payoffMatrix);
  if (results[0] - 1 < 0.000001) {
    // Experimental code: ignore all opponent's moves which are 100% to win,
    // and calculate our strategy based on that matrix.
    std::vector<int> badMoves;
    int p2MoveIdx = 0;
    for (int p2Move = -1; p2Move <= s.p2.bullets; p2Move++) {
      int offset = (p2Move + 1) * (s.p1.bullets + 2);
      // check whether the move is 100% winning for opponent
      int p1Move;
      for (p1Move = -1; p1Move <= s.p1.bullets; p1Move++) {
        if (2.0 - payoffMatrix[offset + (p1Move + 1)] > 0.000001) {
          // then we have a shot of winning
          break;
        }
      }
      if (p1Move <= s.p1.bullets) {
        badMoves.push_back(p2Move);
      }
    }
    if (badMoves.size() > 0) {
      if (badMoves.size() > 1) {
        free(results);  // we're calculating a new one anyway
        double *newPayoffMatrix = reinterpret_cast<double *>(malloc((s.p1.bullets + 2) * (badMoves.size()) * sizeof(double)));
        for (unsigned i = 0; i < badMoves.size(); ++i) {
          int p2Move = badMoves[i];
          int offset = i * (s.p1.bullets + 2);
          for (int p1Move = -1; p1Move <= s.p1.bullets; p1Move++) {
            newPayoffMatrix[offset + (p1Move + 1)] = 1 +
              eval_state(act(s, p1Move, p2Move));
          }
        }
        results = zs_solve(badMoves.size(), s.p1.bullets + 2, newPayoffMatrix);
        free(newPayoffMatrix);
      }
      else {
        // no need to waste our time; take the largest element of the appropriate row
        int offset = badMoves[0] * (s.p1.bullets + 2);
        double maxWinChance = 0;
        int chosenMove = -1;
        for (int p1Move = -1; p1Move <= s.p1.bullets; p1Move++) {
          if (payoffMatrix[offset + (p1Move + 1)] > maxWinChance) {
            maxWinChance = payoffMatrix[offset + (p1Move + 1)];
            chosenMove = p1Move;
          }
        }
        // Write our choice of moves into the results array
        for (int i = 0; i < s.p1.bullets + 2; ++i) {
          results[i + 1] = (chosenMove + 1 == i);
        }
      }
    }
  }
  // print out actual strategy
  /*
    cout << "Mixed-strategy subgame perfect equilibrium:" << endl;
    cout << "Probability to win: " << results[0] - 1 << endl;
    cout << "Reload with probability " << results[1] << endl;
    cout << "Block with probability " << results[2] << endl;
    for (int i = 1; i <= s.p1.bullets; ++i) {
    cout << "Fire " << i << " with probability " << results[2+i] << endl;
    }
  */
  double move = distribution(generator);
  for (int i = 0; i < s.p1.bullets + 2; ++i) {
    move -= results[1 + i];
    if (move <= 0 || i == s.p1.bullets + 1) {
      // choose the action at i-1
      if (i == 0) {
        cout << "Reload" << endl;
      } else if (i == 1) {
        cout << "Block" << endl;
      } else {
        cout << "Fire " << i-1 << endl;
      }
      break;
    }
  }
  free(payoffMatrix);
  free(results);
}

// Prints doubletime strat at a given position
void get_doubletime_strat(const struct game_state &s) {
  using std::cout;
  using std::endl;
  // Check whether the position is actually memoized
  if (!(memoization.count(game_state_rep(s)) || memoization.count(game_state_rev_rep(s)))) {
    cout << "Unreachable state" << endl;
    return;
  }

  double *payoffMatrix = reinterpret_cast<double *>(malloc((s.p1.bullets + 2) *
                                                           (s.p2.bullets + 2) *
                                                           sizeof(double)));
  for (int p2Move = -1; p2Move <= s.p2.bullets; p2Move++) {
    int offset = (p2Move + 1) * (s.p1.bullets + 2);
    for (int p1Move = -1; p1Move <= s.p1.bullets; p1Move++) {
      payoffMatrix[offset + (p1Move + 1)] = 1 +
        eval_state(act(s, p1Move, p2Move));
    }
  }

  double *results = zs_solve(s.p2.bullets + 2, s.p1.bullets + 2, payoffMatrix);
  if (results[0] - 1 < 0.000001) {
    // Experimental code: ignore all opponent's moves which are 100% to win,
    // and calculate our strategy based on that matrix.
    std::vector<int> badMoves;
    int p2MoveIdx = 0;
    for (int p2Move = -1; p2Move <= s.p2.bullets; p2Move++) {
      int offset = (p2Move + 1) * (s.p1.bullets + 2);
      // check whether the move is 100% winning for opponent
      int p1Move;
      for (p1Move = -1; p1Move <= s.p1.bullets; p1Move++) {
        if (2.0 - payoffMatrix[offset + (p1Move + 1)] > 0.000001) {
          // then we have a shot of winning
          break;
        }
      }
      if (p1Move <= s.p1.bullets) {
        badMoves.push_back(p2Move);
      }
    }
    if (badMoves.size() > 0) {
      if (badMoves.size() > 1) {
        free(results);  // we're calculating a new one anyway
        double *newPayoffMatrix = reinterpret_cast<double *>(malloc((s.p1.bullets + 2) *
                                                                    (badMoves.size()) *	sizeof(double)));
        for (unsigned i = 0; i < badMoves.size(); ++i) {
          int p2Move = badMoves[i];
          int offset = i * (s.p1.bullets + 2);
          for (int p1Move = -1; p1Move <= s.p1.bullets; p1Move++) {
            newPayoffMatrix[offset + (p1Move + 1)] = 1 +
              eval_state(act(s, p1Move, p2Move));
          }
        }
        results = zs_solve(badMoves.size(), s.p1.bullets + 2, newPayoffMatrix);
        free(newPayoffMatrix);
      }
      else {
        // no need to waste our time; take the largest element of the appropriate row
        int offset = badMoves[0] * (s.p1.bullets + 2);
        double maxWinChance = 0;
        int chosenMove = -1;
        for (int p1Move = -1; p1Move <= s.p1.bullets; p1Move++) {
          if (payoffMatrix[offset + (p1Move + 1)] > maxWinChance) {
            maxWinChance = payoffMatrix[offset + (p1Move + 1)];
            chosenMove = p1Move;
          }
        }
        // Write our choice of moves into the results array
        for (int i = 0; i < s.p1.bullets + 2; ++i) {
          results[i + 1] = (chosenMove + 1 == i);
        }
      }
    }
  }

  double move = distribution(generator);
  for (int i = 0; i < s.p1.bullets + 2; ++i) {
    move -= results[1 + i];
    if (move <= 0 || i == s.p1.bullets + 1) {
      cout << "\tAction: ";
      // choose the action at i-1
      if (i == 0) {
        cout << "Reload" << endl;
      } else if (i == 1) {
        cout << "Block" << endl;
      } else {
        cout << "Fire " << i-1 << endl;
      }
      cout << "\tDoubletimes" << endl;
      cout << "\tReload: ";
      get_strat(act(s, i-1, -1));
      cout << "\tBlock:  ";
      get_strat(act(s, i-1, 0));
      for (int j = 1; j <= s.p2.bullets; ++j) {
        cout << "\tFire " << j << ": ";
        get_strat(act(s, i-1, j));
      }
      break;
    }
  }
  free(payoffMatrix);
  free(results);
}
