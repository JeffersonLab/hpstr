#include <ExpChebyshevFitFunction.h>
#include <FunctionMath.h>

double ExpChebyshevFitFunction::calculateBackground(double *x, double *par) {
	double xp = FitFunction::getCorrectedX(x[0]);
	if(model_order == FitFunction::ModelOrder::FIRST) {
		return FunctionMath::Order1ExpBackgroundFunction(xp, par[0], par[1]);
	} else if(model_order == FitFunction::ModelOrder::THIRD) {
		return FunctionMath::Order3ExpBackgroundFunction(xp, par[0], par[1], par[2], par[3]);
	} else if(model_order == FitFunction::ModelOrder::FIFTH) {
		return FunctionMath::Order5ExpBackgroundFunction(xp, par[0], par[1], par[2], par[3], par[4], par[5]);
	} else if(model_order == FitFunction::ModelOrder::SEVENTH) {
		return FunctionMath::Order7ExpBackgroundFunction(xp, par[0], par[1], par[2], par[3], par[4], par[5], par[6], par[7]);
	} else {
		return 0.0;
	}
}