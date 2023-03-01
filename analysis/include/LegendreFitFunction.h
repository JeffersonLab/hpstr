#ifndef __LEGENDRE_FUNC_H__
#define __LEGENDRE_FUNC_H__

#include "FitFunction.h"

/**
 * @brief description
 * 
 * details
 */
class LegendreFitFunction: public FitFunction {
    using FitFunction::FitFunction;

    protected:
        /**
         * @brief description
         * 
         * @param x 
         * @param par 
         * @return double 
         */
        double calculateBackground(double *x, double *par);
};

#endif