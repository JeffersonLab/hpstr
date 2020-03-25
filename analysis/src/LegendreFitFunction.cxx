#include "LegendreFitFunction.h"
#include "FunctionMath.h"

double LegendreFitFunction::calculateBackground(double *x, double *par) {
    double xp = FitFunction::getCorrectedX(x[0]);
    return FunctionMath::LegendreFunction(xp, par, order);
}