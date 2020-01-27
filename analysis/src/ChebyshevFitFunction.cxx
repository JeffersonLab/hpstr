#include <ChebyshevFitFunction.h>
#include <FunctionMath.h>

double ChebyshevFitFunction::calculateBackground(double *x, double *par) {
    double xp = FitFunction::getCorrectedX(x[0]);
    return FunctionMath::ChebyshevFunction(xp, par, order);
}
