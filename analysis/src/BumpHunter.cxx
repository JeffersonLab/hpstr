
/** 
 * @file BumpHunter.cxx
 * @brief
 * @author Omar Moreno <omoreno1@ucsc.edu>
 *         Santa Cruz Institute for Particle Physics
 *         University of California, Santa Cruz
 * @date January 14, 2015
 *
 */

#include "BumpHunter.h"

BumpHunter::BumpHunter(FitFunction::BkgModel model, int poly_order, int toy_poly_order, int res_factor, double res_scale, bool asymptotic_limit)
    : ofs(nullptr),
      res_factor_(res_factor), 
      poly_order_(poly_order),
      toy_poly_order_(toy_poly_order),
      asymptotic_limit_(asymptotic_limit),
      res_scale_(res_scale),
      bkg_model_(model) { }

BumpHunter::~BumpHunter() { }

void BumpHunter::initialize(TH1* histogram, double &mass_hypothesis) {
    mass_hypothesis_ = mass_hypothesis; 

    //Set Minuit Minimizer options
    ROOT::Math::MinimizerOptions::SetDefaultStrategy(2);
    
    // Shift the mass hypothesis so it sits in the middle of a bin.  
    std::cout << "[ BumpHunter ]: Shifting mass hypothesis to nearest bin center "
              << mass_hypothesis << " ---> "; 
    int mbin = histogram->GetXaxis()->FindBin(mass_hypothesis);
    bin_width_ = histogram->GetBinWidth(2);
    std::cout << "[ BumpHunter ]: Histogram Bin Width is " << bin_width_ << std::endl;
    mass_hypothesis = histogram->GetXaxis()->GetBinCenter(mbin);
    std::cout << mass_hypothesis << " GeV" << std::endl;
    
    // If the mass hypothesis is below the lower bound, throw an exception.  A 
    // search cannot be performed using an invalid value for the mass hypothesis.
    if(mass_hypothesis < lower_bound_) {
        throw std::runtime_error("Mass hypothesis less than the lower bound!");
    }
    
    // Correct the mass to take into account the mass scale systematic
    //corr_mass_ = correctMass(mass_hypothesis);
    //corr_mass_ = mass_hypothesis;
    
    // Get the mass resolution at the corrected mass 
    mass_resolution_ = getMassResolution(mass_hypothesis_);
    std::cout << "[ BumpHunter ]: Mass resolution: " << mass_resolution_ << " GeV" << std::endl;
    
    // Calculate the fit window size
    window_size_ = mass_resolution_*res_factor_;
    printDebug("Window size: " + std::to_string(window_size_));
    
    // Find the starting position of the window. This is set to the low edge of 
    // the bin closest to the calculated value. If the start position falls 
    // below the lower bound of the histogram, set it to the lower bound.
    window_start_ = mass_hypothesis - window_size_/2;
    int window_start_bin = histogram->GetXaxis()->FindBin(window_start_);
    window_start_ = histogram->GetXaxis()->GetBinLowEdge(window_start_bin);
    if(window_start_ < lower_bound_) {
        std::cout << "[ BumpHunter ]: Starting edge of window (" << window_start_
                  << " MeV) is below lower bound." << std::endl;
        window_start_bin = histogram->GetXaxis()->FindBin(lower_bound_);
        window_start_ = histogram->GetXaxis()->GetBinLowEdge(window_start_bin);
    }
    
    // Find the end position of the window.  This is set to the lower edge of 
    // the bin closest to the calculated value. If the window edge falls above
    // the upper bound of the histogram, set it to the upper bound.
    // Furthermore, check that the bin serving as the upper boundary contains
    // events. If the upper bound is shifted, reset the lower window bound.
    window_end_ = mass_hypothesis + window_size_/2;
    int window_end_bin = histogram->GetXaxis()->FindBin(window_end_);
    window_end_ = histogram->GetXaxis()->GetBinUpEdge(window_end_bin);
    if(window_end_ > upper_bound_) {
        std::cout << "[ BumpHunter ]: Upper edge of window (" << window_end_
                  << " GeV) is above upper bound." << std::endl;
        window_end_bin = histogram->GetXaxis()->FindBin(upper_bound_);
        
        int last_bin_above = histogram->FindLastBinAbove();
        if(window_end_bin > last_bin_above) window_end_bin = last_bin_above;
        
        window_end_ = histogram->GetXaxis()->GetBinUpEdge(window_end_bin);
        window_start_bin = histogram->GetXaxis()->FindBin(window_end_ - window_size_);
        window_start_ = histogram->GetXaxis()->GetBinLowEdge(window_start_bin);
    }
    window_size_ = window_end_ - window_start_;
    bins_ = window_end_bin - window_start_bin + 1;
    std::cout << "[ BumpHunter ]: Setting starting edge of fit window to "
              << window_start_ << " GeV. Bin " << window_start_bin << std::endl;
    std::cout << "[ BumpHunter ]: Setting upper edge of fit window to "
              << window_end_ << " GeV. Bin " << window_end_bin << std::endl;
    std::cout << "[ BumpHunter ]: Total Number of Bins: " << bins_ << std::endl;
    
    // Estimate the background normalization within the window by integrating
    // the histogram within the window range.  This should be close to the 
    // background yield in the case where there is no signal present.
    integral_ = histogram->Integral(window_start_bin, window_end_bin);
    std::cout << "[ BumpHunter ]: Integral within window: " << integral_ << std::endl;
}

