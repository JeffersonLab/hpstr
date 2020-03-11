#ifndef __CHEBYSHEV_FUNC_H__
#define __CHEBYSHEV_FUNC_H__

#include "FitFunction.h"

class ChebyshevFitFunction: public FitFunction {
    using FitFunction::FitFunction;

    protected:
    double calculateBackground(double *x, double *par);
};

#endif
