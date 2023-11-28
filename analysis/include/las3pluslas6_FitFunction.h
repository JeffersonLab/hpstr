#ifndef __LAS3PLUSLAS6_FUNC_H__
#define __LAS3PLUSLAS6_FUNC_H__

#include "FitFunction.h"

/**
 * @brief description
 * 
 * details
 */
class las3pluslas6_FitFunction: public FitFunction {
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