HpsFitResult* BumpHunter::performSearch(TH1* histogram, double mass_hypothesis, bool skip_bkg_fit, bool skip_ul) {
    // Calculate all of the fit parameters e.g. window size, mass hypothesis
    initialize(histogram, mass_hypothesis);
    double initNorm = log10(integral_);
    
    // Instantiate a new fit result object to store all of the results.
    HpsFitResult* fit_result = new HpsFitResult();
    fit_result->setPolyOrder(poly_order_);
    fit_result->setBkgModelType(bkg_model_);

    // Determine whether to use an exponential polynomial or normal polynomial.
    bool isChebyshev = (bkg_model_ == FitFunction::BkgModel::CHEBYSHEV || bkg_model_ == FitFunction::BkgModel::EXP_CHEBYSHEV);
    bool isExp = (bkg_model_ == FitFunction::BkgModel::EXP_CHEBYSHEV || bkg_model_ == FitFunction::BkgModel::EXP_LEGENDRE);
    TF1* bkg{nullptr};
    TF1* bkg_toys{nullptr};
        
    std::cout << "Defining fit functions." << std::endl;
    std::cout << "    Model :: ";
    if(bkg_model_ == FitFunction::BkgModel::CHEBYSHEV) { std::cout << "Chebyshev Polynomial" << std::endl; }
    else if(bkg_model_ == FitFunction::BkgModel::EXP_CHEBYSHEV) { std::cout << "Exponential Chebyshev Polynomial" << std::endl; }
    else if(bkg_model_ == FitFunction::BkgModel::LEGENDRE) { std::cout << "Legendre Polynomial" << std::endl; }
    else if(bkg_model_ == FitFunction::BkgModel::EXP_LEGENDRE) { std::cout << "Exponential Legendre Polynomial" << std::endl; }
    
    std::cout << "    Background Order :: ";
    FitFunction::ModelOrder bkg_order_model;
    if(poly_order_ == 1) {
        std::cout << "1" << std::endl;
        bkg_order_model = FitFunction::ModelOrder::FIRST;
    } else if(poly_order_ == 3) {
        std::cout << "3" << std::endl;
        bkg_order_model = FitFunction::ModelOrder::THIRD;
    } else if(poly_order_ == 5) {
        std::cout << "5" << std::endl;
        bkg_order_model = FitFunction::ModelOrder::FIFTH;
    }
    
    std::cout << "    Toy Generator Order :: ";
    FitFunction::ModelOrder toy_order_model;
    if(toy_poly_order_ == 1) {
        std::cout << "1" << std::endl;
        toy_order_model = FitFunction::ModelOrder::FIRST;
    } else if(toy_poly_order_ == 3) {
        std::cout << "3" << std::endl;
        toy_order_model = FitFunction::ModelOrder::THIRD;
    } else if(toy_poly_order_ == 5) {
        std::cout << "5" << std::endl;
        toy_order_model = FitFunction::ModelOrder::FIFTH;
    } else if(toy_poly_order_ == 7) {
        std::cout << "7" << std::endl;
        toy_order_model = FitFunction::ModelOrder::SEVENTH;
    }
    
    // If not fitting toys, start by performing a background only fit.
    if(!skip_bkg_fit) {
        // Start by performing a background only fit.  The results from this fit 
        // are used to get an intial estimate of the background parameters.
        std::cout << "*************************************************" << std::endl;
        std::cout << "*************************************************" << std::endl;
        std::cout << "[ BumpHunter ]:  Performing background only fits." << std::endl;
        std::cout << "*************************************************" << std::endl;
        std::cout << "*************************************************" << std::endl;
        
        // Define the background-only fit model.
        if(isChebyshev) {
            ChebyshevFitFunction bkg_func(mass_hypothesis, window_end_ - window_start_, bin_width_, bkg_order_model, FitFunction::SignalFitModel::NONE, isExp);
            bkg = new TF1("bkg", bkg_func, -1, 1, poly_order_ + 1);
        } else {
            LegendreFitFunction bkg_func(mass_hypothesis, window_end_ - window_start_, bin_width_, bkg_order_model, FitFunction::SignalFitModel::NONE, isExp);
            bkg = new TF1("bkg", bkg_func, -1, 1, poly_order_ + 1);
        }
        bkg->SetParameter(0, initNorm);
        bkg->SetParName(0, "pol0");
        for(int i = 1; i < poly_order_ + 1; i++) {
            bkg->SetParameter(i, 0);
            bkg->SetParName(i, Form("pol%i", i));
        }
        
        // Define the toy generator fit model.
        if(isChebyshev) {
            ChebyshevFitFunction bkg_toy_func(mass_hypothesis, window_end_ - window_start_, bin_width_, bkg_order_model, FitFunction::SignalFitModel::NONE, isExp);
            bkg_toys = new TF1("bkg_toys", bkg_toy_func, -1, 1, toy_poly_order_ + 1);
        } else {
            LegendreFitFunction bkg_toy_func(mass_hypothesis, window_end_ - window_start_, bin_width_, bkg_order_model, FitFunction::SignalFitModel::NONE, isExp);
            bkg_toys = new TF1("bkg_toys", bkg_toy_func, -1, 1, toy_poly_order_ + 1);
        }
        bkg_toys->SetParameter(0, initNorm);
        bkg_toys->SetParName(0, "pol0");
        for(int i = 1; i < toy_poly_order_ + 1; i++) {
            bkg_toys->SetParameter(i, 0);
            bkg_toys->SetParName(i, Form("pol%i", i));
        }
        
        // Perform the background-only fit and store the result.
        TFitResultPtr result = histogram->Fit("bkg", "VLES+", "", window_start_, window_end_);
        fit_result->setBkgFitResult(result);

        std::cout << "*************************************************" << std::endl;
        std::cout << "*************************************************" << std::endl;
        std::cout << "[ BumpHunter ]:  Performing background Toy fit." << std::endl;
        std::cout << "*************************************************" << std::endl;
        std::cout << "*************************************************" << std::endl;

        // Perform the toy model fit and store the result.
        TFitResultPtr result_toys = histogram->Fit("bkg_toys", "VLES+", "", window_start_, window_end_);
        fit_result->setBkgToysFitResult(result_toys);
    }
    
    std::cout << "***************************************************" << std::endl;
    std::cout << "***************************************************" << std::endl;
    std::cout << "[ BumpHunter ]: Performing a signal+background fit." << std::endl;
    std::cout << "***************************************************" << std::endl;
    std::cout << "***************************************************" << std::endl;
    
    TF1* full{nullptr};
    
    // Define the background+signal fit model.
    if(isChebyshev) {
        ChebyshevFitFunction full_func(mass_hypothesis, window_end_ - window_start_, bin_width_, bkg_order_model, FitFunction::SignalFitModel::GAUSSIAN, isExp);
        full = new TF1("full", full_func, -1, 1, poly_order_ + 4);
    } else {
        LegendreFitFunction full_func(mass_hypothesis, window_end_ - window_start_, bin_width_, bkg_order_model, FitFunction::SignalFitModel::GAUSSIAN, isExp);
        full = new TF1("full", full_func, -1, 1, poly_order_ + 4);
    }
    full->SetParameter(0, initNorm);
    full->SetParName(0, "pol0");
    full->SetParameter(poly_order_ + 1, 0.0);
    full->SetParName(poly_order_ + 1, "signal norm");
    full->SetParName(poly_order_ + 2, "mean");
    full->SetParName(poly_order_ + 3, "sigma");
    for(int i = 1; i < poly_order_ + 1; i++) {
        full->SetParameter(i, 0);
        full->SetParName(i, Form("pol%i", i));
    }
    full->FixParameter(poly_order_ + 2, mass_hypothesis);
    full->FixParameter(poly_order_ + 3, mass_resolution_);
    
    for(int parI = 0; parI < poly_order_ + 1; parI++) {
        full->SetParameter(parI, bkg->GetParameter(parI));
    }
    TFitResultPtr full_result = histogram->Fit("full", "VLES+", "", window_start_, window_end_);
    fit_result->setCompFitResult(full_result);
    
    calculatePValue(fit_result);
    std::cout << "[ BumpHunter ]: Bkg Fit Status: " << fit_result->getBkgFitResult()->IsValid() <<  std::endl;
    std::cout << "[ BumpHunter ]: Bkg Toys Fit Status: " << fit_result->getBkgToysFitResult()->IsValid() <<  std::endl;
    std::cout << "[ BumpHunter ]: Full Fit Status: " << fit_result->getCompFitResult()->IsValid() <<  std::endl;
    if((!skip_ul) && full_result->IsValid()) { getUpperLimit(histogram, fit_result); }
    
    // Persist the mass hypothesis used for this fit
    fit_result->setMass(mass_hypothesis_);
    
    // Persist the mass after correcting for the mass scale
    fit_result->setCorrectedMass(corr_mass_);
    
    // Set the window size
    fit_result->setWindowSize(window_size_);
    
    // Set the total number of events within the window
    fit_result->setIntegral(integral_);
    
    return fit_result;
}

