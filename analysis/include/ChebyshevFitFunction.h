#ifndef __CHEBYSHEV_FUNC_H__
#define __CHEBYSHEV_FUNC_H__

#include "FitFunction.h"

/**
 * @brief description
 * 
 */
class ChebyshevFitFunction: public FitFunction {
    using FitFunction::FitFunction;

    protected:
        /**
         * @brief calculate background
         * 
         * @param x 
         * @param par 
         * @return double 
         */
        double calculateBackground(double *x, double *par);
};

#endif
