#include <fstream>
#include <iostream>
#include <string>
#include <algorithm>

using std::cerr;
using std::cout;

#include "eval_state.h"
#include "game_state.h"

static const std::string type_names[] = {"Prep", "Catch", "Pierce", "Aura"};

int main() {
  cerr << "N1 bot generator: ver 0.6.0" << std::endl;
  cerr << "Generating JSON..." << std::endl;

  std::ofstream states("states.json"), wins("n1wins.txt");
  int type1, type2;
  states << "{" << std::endl;
  for (type1 = 0; type1 < 4; ++type1) {
    for (type2 = 0; type2 < 4; ++type2) {
      struct game_state state1 = new_game(type1, type2);
      double winChance = eval_state(state1, states, wins);
      cerr << type_names[type1] << " v " << type_names[type2] << '\t'
           << winChance << std::endl;
    }
  }
  states << std::endl << "}" << std::endl;

  cerr << "JSON generated." << std::endl;
}
