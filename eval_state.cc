#include "eval_state.h"

#include <chrono>
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

std::unordered_map<int, double> memoization;

bool has_printed = false;
const std::string quote = "\"";
const std::string tab = "    ";

// Computes the probability that the first player will win
double eval_state(const struct game_state &s) {
  int state_rep = game_state_rep(s);
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

  // print json
  if (has_printed) {
    std::cout << "," << std::endl;
  }
  has_printed = true;
  std::cout << tab << quote << state_rep << quote << " : {" << std::endl;
  for (int p1Move = -1; p1Move <= s.p1.bullets; ++p1Move) {
    // results: 0 => win chance, 1 => reload proba, ...
    // so -1 => 1
    std::cout << tab << tab << quote << p1Move << quote << " : " << results[p1Move+2];
    if (p1Move < s.p1.bullets)
      std::cout << ",";
    std::cout << std::endl;
  }
  std::cout << tab << "}";


  double value = results[0];
  free(results);
  memoization[state_rep] = value;
  return value;
}