void BumpHunter::calculatePValue(HpsFitResult* result) {
    std::cout << "[ BumpHunter ]: Calculating p-value: " << std::endl;
    
    double signal_yield = result->getCompFitResult()->Parameter(poly_order_ + 1);
    printDebug("Signal yield: " + std::to_string(signal_yield));
    
    // In searching for a resonance, a signal is expected to lead to an 
    // excess of events.  In this case, a signal yield of < 0 is  
    // meaningless so we set the p-value = 1.  This follows the formulation
    // put forth by Cowen et al. in https://arxiv.org/pdf/1007.1727.pdf. 
    if(signal_yield <= 0) {
        result->setPValue(1);
        printDebug("Signal yield is negative ... setting p-value = 1");
        return;
    }
    
    // Get the NLL obtained by minimizing the composite model with the signal
    // yield floating.
    double mle_nll = result->getCompFitResult()->MinFcnValue();
    printDebug("NLL when mu = " + std::to_string(signal_yield) + ": " + std::to_string(mle_nll));
    
    // Get the NLL obtained from the Bkg only fit.
    double cond_nll = result->getBkgFitResult()->MinFcnValue();
    printDebug("NLL when mu = 0: " + std::to_string(cond_nll));
    
    // 1) Calculate the likelihood ratio which is chi2 distributed.
    // 2) From the chi2, calculate the p-value.
    double q0 = 0;
    double p_value = 0;
    getChi2Prob(cond_nll, mle_nll, q0, p_value);
    
    std::cout << "[ BumpHunter ]: p-value: " << p_value << std::endl;
    
    // Update the result
    result->setPValue(p_value);
    result->setQ0(q0);
}

