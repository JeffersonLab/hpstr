#include <HpsFitResult.h>

HpsFitResult::HpsFitResult() 
    : 
      q0_(0),  
      p_value_(0),
      upper_limit_(0) { 
}

HpsFitResult::~HpsFitResult() { }

double HpsFitResult::getFullBkgRate() {
    double bkgRate = -1.0;
    double fitParams[6];
    double mass[1];
    mass[0] = mass_hypo_;
    for(int ipar = 0; ipar < poly_order_+1; ipar++) {
        fitParams[ipar] = comp_result_->GetParams()[ipar];
    }
    
    // Determine the type of background fit model.
    bool isChebyshev = (bkg_model_ == FitFunction::BkgModel::CHEBYSHEV || bkg_model_ == FitFunction::BkgModel::EXP_CHEBYSHEV);
    bool isExp = (bkg_model_ == FitFunction::BkgModel::EXP_CHEBYSHEV || bkg_model_ == FitFunction::BkgModel::EXP_LEGENDRE);
    
    // Create the background function.
    if(poly_order_ == 1 && isChebyshev) {
        ChebyshevFitFunction bkg_func(mass_hypo_, window_size_, bin_width_, FitFunction::ModelOrder::FIRST, FitFunction::SignalFitModel::NONE, isExp);
        bkgRate = bkg_func(mass, fitParams);
    } else if(poly_order_ == 1 && !isChebyshev) {
        LegendreFitFunction bkg_func(mass_hypo_, window_size_, bin_width_, FitFunction::ModelOrder::FIRST, FitFunction::SignalFitModel::NONE, isExp);
        bkgRate = bkg_func(mass, fitParams);
    } else if(poly_order_ == 3 && isChebyshev) {
        ChebyshevFitFunction bkg_func(mass_hypo_, window_size_, bin_width_, FitFunction::ModelOrder::THIRD, FitFunction::SignalFitModel::NONE, isExp);
        bkgRate = bkg_func(mass, fitParams);
    } else if(poly_order_ == 3 && !isChebyshev) {
        LegendreFitFunction bkg_func(mass_hypo_, window_size_, bin_width_, FitFunction::ModelOrder::THIRD, FitFunction::SignalFitModel::NONE, isExp);
        bkgRate = bkg_func(mass, fitParams);
    } else if(poly_order_ == 5 && isChebyshev) {
        ChebyshevFitFunction bkg_func(mass_hypo_, window_size_, bin_width_, FitFunction::ModelOrder::FIFTH, FitFunction::SignalFitModel::NONE, isExp);
        bkgRate = bkg_func(mass, fitParams);
    } else {
        LegendreFitFunction bkg_func(mass_hypo_, window_size_, bin_width_, FitFunction::ModelOrder::FIFTH, FitFunction::SignalFitModel::NONE, isExp);
        bkgRate = bkg_func(mass, fitParams);
    }

    return bkgRate;
}

double HpsFitResult::getFullBkgRateError() {
    return comp_result_->GetErrors()[0];
}
