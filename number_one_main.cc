#include <fstream>
#include <iostream>
#include <string>

using std::cin;
using std::cout;
using std::endl;
using std::string;

#include "eval_state.h"
#include "game_state.h"

static const string type_names[] = {"Prep", "Catch", "Pierce", "Aura"};

void printState(const struct game_state &state) {
  cout << "Player 1" << endl;
  cout << "HP: " << state.p1.hp << endl;
  cout << "Ammo: " << (int) state.p1.bullets << endl;
  cout << "Fatigue: " << (int) state.p1.fatigue << endl;
  cout << "Power: " << type_names[state.p1.type] << endl << endl;

  cout << "Player 2" << endl;
  cout << "HP: " << state.p2.hp << endl;
  cout << "Ammo: " << (int) state.p2.bullets << endl;
  cout << "Fatigue: " << (int) state.p2.fatigue << endl;
  cout << "Power: " << type_names[state.p2.type] << endl << endl;

  cout << "Round " << (int) state.round << endl;
}

// MoonBurst is a lazy pony
// No error checking - if you messed up copy/pasting you deserve whatever you get
static void moonburst_loop() {
  for (;;) {
    string s, p1power, p2power;
    int p1hp, p1ammo, p1fat, p2hp, p2ammo, p2fat, round;
    do {
      cin >> s;
    } while (s != "HP:");
    cin >> p1hp >> s >> p1ammo >> s >> p1fat >> p1power;
    if (p1power == "Power:") {
      cin >> p1power;
    }
    do {
      cin >> s;
    } while (s != "HP:");
    cin >> p2hp >> s >> p2ammo >> s >> p2fat >> p2power;
    if (p2power == "Power:") {
      cin >> p2power;
    }
    cin >> s >> round;
    if (!cin) {
      if (cin.eof())
        return;
      std::cerr << "NO U" << endl;
      cin.clear();
      continue;
    }
    struct game_state state1;
    state1.p1.hp = p1hp; state1.p1.bullets = p1ammo; state1.p1.fatigue = p1fat;
    state1.p2.hp = p2hp; state1.p2.bullets = p2ammo; state1.p2.fatigue = p2fat;
    if (p1power.find("Prep") != string::npos) {
      state1.p1.type = PREPARATION;
    } else if (p1power.find("Pierce") != string::npos) {
      state1.p1.type = PIERCE;
    } else if (p1power.find("Catch") != string::npos) {
      state1.p1.type = CATCH;
    } else if (p1power.find("Aura") != string::npos) {
      state1.p1.type = AURA;
    } else {
      std::cerr << "Invalid power for player 1" << endl;
    }
    if (p2power.find("Prep") != string::npos) {
      state1.p2.type = PREPARATION;
    } else if (p2power.find("Pierce") != string::npos) {
      state1.p2.type = PIERCE;
    } else if (p2power.find("Catch") != string::npos) {
      state1.p2.type = CATCH;
    } else if (p2power.find("Aura") != string::npos) {
      state1.p2.type = AURA;
    } else {
      std::cerr << "Invalid power for player 2" << endl;
    }
    state1.round = round;
    get_doubletime_strat(state1);
  }
}

// legacy code - takes states in the old format
static void solve_loop() {
  // Takes input from cin
  string argv[10];  // this array is named argv because I'm lazy
  while (true) {
    cout << "Input position:" << endl;
    cin >> argv[1] >> argv[2] >> argv[3] >> argv[4];
    cin >> argv[5] >> argv[6] >> argv[7] >> argv[8];
    cin >> argv[9];
    if (!cin) {
      if (cin.eof())
        return;  // who cares what happened
      std::cerr << "Invalid position" << endl;
      cin.clear();
      continue;
    }
    struct game_state state1;
    state1.p1.hp = stoi(argv[1]);
    state1.p1.bullets = stoi(argv[2]);
    state1.p1.fatigue = stoi(argv[3]);
    if (argv[4] == "prep") {
      state1.p1.type = PREPARATION;
    } else if (argv[4] == "catch") {
      state1.p1.type = CATCH;
    } else if (argv[4] == "pierce") {
      state1.p1.type = PIERCE;
    } else if (argv[4] == "aura") {
      state1.p1.type = AURA;
    } else {
      std::cerr << "Invalid power for player 1" << endl;
      continue;
    }

    state1.p2.hp = stoi(argv[5]);
    state1.p2.bullets = stoi(argv[6]);
    state1.p2.fatigue = stoi(argv[7]);
    if (argv[8] == "prep") {
      state1.p2.type = PREPARATION;
    } else if (argv[8] == "catch") {
      state1.p2.type = CATCH;
    } else if (argv[8] == "pierce") {
      state1.p2.type = PIERCE;
    } else if (argv[8] == "aura") {
      state1.p2.type = AURA;
    } else {
      std::cerr << "Invalid power for player 2" << endl;	
      continue;
    }
    state1.round = stoi(argv[9]);
    cout << endl << "Solving position (with doubletimes):" << endl;
    printState(state1);
    get_doubletime_strat(state1);
  }
}

int main() {
  cout << "N1 bot: ver 0.1.1" << endl;
  cout << "Copy the match information from the N1 page (N1 Enhancer script compatible)" << endl;

  std::ifstream data_file ("n1bot_data.bin", std::ios::in | std::ios::binary);
  if (!data_file) {
    cout << "Data file not found, generating..." << endl;
    dout.reset(new std::ofstream("n1bot_data.bin",
                                 std::ios::out | std::ios::binary));
    char type1, type2;
    for (type1 = 0; type1 < 4; ++type1) {
      for (type2 = 0; type2 < 4; ++type2) {
        struct game_state state1 = new_game(type1, type2);
        double winChance = eval_state(state1);
        cout << type_names[type1] << " v " << type_names[type2] << '\t'
             << winChance << endl;
      }
    }
    dout.reset();
  } else {
    // Read from data file
    int nstates;
    int state;
    double winchance;
    for (nstates = 0;;nstates++) {
      data_file.read((char *)&state, sizeof(state));
      data_file.read((char *)&winchance, sizeof(winchance));
      if (!data_file)
        break;
      if (state >= 1 << 25 ||
          winchance < -0.01 ||
          winchance > 1.01) {
        std::cerr << "Data file is corrupt" << endl;
        exit(-1);
      }
      memoization[state] = winchance;
    }
    cout << "Read " << nstates << " states from data file" << endl;
  }
  cout << "Finished initializing." << endl;
  moonburst_loop();
}