void BumpHunter::printDebug(std::string message) {
    std::cout << "[ BumpHunter ]: " << message << std::endl;
}

void BumpHunter::getUpperLimit(TH1* histogram, HpsFitResult* result) {
    if(asymptotic_limit_) {
        BumpHunter::getUpperLimitAsymptotic(histogram, result);
    } else {
        BumpHunter::getUpperLimitPower(histogram, result);
    }
}

void BumpHunter::getUpperLimitAsymptotic(TH1* histogram, HpsFitResult* result) {
    std::cout << "[ BumpHunter ]: Calculating upper limit." << std::endl;
    
    // Get the signal yield and signal yield error.
    double signal_yield = result->getCompFitResult()->Parameter(poly_order_ + 1);
    if(signal_yield < 0) { signal_yield = 0; }
    double signal_yield_error = result->getCompFitResult()->ParError(poly_order_ + 1);
    
    // Debug print the signal yield and its error.
    std::cout << "Signal Yield       :: " << signal_yield << std::endl;
    std::cout << "Signal Yield Error :: " << signal_yield_error << std::endl;
    
    // Calculate the upper limit according to Equation (69) of "Asymptotic formulae
    // for likelihood-based tests of new physics" by Cowan et alii. 1.64 is derived
    // from Equation (1) for a 95% confidence level (alpha = 0.05).
    double upper_limit = signal_yield + 1.64 * signal_yield_error;
    
    // The p-value is 0.05 by definition.
    double p_value = 0.05;
    
    // Debug print the upper limit.
    std::cout << "Upper Limit        :: " << upper_limit << std::endl;
    std::cout << "p-Value            :: " << p_value << std::endl;
    
    // Set the upper limit and upper limit p-value.
    result->setUpperLimit(upper_limit);
    result->setUpperLimitPValue(p_value);
}

