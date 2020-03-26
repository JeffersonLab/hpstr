#ifndef __LEGENDRE_FUNC_H__
#define __LEGENDRE_FUNC_H__

#include "FitFunction.h"

class LegendreFitFunction: public FitFunction {
    using FitFunction::FitFunction;

    protected:
    double calculateBackground(double *x, double *par);
};

#endif