#include "ua23nolinpluslas1_FitFunction.h"
#include "FunctionMath.h"

double ua23nolinpluslas1_FitFunction::calculateBackground(double *x, double *par) {
    /*double xp = FitFunction::getCorrectedX(x[0]);*/
    return FunctionMath::ua23nolinpluslas1_Function(x[0], par);//, order);
}