void BumpHunter::getUpperLimitPower(TH1* histogram, HpsFitResult* result) {
    // Determine whether to use an exponential polynomial or normal polynomial.
    bool isChebyshev = (bkg_model_ == FitFunction::BkgModel::CHEBYSHEV || bkg_model_ == FitFunction::BkgModel::EXP_CHEBYSHEV);
    bool isExp = (bkg_model_ == FitFunction::BkgModel::EXP_CHEBYSHEV || bkg_model_ == FitFunction::BkgModel::EXP_LEGENDRE);
    double initNorm = log10(integral_);
    
    // Instantiate a fit function for the appropriate polynomial order.
    TF1* comp{nullptr};
    FitFunction::ModelOrder bkg_order_model;
    if(poly_order_ == 1) { bkg_order_model = FitFunction::ModelOrder::FIRST; }
    else if(poly_order_ == 3) { bkg_order_model = FitFunction::ModelOrder::THIRD; }
    else if(poly_order_ == 5) { bkg_order_model = FitFunction::ModelOrder::FIFTH; }
    if(isChebyshev) {
        ChebyshevFitFunction comp_func(mass_hypothesis_, window_end_ - window_start_, bin_width_, bkg_order_model, FitFunction::SignalFitModel::GAUSSIAN, isExp);
        comp = new TF1("comp_ul", comp_func, -1, 1, poly_order_ + 4);
    } else {
        LegendreFitFunction comp_func(mass_hypothesis_, window_end_ - window_start_, bin_width_, bkg_order_model, FitFunction::SignalFitModel::GAUSSIAN, isExp);
        comp = new TF1("comp_ul", comp_func, -1, 1, poly_order_ + 4);
    }
    comp->SetParameter(0, initNorm);
    comp->SetParName(0, "pol0");
    comp->SetParameter(poly_order_ + 1, 0.0);
    comp->SetParName(poly_order_ + 1, "signal norm");
    comp->SetParName(poly_order_ + 2, "mean");
    comp->SetParName(poly_order_ + 3, "sigma");
    for(int i = 1; i < poly_order_ + 1; i++) {
        comp->SetParameter(i, 0);
        comp->SetParName(i, Form("pol%i", i));
    }
    comp->FixParameter(poly_order_ + 2, mass_hypothesis_);
    comp->FixParameter(poly_order_ + 3, mass_resolution_);
    
    std::cout << "Mass resolution: " << mass_resolution_ << std::endl;
    std::cout << "[ BumpHunter ]: Calculating upper limit." << std::endl;
    
    //  Get the signal yield obtained from the signal+bkg fit
    double signal_yield = result->getCompFitResult()->Parameter(poly_order_ + 1);
    printDebug("Signal yield: " + std::to_string(signal_yield));
    
    // Get the minimum NLL value that will be used for testing upper limits.
    // If the signal yield (mu estimator) at the min NLL is < 0, use the NLL
    // obtained when mu = 0.
    double mle_nll = result->getCompFitResult()->MinFcnValue();
    
    if(signal_yield < 0) {
        printDebug("Signal yield @ min NLL is < 0. Using NLL when signal yield = 0");
        
        // Get the NLL obtained assuming the background only hypothesis
        mle_nll = result->getBkgFitResult()->MinFcnValue();
        
        signal_yield = 0;
    }
    printDebug("MLE NLL: " + std::to_string(mle_nll));
    
    signal_yield = floor(signal_yield) + 1;
    
    double p_value = 1;
    double q0 = 0;
    while(true) {
        printDebug("Setting signal yield to: " + std::to_string(signal_yield));
        //std::cout << "[ BumpHunter ]: Current p-value: " << p_value << std::endl;
        comp->FixParameter(poly_order_ + 1, signal_yield);
        
        TFitResultPtr full_result = histogram->Fit("comp_ul", "VLES+", "", window_start_, window_end_);
        double cond_nll = full_result->MinFcnValue();
        
        // 1) Calculate the likelihood ratio which is chi2 distributed.
        // 2) From the chi2, calculate the p-value.
        getChi2Prob(cond_nll, mle_nll, q0, p_value);
        
        printDebug("p-value after fit : " + std::to_string(p_value));
        std::cout << "[ BumpHunter ]: Current Signal Yield: " << signal_yield << std::endl;
        std::cout << "[ BumpHunter ]: Current p-value: " << p_value << std::endl;
        
        if((p_value <= 0.0455 && p_value > 0.044)) {
            std::cout << "[ BumpHunter ]: Upper limit: " << signal_yield << std::endl;
            std::cout << "[ BumpHunter ]: p-value: " << p_value << std::endl;
            
            result->setUpperLimit(signal_yield);
            result->setUpperLimitPValue(p_value);
            
            break;
        } else if (signal_yield <= 0 && p_value < 0.044) {
            std::cout << "[ BumpHunter ]: Caution Background Model suspicious!" << std::endl;
            std::cout << "[ BumpHunter ]: Upper limit: " << signal_yield << std::endl;
            std::cout << "[ BumpHunter ]: p-value: " << p_value << std::endl;
            
            result->setUpperLimit(signal_yield);
            result->setUpperLimitPValue(p_value);
            
            break;
        }
        else if(p_value <= 0.044) { signal_yield -= 1; }
        else if(p_value <= 0.059) { signal_yield += 10; }
        else if(p_value <= 0.10) { signal_yield += 20; }
        else if(p_value <= 0.2) { signal_yield += 40; }
        else { signal_yield += 100; }
    }
}

