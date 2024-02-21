#ifndef __UA23NOLINPLUSLAS1_FUNC_H__
#define __UA23NOLINPLUSLAS1_FUNC_H__

#include "FitFunction.h"

/**
 * @brief description
 * 
 * details
 */
class ua23nolinpluslas1_FitFunction: public FitFunction {
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
