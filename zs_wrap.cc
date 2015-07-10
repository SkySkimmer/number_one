// Solves a zero-sum game using lp_solve
#include "zs_wrap.h"

#include <lp_solve/lp_lib.h>
#include <cstdlib>
#include <iostream>


double* zs_solve_(int nRows, int nCols, double *payoffMatrix) {
  lprec *lp;
  lp = make_lp(0, nCols);
  set_verbose(lp, IMPORTANT);
  // Objective function is simply the sum of all the variables
  double *tmp = reinterpret_cast<double *>(malloc((nCols + 1)*sizeof(double)));
  for (int i = 1; i < nCols + 1; ++i) {
    tmp[i] = 1;
  }
  set_obj_fn(lp, tmp);
  set_add_rowmode(lp, 1);
  for (int i = 0; i < nRows; ++i) {
    add_constraint(lp, payoffMatrix + (i * nCols) - 1, GE, 1);
  }
  set_add_rowmode(lp, 0);
  int ret = solve(lp);
  if (ret == 0) {
    get_variables(lp, tmp + 1);
    delete_lp(lp);
    tmp[0] = 0;
    for (int i = 1; i < nCols + 1; ++i) {
      tmp[0] += tmp[i];
    }
    for (int i = 1; i < nCols + 1; ++i) {
      tmp[i] /= tmp[0];
    }
    tmp[0] = 1 / tmp[0];
    return tmp;
  }
  delete_lp(lp);
  free(tmp);

  // We need to be cleverer here
  return NULL;
}

double* zs_solve(int nRows, int nCols, double *payoffMatrix) {
  // reverse every row of the payoff matrix (i mean yeah, i could reverse
  // the columns too but there's not any point to it)
  // #totallytestedcode #yolo
  double *revPayoffMatrix = reinterpret_cast<double *>(
                              malloc(nRows * nCols * sizeof(double)));
  for (int i = 0; i < nRows; ++i) {
    // copy reversed row to buffer
    for (int j = 0; j < nCols; ++j) {
      revPayoffMatrix[i * nCols + j] = payoffMatrix[(i + 1) * nCols - j - 1];
    }
  }
  // send it off to already existing thing
  double *result = zs_solve_(nRows, nCols, revPayoffMatrix);
  free(revPayoffMatrix);
  // then reverse the probability distribution before sending it back
  double *revresult = reinterpret_cast<double *>(
                        malloc((nCols + 1) * sizeof(double)));
  revresult[0] = result[0];
  for (int i = 1; i <= nCols; ++i) {
    revresult[i] = result[nCols + 1 - i];
  }
  free(result);
  return revresult;
}
