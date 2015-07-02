#ifndef EVAL_STATE_H_
#define EVAL_STATE_H_

#include <fstream>
#include <memory>
#include <unordered_map>
#include <vector>

#include "game_state.h"

double eval_state(const struct game_state &s);

void get_strat(const struct game_state &s);

void get_doubletime_strat(const struct game_state &s);

extern std::unordered_map<int, double> memoization;
extern std::unique_ptr<std::ofstream> dout;


#endif  // EVAL_STATE_H_
