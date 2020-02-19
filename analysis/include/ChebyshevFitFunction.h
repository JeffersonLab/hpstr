#include <FitFunction.h>

class ChebyshevFitFunction: public FitFunction {
    using FitFunction::FitFunction;

    protected:
    double calculateBackground(double *x, double *par);
};
