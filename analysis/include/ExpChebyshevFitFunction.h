#include <FitFunction.h>

class ExpChebyshevFitFunction: public FitFunction {
	using FitFunction::FitFunction;
	
	protected:
		double calculateBackground(double *x, double *par);
};