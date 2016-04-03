#include <fstream>
#include <iostream>
#include <string>
#include <algorithm>
#include <argp.h>

using std::cin;
using std::cerr;
using std::endl;
using std::string;

#include "eval_state.h"
#include "game_state.h"

static const string type_names[] = {"Prep", "Catch", "Pierce", "Aura"};

void printState(const struct game_state &state) {
  cerr << "Player 1" << endl;
  cerr << "HP: " << static_cast<int>(state.p1.hp) << endl;
  cerr << "Ammo: " << static_cast<int>(state.p1.bullets) << endl;
  cerr << "Fatigue: " << static_cast<int>(state.p1.fatigue) << endl;
  cerr << "Power: " << type_names[state.p1.type] << endl << endl;

  cerr << "Player 2" << endl;
  cerr << "HP: " << static_cast<int>(state.p2.hp) << endl;
  cerr << "Ammo: " << static_cast<int>(state.p2.bullets) << endl;
  cerr << "Fatigue: " << static_cast<int>(state.p2.fatigue) << endl;
  cerr << "Power: " << type_names[state.p2.type] << endl << endl;

  cerr << "Round " << static_cast<int>(state.round) << endl;
}

// MoonBurst is a lazy pony
static void moonburst_loop() {
  for (;;) {
    string s, p1power, p2power;
    int p1hp, p1ammo, p1fat, p2hp, p2ammo, p2fat, round;
    do {
      cin >> s;
    } while (s != "HP:" && cin);
    cin >> p1hp >> s >> p1ammo >> s >> p1fat >> p1power;
    if (p1power == "Power:") {
      cin >> p1power;
    }
    do {
      cin >> s;
    } while (s != "HP:" && cin);
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
    cerr << "Input position:" << endl;
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
    cerr << endl << "Solving position (with doubletimes):" << endl;
    printState(state1);
    get_doubletime_strat(state1);
  }
}

string dirname(string source)
{
  source.erase(std::find(source.rbegin(), source.rend(), '/').base(), source.end());
  return source;
}


const char *argp_program_version =
  "N1 bot 0.3";
const char *argp_program_bug_address =
  "<https://github.com/yichizhng/number_one/issues>";

/* Short description. */
static char doc[] =
  "BvS Number One bot";

/* The options we understand. */
static struct argp_option options[] = {
  {"data",   'd', "FILE", 0,
   "Use FILE as data file instead of 'n1bot_data.bin' next to the executable" },
  { 0 }
};

/* Used by main to communicate with parse_opt. */
struct arguments
{
  string data_path;
};

/* Parse a single option. */
static error_t
parse_opt (int key, char *arg, struct argp_state *state)
{
  /* Get the input argument from argp_parse, which we
     know is a pointer to our arguments structure. */
  struct arguments *arguments = (struct arguments*) state->input;

  switch (key)
    {
    case 'd':
      arguments->data_path = arg;
      break;

    case ARGP_KEY_ARG:
      /* Too many arguments (ie more than 0 non option arguments). */
      argp_usage (state);
      break;

    case ARGP_KEY_END:
      break;

    default:
      return ARGP_ERR_UNKNOWN;
    }
  return 0;
}

static struct argp argp = { options, parse_opt, 0, doc };

int main(int argc, char** argv) {

  struct arguments arguments;
  arguments.data_path = dirname(argv[0]) + "n1bot_data.bin";

  argp_parse (&argp, argc, argv, 0, 0, &arguments);

  cerr << "N1 bot: ver 0.3" << endl;
  cerr << "Copy the match information from the N1 page (N1 Enhancer script"
          " compatible)" << endl;

  std::ifstream data_file(arguments.data_path, std::ios::in | std::ios::binary);
  if (!data_file) {
    cerr << "Data file not found, generating..." << endl;
    dout.reset(new std::ofstream(arguments.data_path,
                                 std::ios::out | std::ios::binary));
    int type1, type2;
    for (type1 = 0; type1 < 4; ++type1) {
      for (type2 = 0; type2 < 4; ++type2) {
        struct game_state state1 = new_game(type1, type2);
        double winChance = eval_state(state1);
        cerr << type_names[type1] << " v " << type_names[type2] << '\t'
             << winChance << endl;
      }
    }
    dout.reset();
  } else {
    // Read from data file
    int nstates;
    union {
      int state;
      char state_[sizeof(state)];
    };
    union {
      double winchance;
      char winchance_[sizeof(winchance)];
    };
    for (nstates = 0; ; nstates++) {
      data_file.read(state_, sizeof(state));
      data_file.read(winchance_, sizeof(winchance));
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
    cerr << "Read " << nstates << " states from data file" << endl;
  }
  cerr << "Finished initializing." << endl;
  moonburst_loop();
}
