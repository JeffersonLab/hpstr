#include "las3pluslas6_FitFunction.h"
#include "FunctionMath.h"

double las3pluslas6_FitFunction::calculateBackground(double *x, double *par) {
    /*double xp = FitFunction::getCorrectedX(x[0]);*/
    return FunctionMath::las3pluslas6_Function(x[0], par);//, order);
}