std::vector<TH1*> BumpHunter::generateToys(TH1* histogram, double n_toys, int seed, int toy_sig_samples, int bkg_mult, TH1* signal_hist) {
    gRandom->SetSeed(seed);
    
    TF1* bkg_toys = histogram->GetFunction("bkg_toys");
    TF1* sig_toys = new TF1("sig_toys", "gaus", window_start_, window_end_);
    sig_toys->SetParameters(1.0, mass_hypothesis_, mass_resolution_);
    
    std::vector<TH1*> hists;
    int bkg_events = bkg_mult * int(integral_);
    for(int itoy = 0; itoy < n_toys; ++itoy) {
        std::string name = "invariant_mass_" + std::to_string(itoy);
        if(itoy%100 == 0) {
            std::cout << "Generating Toy " << itoy << std::endl;
        }
        TH1F* hist = new TH1F(name.c_str(), name.c_str(), bins_, window_start_, window_end_);
        for(int i = 0; i < bkg_events; ++i) {
            hist->Fill(bkg_toys->GetRandom(window_start_, window_end_));
        }
        for(int i = 0; i < toy_sig_samples; i++) {
            double sig_sample = 0;
            if(signal_hist != NULL) { sig_sample = signal_hist->GetRandom(); }
            else { sig_sample = sig_toys->GetRandom(window_start_, window_end_); }
            hist->Fill(sig_sample);
        }
        hists.push_back(hist); 
    }
    
    return hists;
}

void BumpHunter::getChi2Prob(double cond_nll, double mle_nll, double &q0, double &p_value) {
    printDebug("Cond NLL: " + std::to_string(cond_nll));
    printDebug("Uncod NLL: " + std::to_string(mle_nll));
    double diff = cond_nll - mle_nll;
    printDebug("Delta NLL: " + std::to_string(diff));
    
    q0 = 2 * diff;
    printDebug("q0: " + std::to_string(q0));
    
    p_value = ROOT::Math::chisquared_cdf_c(q0, 1);
    printDebug("p-value before dividing: " + std::to_string(p_value));
    p_value *= 0.5;
    printDebug("p-value: " + std::to_string(p_value));
}

void BumpHunter::setBounds(double lower_bound, double upper_bound) {
    lower_bound_ = lower_bound;
    upper_bound_ = upper_bound;
    printf("Fit bounds set to [ %f , %f ]\n", lower_bound_, upper_bound_);
}

double BumpHunter::correctMass(double mass) {
    double offset = -1.19892320e4 * pow(mass, 3) + 1.50196798e3 * pow(mass, 2) - 8.38873712e1 * mass + 6.23215746;
    offset /= 100;
    printDebug("Offset: " + std::to_string(offset));
    double cmass = mass - mass * offset;
    printDebug("Corrected Mass: " + std::to_string(cmass));
    return cmass;
}
