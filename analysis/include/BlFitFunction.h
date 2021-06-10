#ifndef BLFITFUNCTION_H
#define BLFITFUNCTION_H
#include <TMath.h>
#include <FunctionMath.h>
#include <iostream>

class BlFitFunction {
    public: 

        BlFitFunction(double dummy){dummyVar_ = dummy;};

        double operator() (double *x, double *par) {
            return FunctionMath::Gaussian(x[0], par[0], par[1], par[2])+FunctionMath::HeavisideFunction(x[0],par[3])*FunctionMath::Landau(x[0],par[4],par[5],par[6], par[7]);
        };

    private:

        double dummyVar_{2.0};


};

#endif
