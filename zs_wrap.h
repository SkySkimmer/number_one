#ifndef ZS_WRAP_H_
#define ZS_WRAP_H_

// With reference to the strategic form, this function solves for the strategy
// for the column player, i.e. the player with choice set {A, B, C} below
// The payoffs matrix should contain the column player's payoffs plus a
// constant which makes them nonnegative (in this case, it would be 30)
// in this case, {0, 40, 10, 20, 10, 50} - you may want/need to add a larger value
// if one of your rows / columns is entirely zero.
// Return value is a newly allocated array; first element is the value of the
// game for the other player, followed by the weights of the column player's
// actions, or NULL if the solve failed for any reason.
/*      A               B               C
1       30, -30         -10, 10         20, -20
2       10, -10         20, -20         -20, 20
 */
double* zs_solve(int nRows, int nCols, double *payoffMatrix);

//double *zs_solve_rev(int nRows, int nCols, double *payoffMatrix);

#endif  // ZS_WRAP_H_
