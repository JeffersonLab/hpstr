
/** 
 * @file BumpHunter.cxx
 * @brief
 * @author Omar Moreno <omoreno1@ucsc.edu>
 *         Santa Cruz Institute for Particle Physics
 *         University of California, Santa Cruz
 * @date January 14, 2015
 *
 */

#include <BumpHunter.h>

BumpHunter::BumpHunter(BkgModel model, int poly_order, int res_factor) 
    : ofs(nullptr),
      res_factor_(res_factor), 
      poly_order_(poly_order) {

}

BumpHunter::~BumpHunter() {
}

void BumpHunter::initialize(TH1* histogram, double &mass_hypothesis) { 

    mass_hypothesis_ = mass_hypothesis; 

    // Shift the mass hypothesis so it sits in the middle of a bin.  
    std::cout << "[ BumpHunter ]: Shifting mass hypothesis to nearest bin center " 
              << mass_hypothesis << " ---> "; 
    int mbin = histogram->GetXaxis()->FindBin(mass_hypothesis); 
    mass_hypothesis = histogram->GetXaxis()->GetBinCenter(mbin);
    std::cout << mass_hypothesis << " GeV" << std::endl;

    // If the mass hypothesis is below the lower bound, throw an exception.  A 
    // search cannot be performed using an invalid value for the mass hypothesis.
    if (mass_hypothesis < lower_bound_) {
        throw std::runtime_error("Mass hypothesis less than the lower bound!"); 
    }

    // Correct the mass to take into account the mass scale systematic
    corr_mass_ = this->correctMass(mass_hypothesis);

    // Get the mass resolution at the corrected mass 
    mass_resolution_ = this->getMassResolution(corr_mass_);
    std::cout << "[ BumpHunter ]: Mass resolution: " << mass_resolution_ << " GeV" << std::endl;

    // Calculate the fit window size
    window_size_ = mass_resolution_*res_factor_;
    this->printDebug("Window size: " + std::to_string(window_size_));

    // Find the starting position of the window. This is set to the low edge of 
    // the bin closest to the calculated value. If the start position falls 
    // below the lower bound of the histogram, set it to the lower bound.
    window_start_ = mass_hypothesis - window_size_/2;
    int window_start_bin = histogram->GetXaxis()->FindBin(window_start_);  
    window_start_ = histogram->GetXaxis()->GetBinLowEdge(window_start_bin);
    if (window_start_ < lower_bound_) { 
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
    window_end_ = window_start_ + window_size_;
    int window_end_bin = histogram->GetXaxis()->FindBin(window_end_);
    window_end_ = histogram->GetXaxis()->GetBinUpEdge(window_end_bin);
    if (window_end_ > upper_bound_) { 
        std::cout << "[ BumpHunter ]: Upper edge of window (" << window_end_ 
                  << " GeV) is above upper bound." << std::endl;
        window_end_bin = histogram->GetXaxis()->FindBin(upper_bound_);
        
        int last_bin_above = histogram->FindLastBinAbove(); 
        if (window_end_bin > last_bin_above) window_end_bin = last_bin_above; 
        
        window_end_ = histogram->GetXaxis()->GetBinUpEdge(window_end_bin);
        window_start_bin = histogram->GetXaxis()->FindBin(window_end_ - window_size_);  
        window_start_ = histogram->GetXaxis()->GetBinLowEdge(window_start_bin);
    }
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
    this->initialize(histogram, mass_hypothesis);

    // Instantiate a new fit result object to store all of the results.
    HpsFitResult* fit_result = new HpsFitResult(); 
    fit_result->setPolyOrder(poly_order_); 

    // If not fitting toys, start by performing a background only fit.
    if (!skip_bkg_fit)  {
         
        // Start by performing a background only fit.  The results from this fit 
        // are used to get an intial estimate of the background parameters.  
        std::cout << "*************************************************" << std::endl;
        std::cout << "*************************************************" << std::endl;
        std::cout << "[ BumpHunter ]:  Performing background only fits." << std::endl;
        std::cout << "*************************************************" << std::endl;
        std::cout << "*************************************************" << std::endl;
   
        //
        TF1* bkg{nullptr}; 
        TF1* bkg_toys{nullptr}; 
        if (poly_order_ == 1) 
        { 
            
            //
            ExpPol1BkgFunction bkg_func(mass_hypothesis, window_end_ - window_start_); 
            bkg = new TF1("bkg", bkg_func, -1, 1, 2);

            //
            bkg->SetParameters(4,0);
            bkg->SetParNames("pol0","pol1");

            //Set bkg function fit for toys to come from next polynomial order
            ExpPol3BkgFunction bkg_toy_func(mass_hypothesis, window_end_ - window_start_); 
            bkg_toys = new TF1("bkg_toys", bkg_toy_func, -1, 1, 4);

            //
            bkg_toys->SetParameters(4,0,0,0);
            bkg_toys->SetParNames("pol0","pol1","pol2","pol3");
        } 
        else if (poly_order_ == 3) 
        { 
            
            //
            ExpPol3BkgFunction bkg_func(mass_hypothesis, window_end_ - window_start_); 
            bkg = new TF1("bkg", bkg_func, -1, 1, 4);

            //
            bkg->SetParameters(4,0,0,0);
            bkg->SetParNames("pol0","pol1","pol2","pol3");

            //
            ExpPol5BkgFunction bkg_toy_func(mass_hypothesis, window_end_ - window_start_); 
            bkg_toys = new TF1("bkg_toys", bkg_toy_func, -1, 1, 6);

            //
            bkg_toys->SetParameters(4,0,0,0,0,0);
            bkg_toys->SetParNames("pol0","pol1","pol2","pol3","pol4","pol5");
        } 
        else 
        { 
            //
            ExpPol5BkgFunction bkg_func(mass_hypothesis, window_end_ - window_start_); 
            bkg = new TF1("bkg", bkg_func, -1, 1, 6);

            //
            bkg->SetParameters(4,0,0,0,0,0);
            bkg->SetParNames("pol0","pol1","pol2","pol3","pol4","pol5");

            //
            ExpPol7BkgFunction bkg_toy_func(mass_hypothesis, window_end_ - window_start_); 
            bkg_toys = new TF1("bkg_toys", bkg_toy_func, -1, 1, 8);

            //
            bkg_toys->SetParameters(4,0,0,0,0,0,0,0);
            bkg_toys->SetParNames("pol0","pol1","pol2","pol3","pol4","pol5","pol6","pol7");
        }

        TFitResultPtr result = histogram->Fit("bkg", "LES+", "", window_start_, window_end_); 
        fit_result->setBkgFitResult(result); 
        //result->Print();
        TFitResultPtr result_toys = histogram->Fit("bkg_toys", "LES+", "", window_start_, window_end_); 
        fit_result->setBkgToysFitResult(result_toys); 

    }
         
    std::cout << "***************************************************" << std::endl;
    std::cout << "***************************************************" << std::endl;
    std::cout << "[ BumpHunter ]: Performing a signal+background fit." << std::endl;
    std::cout << "***************************************************" << std::endl;
    std::cout << "***************************************************" << std::endl;
    
    TF1* full{nullptr};  
    if (poly_order_ == 1) 
    { 
        ExpPol1FullFunction full_func(mass_hypothesis, window_end_ - window_start_); 
        full = new TF1("full", full_func, -1, 1, 5);
    
        full->SetParameters(4,0,0,0,0);
        full->SetParNames("pol0","pol1","signal norm","mean","sigma");
        full->FixParameter(3,0.0); 
        full->FixParameter(4, mass_resolution_); 
    } 
    else if (poly_order_ == 3) 
    { 
        ExpPol3FullFunction full_func(mass_hypothesis, window_end_ - window_start_); 
        full = new TF1("full", full_func, -1, 1, 7);
    
        full->SetParameters(4,0,0,0,0,0,0);
        full->SetParNames("pol0","pol1","pol2","pol3","signal norm","mean","sigma");
        full->FixParameter(5,0.0); 
        full->FixParameter(6, mass_resolution_); 
    } 
    else 
    { 
        ExpPol5FullFunction full_func(mass_hypothesis, window_end_ - window_start_); 
        full = new TF1("full", full_func, -1, 1, 9);
    
        full->SetParameters(4,0,0,0,0,0,0,0,0);
        full->SetParNames("pol0","pol1","pol2","pol3","pol4","pol5","signal norm","mean","sigma");
        full->FixParameter(7,0.0); 
        full->FixParameter(8, mass_resolution_); 
    }    
       
    TFitResultPtr full_result = histogram->Fit("full", "LES+", "", window_start_, window_end_);
    fit_result->setCompFitResult(full_result); 
    //full_result->Print();

    this->calculatePValue(fit_result);
    std::cout << "[ BumpHunter ]: Bkg Fit Status: " << fit_result->getBkgFitResult()->IsValid() <<  std::endl;
    std::cout << "[ BumpHunter ]: Bkg Toys Fit Status: " << fit_result->getBkgToysFitResult()->IsValid() <<  std::endl;
    std::cout << "[ BumpHunter ]: Full Fit Status: " << fit_result->getCompFitResult()->IsValid() <<  std::endl;
    if((!skip_ul) && full_result->IsValid() ) this->getUpperLimit(histogram, fit_result);
    
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

    //
    double signal_yield = result->getCompFitResult()->Parameter(poly_order_ + 1); 
    this->printDebug("Signal yield: " + std::to_string(signal_yield)); 

    // In searching for a resonance, a signal is expected to lead to an 
    // excess of events.  In this case, a signal yield of < 0 is  
    // meaningless so we set the p-value = 1.  This follows the formulation
    // put forth by Cowen et al. in https://arxiv.org/pdf/1007.1727.pdf. 
    if (signal_yield <= 0) { 
        result->setPValue(1);
        this->printDebug("Signal yield is negative ... setting p-value = 1"); 
        return; 
    }

    // Get the NLL obtained by minimizing the composite model with the signal
    // yield floating.
    double mle_nll = result->getCompFitResult()->MinFcnValue(); 
    this->printDebug("NLL when mu = " + std::to_string(signal_yield) + ": " + std::to_string(mle_nll));

    // Get the NLL obtained from the Bkg only fit.
    double cond_nll = result->getBkgFitResult()->MinFcnValue(); 
    printDebug("NLL when mu = 0: " + std::to_string(cond_nll));

    // 1) Calculate the likelihood ratio which is chi2 distributed. 
    // 2) From the chi2, calculate the p-value.
    double q0 = 0; 
    double p_value = 0; 
    this->getChi2Prob(cond_nll, mle_nll, q0, p_value);  

    std::cout << "[ BumpHunter ]: p-value: " << p_value << std::endl;
    
    // Update the result
    result->setPValue(p_value);
    result->setQ0(q0);  
    
}

void BumpHunter::printDebug(std::string message) { 
    std::cout << "[ BumpHunter ]: " << message << std::endl;
    //if (debug) std::cout << "[ BumpHunter ]: " << message << std::endl;
}

void BumpHunter::getUpperLimit(TH1* histogram, HpsFitResult* result) {
    
    TF1* comp{nullptr};
    if (poly_order_ == 3) { 
  
        ExpPol3FullFunction comp_func(mass_hypothesis_, window_end_ - window_start_); 
        comp = new TF1("comp_ul", comp_func, -1, 1, 7);
    
        comp->SetParameters(4,0,0,0,0,0,0);
        comp->SetParNames("pol0","pol1","pol2","pol3","signal norm","mean","sigma");
        comp->FixParameter(5,0.0); 
        comp->FixParameter(6, mass_resolution_); 
        
    } else { 
       
        ExpPol5FullFunction comp_func(mass_hypothesis_, window_end_ - window_start_); 
        comp = new TF1("comp_ul", comp_func, -1, 1, 9);
    
        comp->SetParameters(4,0,0,0,0,0,0,0,0);
        comp->SetParNames("pol0","pol1","pol2","pol3","pol4","pol5","signal norm","mean","sigma");
        comp->FixParameter(7,0.0); 
        comp->FixParameter(8, mass_resolution_);
   
    }    

    std::cout << "Mass resolution: " << mass_resolution_ << std::endl; 
    std::cout << "[ BumpHunter ]: Calculating upper limit." << std::endl;

    //  Get the signal yield obtained from the signal+bkg fit
    double signal_yield = result->getCompFitResult()->Parameter(poly_order_ + 1); 
    this->printDebug("Signal yield: " + std::to_string(signal_yield)); 

    // Get the minimum NLL value that will be used for testing upper limits.
    // If the signal yield (mu estimator) at the min NLL is < 0, use the NLL
    // obtained when mu = 0.
    double mle_nll = result->getCompFitResult()->MinFcnValue(); 

    if (signal_yield < 0) {
        this->printDebug("Signal yield @ min NLL is < 0. Using NLL when signal yield = 0");

        // Get the NLL obtained assuming the background only hypothesis
        mle_nll = result->getBkgFitResult()->MinFcnValue(); 
        
        signal_yield = 0;
    } 
    this->printDebug("MLE NLL: " + std::to_string(mle_nll));   

    signal_yield = floor(signal_yield) + 1; 

    double p_value = 1;
    double q0 = 0; 
    while(true) {
       
        this->printDebug("Setting signal yield to: " + std::to_string(signal_yield));
        //std::cout << "[ BumpHunter ]: Current p-value: " << p_value << std::endl;
        comp->FixParameter(poly_order_ + 1, signal_yield); 
        
        TFitResultPtr full_result 
            = histogram->Fit("comp_ul", "LES+", "", window_start_, window_end_);
        double cond_nll = full_result->MinFcnValue(); 

        // 1) Calculate the likelihood ratio which is chi2 distributed. 
        // 2) From the chi2, calculate the p-value.
        this->getChi2Prob(cond_nll, mle_nll, q0, p_value);  

        this->printDebug("p-value after fit : " + std::to_string(p_value)); 
        std::cout << "[ BumpHunter ]: Current Signal Yield: " << signal_yield << std::endl;
        std::cout << "[ BumpHunter ]: Current p-value: " << p_value << std::endl;
        
        if ((p_value <= 0.0455 && p_value > 0.044)) { 
            
            std::cout << "[ BumpHunter ]: Upper limit: " << signal_yield << std::endl;
            std::cout << "[ BumpHunter ]: p-value: " << p_value << std::endl;

            result->setUpperLimit(signal_yield);
            result->setUpperLimitPValue(p_value); 
     
            break; 
        } 
        else if (signal_yield <= 0 && p_value < 0.044) 
        {
            std::cout << "[ BumpHunter ]: Caution Background Model suspicious!" << std::endl;
            std::cout << "[ BumpHunter ]: Upper limit: " << signal_yield << std::endl;
            std::cout << "[ BumpHunter ]: p-value: " << p_value << std::endl;

            result->setUpperLimit(signal_yield);
            result->setUpperLimitPValue(p_value); 
     
            break; 
        }
        else if (p_value <= 0.044) signal_yield -= 1;
        else if (p_value <= 0.059) signal_yield += 10;
        else if (p_value <= 0.10) signal_yield += 20;
        else if (p_value <= 0.2) signal_yield += 40; 
        else signal_yield += 100;  
    }
}

std::vector<TH1*> BumpHunter::generateToys(TH1* histogram, double n_toys, int seed, int toy_sig_samples) { 

    gRandom->SetSeed(seed); 

    TF1* bkg_toys = histogram->GetFunction("bkg_toys");
    TF1* sig_toys = new TF1("sig_toys", "gaus(0)", window_start_, window_end_);
    // AMPLITUDE = 1 / sqrt(SIGMA * 2 * pi)
    // MU = mass_hypothesis_
    // SIGMA = mass_resolution_ / 2
    sig_toys->SetParameters(1 / sqrt(mass_resolution_ * 2 * 3.14159265358979323846264338), mass_hypothesis_, mass_resolution_);

    std::vector<TH1*> hists;
    for (int itoy = 0; itoy < n_toys; ++itoy) { 
        std::string name = "invariant_mass_" + std::to_string(itoy); 
        if(itoy%100 == 0) std::cout << "Generating Toy " << itoy << std::endl;
        TH1F* hist = new TH1F(name.c_str(), name.c_str(), bins_, window_start_, window_end_);
        for (int i = 0; i < int(integral_); ++i) { 
            hist->Fill(bkg_toys->GetRandom(window_start_, window_end_)); 
        }
        for(int i = 0; i < toy_sig_samples; i++) {
            hist->Fill(sig_toys->GetRandom(window_start_, window_end_));
        }
        hists.push_back(hist); 
    }

    return hists;
}


void BumpHunter::getChi2Prob(double cond_nll, double mle_nll, double &q0, double &p_value) {
   

    this->printDebug("Cond NLL: " + std::to_string(cond_nll)); 
    this->printDebug("Uncod NLL: " + std::to_string(mle_nll));  
    double diff = cond_nll - mle_nll;
    this->printDebug("Delta NLL: " + std::to_string(diff));
    
    q0 = 2*diff;
    this->printDebug("q0: " + std::to_string(q0));
    
    p_value = ROOT::Math::chisquared_cdf_c(q0, 1);
    this->printDebug("p-value before dividing: " + std::to_string(p_value));  
    p_value *= 0.5;
    this->printDebug("p-value: " + std::to_string(p_value)); 
}

void BumpHunter::setBounds(double lower_bound, double upper_bound) {
    lower_bound_ = lower_bound; 
    upper_bound_ = upper_bound;
    printf("Fit bounds set to [ %f , %f ]\n", lower_bound_, upper_bound_);   
}

double BumpHunter::correctMass(double mass) { 
    double offset = -1.19892320e4*pow(mass, 3) + 1.50196798e3*pow(mass,2) 
                    - 8.38873712e1*mass + 6.23215746; 
    offset /= 100; 
    this->printDebug("Offset: " + std::to_string(offset)); 
    double cmass = mass - mass*offset; 
    this->printDebug("Corrected Mass: " + std::to_string(cmass)); 
    return cmass;
}

/**
 * BkgFunction
 */

//
// TODO: Move the classes externally
//

ExpPol1BkgFunction::ExpPol1BkgFunction(double mass_hypothesis, double window_size)
    : mass_hypothesis_(mass_hypothesis), 
      window_size_(window_size) { 
}

double ExpPol1BkgFunction::operator() (double* x, double* par) { 
    
    double xp = (x[0] - mass_hypothesis_)/(window_size_*2.0); 
  
    // Chebyshevs between given limits
    double t0 = par[0];
    double t1 = par[1]*xp;
  
    double pol = t0+t1;

    return TMath::Power(10,pol);
}

ExpPol1FullFunction::ExpPol1FullFunction(double mass_hypothesis, double window_size)
    : mass_hypothesis_(mass_hypothesis), 
      window_size_(window_size) { 
}


double ExpPol1FullFunction::operator() (double* x, double* par) { 
    
    double xp = (x[0] - mass_hypothesis_)/(window_size_*2.0); 
  
    // Chebyshevs between given limits
    double t0 = par[0];
    double t1 = par[1]*xp;
  
    double pol = t0+t1;

    double gauss = (1.0)/(sqrt(2.0*TMath::Pi()*pow(par[4],2))) *
        TMath::Exp( - pow((xp-par[3]),2)/(2.0*pow(par[4],2)) );
  
    return TMath::Power(10,pol)+0.0001*par[2]*gauss;
}

ExpPol3BkgFunction::ExpPol3BkgFunction(double mass_hypothesis, double window_size)
    : mass_hypothesis_(mass_hypothesis), 
      window_size_(window_size) { 
}

double ExpPol3BkgFunction::operator() (double* x, double* par) { 
    
    double xp = (x[0] - mass_hypothesis_)/(window_size_*2.0); 
  
    // Chebyshevs between given limits
    double t0 = par[0];
    double t1 = par[1]*xp;
    double t2 = par[2]*(2*xp*xp - 1);
    double t3 = par[3]*(4*xp*xp*xp - 3*xp);
  
    double pol = t0+t1+t2+t3;

    return TMath::Power(10,pol);
}

ExpPol3FullFunction::ExpPol3FullFunction(double mass_hypothesis, double window_size)
    : mass_hypothesis_(mass_hypothesis), 
      window_size_(window_size) { 
}


double ExpPol3FullFunction::operator() (double* x, double* par) { 
    
    double xp = (x[0] - mass_hypothesis_)/(window_size_*2.0); 
  
    // Chebyshevs between given limits
    double t0 = par[0];
    double t1 = par[1]*xp;
    double t2 = par[2]*(2*xp*xp - 1);
    double t3 = par[3]*(4*xp*xp*xp - 3*xp);
  
    double pol = t0+t1+t2+t3;

    double gauss = (1.0)/(sqrt(2.0*TMath::Pi()*pow(par[6],2))) *
        TMath::Exp( - pow((xp-par[5]),2)/(2.0*pow(par[6],2)) );
  
    return TMath::Power(10,pol)+0.0001*par[4]*gauss;
}

ExpPol5BkgFunction::ExpPol5BkgFunction(double mass_hypothesis, double window_size)
    : mass_hypothesis_(mass_hypothesis), 
      window_size_(window_size) { 
}

double ExpPol5BkgFunction::operator() (double* x, double* par) { 
    
    double xp = (x[0] - mass_hypothesis_)/(window_size_*2.0); 
  
    // Chebyshevs between given limits
    double t0 = par[0];
    double t1 = par[1]*xp;
    double t2 = par[2]*(2*xp*xp - 1);
    double t3 = par[3]*(4*xp*xp*xp - 3*xp);
    double t4 = par[4]*(8*xp*xp*xp*xp - 8*xp*xp + 1);
    double t5 = par[5]*(16*xp*xp*xp*xp*xp - 20*xp*xp*xp + 5*xp);
  
    double pol = t0+t1+t2+t3+t4+t5;

    return TMath::Power(10,pol);
}

ExpPol5FullFunction::ExpPol5FullFunction(double mass_hypothesis, double window_size)
    : mass_hypothesis_(mass_hypothesis), 
      window_size_(window_size) { 
}

double ExpPol5FullFunction::operator() (double* x, double* par) { 
    
    double xp = (x[0] - mass_hypothesis_)/(window_size_*2.0); 
  
    // Chebyshevs between given limits
    double t0 = par[0];
    double t1 = par[1]*xp;
    double t2 = par[2]*(2*xp*xp - 1);
    double t3 = par[3]*(4*xp*xp*xp - 3*xp);
    double t4 = par[4]*(8*xp*xp*xp*xp - 8*xp*xp + 1);
    double t5 = par[5]*(16*xp*xp*xp*xp*xp - 20*xp*xp*xp + 5*xp);
  
    double pol = t0+t1+t2+t3+t4+t5;

    double gauss = (1.0)/(sqrt(2.0*TMath::Pi()*pow(par[8],2))) *
        TMath::Exp( - pow((xp-par[7]),2)/(2.0*pow(par[8],2)) );
  
    return TMath::Power(10,pol)+0.0001*par[6]*gauss;
}

ExpPol7BkgFunction::ExpPol7BkgFunction(double mass_hypothesis, double window_size)
    : mass_hypothesis_(mass_hypothesis), 
      window_size_(window_size) { 
}

double ExpPol7BkgFunction::operator() (double* x, double* par) { 
    
    double xp = (x[0] - mass_hypothesis_)/(window_size_*2.0); 
  
    // Chebyshevs between given limits
    double t0 = par[0];
    double t1 = par[1]*xp;
    double t2 = par[2]*(2*xp*xp - 1);
    double t3 = par[3]*(4*xp*xp*xp - 3*xp);
    double t4 = par[4]*(8*xp*xp*xp*xp - 8*xp*xp + 1);
    double t5 = par[5]*(16*xp*xp*xp*xp*xp - 20*xp*xp*xp + 5*xp);
    double t6 = par[6]*(32*xp*xp*xp*xp*xp*xp - 48*xp*xp*xp*xp + 18*xp*xp - 1);
    double t7 = par[7]*(64*xp*xp*xp*xp*xp*xp*xp - 112*xp*xp*xp*xp*xp + 56*xp*xp*xp - 7*xp);
  
    double pol = t0+t1+t2+t3+t4+t5+t6+t7;

    return TMath::Power(10,pol);
}

