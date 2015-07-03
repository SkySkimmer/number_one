#ifndef ZS_WRAP_H_
#define ZS_WRAP_H_

// With reference to the strategic form, this function solves for the Nash
// Equilibrium strategy for the column player, i.e. the player with choice
// set {A, B, C} below.
// The payoffs matrix should contain the column player's payoffs plus a
// constant which makes them positive, e.g. 35 in this case.
// Return value is a newly allocated array; first element is the value of the
// game for the other player, followed by the weights of the column player's
// actions. If the solve failed for any reason, nullptr is returned.
/*      A               B               C
1       30, -30         -10, 10         20, -20
2       10, -10         20, -20         -20, 20
 */
double* zs_solve(int nRows, int nCols, double *payoffMatrix);

#endif  // ZS_WRAP_H_
