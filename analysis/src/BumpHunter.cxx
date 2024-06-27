
/** 
 * @file BumpHunter.cxx
 * @brief
 * @author Cam "The Man" Bravo, Omar Moreno, Emrys Peets
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
    bkg_model_(model) { 
    }

BumpHunter::~BumpHunter() {
}

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

    if(bkg_model_ < 4)
    {
        // Calculate the fit window size
        window_size_ = 0;
        if(window_use_res_scale_) { window_size_ = res_factor_ * mass_resolution_; }
        else { window_size_ = res_factor_ * this->getMassResolution(mass_hypothesis_, 1.0); }
        this->printDebug("Window size: " + std::to_string(window_size_));

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
    else
    {
        // Calculate the fit window size for the global fit function case
        window_size_ = 0;
        //if(window_use_res_scale_) { window_size_ = res_factor_ * mass_resolution_; }
        //else { window_size_ = res_factor_ * this->getMassResolution(mass_hypothesis_, 1.0); }
        //this->printDebug("Window size: " + std::to_string(window_size_));

        // Find the starting position of the window. This is set to the low edge of 
        // the bin closest to the calculated value. If the start position falls 
        // below the lower bound of the histogram, set it to the lower bound.

        //setting starting position of window 
        window_start_ = lower_bound_ ; //mass_hypothesis - window_size_/2;
        int window_start_bin = histogram->GetXaxis()->FindBin(window_start_);
        window_start_ = histogram->GetXaxis()->GetBinLowEdge(window_start_bin);
        std::cout << "[ BumpHunter ]: Window Start " << window_start_ << std::endl;
        std::cout << "[ BumpHunter ]: Lower Bound " << lower_bound_ << std::endl;
        std::cout << "[ BumpHunter ]: Window Start Bin " << window_start_bin << std::endl;


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

        //setting end position of fit window
        window_end_ = upper_bound_ ;//mass_hypothesis + window_size_/2;
        int window_end_bin = histogram->GetXaxis()->FindBin(window_end_);
        window_end_ = histogram->GetXaxis()->GetBinUpEdge(window_end_bin);

        if(window_end_ > upper_bound_) {
            std::cout << "[ BumpHunter ]: Upper edge of window (" << window_end_
                << " GeV) is above upper bound." << std::endl;
            window_end_bin = histogram->GetXaxis()->FindBin(upper_bound_);

            int last_bin_above = histogram->FindLastBinAbove();
            if(window_end_bin > last_bin_above) window_end_bin = last_bin_above;

            window_end_ = histogram->GetXaxis()->GetBinUpEdge(window_end_bin);
        }

        window_size_ = window_end_ - window_start_;
        bins_ = window_end_bin - window_start_bin + 1;
        std::cout << "[ BumpHunter ]: Setting starting edge of fit window to "
            << window_start_ << " GeV. Bin " << window_start_bin << std::endl;
        std::cout << "[ BumpHunter ]: Setting upper edge of fit window to "
            << window_end_ << " GeV. Bin " << window_end_bin << std::endl;
        std::cout << "[ BumpHunter ]: Total Number of Bins: " << bins_ << std::endl;
        std::cout << "[ BumpHunter ]: Window Start " << window_start_ << std::endl;
        std::cout << "[ BumpHunter ]: Window End " << window_end_ << std::endl;

        // Estimate the background normalization within the window by integrating
        // the histogram within the window range.  This should be close to the 
        // background yield in the case where there is no signal present.
        integral_ = histogram->Integral(window_start_bin, window_end_bin);
        std::cout << "[ BumpHunter ]: Integral within window: " << integral_ << std::endl;
    }
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
    bool isLegendre = (bkg_model_ == FitFunction::BkgModel::LEGENDRE|| bkg_model_ == FitFunction::BkgModel::EXP_LEGENDRE); 
    
    bool isL3L6 = (bkg_model_ == FitFunction::BkgModel::LAS3PLUSLAS6);
    bool isUA23L1 = (bkg_model_ == FitFunction::BkgModel::UA23NOLINPLUSLAS1);    

    //setting poly order here 
    //if (isLegendre or isChebyshev) {
    //fit_result->setPolyOrder(poly_order_);
    //}    




    TF1* bkg{nullptr};
    TF1* bkg_toys{nullptr};

    std::cout << "Defining fit functions." << std::endl;
    std::cout << "    Model :: ";
    if(bkg_model_ == FitFunction::BkgModel::CHEBYSHEV) { std::cout << "Chebyshev Polynomial" << std::endl; }
    else if(bkg_model_ == FitFunction::BkgModel::EXP_CHEBYSHEV) { std::cout << "Exponential Chebyshev Polynomial" << std::endl; }
    else if(bkg_model_ == FitFunction::BkgModel::LEGENDRE) { std::cout << "Legendre Polynomial" << std::endl; }
    else if(bkg_model_ == FitFunction::BkgModel::EXP_LEGENDRE) { std::cout << "Exponential Legendre Polynomial" << std::endl; }
    else if(bkg_model_ == FitFunction::BkgModel::LAS3PLUSLAS6) { std::cout << "LAS3PLUSLAS6 Global" << std::endl; }
    else if(bkg_model_ == FitFunction::BkgModel::UA23NOLINPLUSLAS1) { std::cout << "UA23NOLINPLUSLAS1 Global" << std::endl; }

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
    } else if (poly_order_ == 10){
        std::cout << "10 Parameter Global Fit" << std::endl;
        bkg_order_model = FitFunction::ModelOrder::GLOBAL_L3L6;
    } else if (poly_order_ == 11){
        std::cout << "11 Parameter Global Fit" << std::endl;
        bkg_order_model = FitFunction::ModelOrder::GLOBAL_UA23L1;
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
    } else if (poly_order_ == 10){
        std::cout << "10 Parameter Global Fit" << std::endl;
        toy_order_model = FitFunction::ModelOrder::GLOBAL_L3L6;
    } else if (poly_order_ == 11){
        std::cout << "11 Parameter Global Fit" << std::endl;
        toy_order_model = FitFunction::ModelOrder::GLOBAL_UA23L1;
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
        } else if(isLegendre){
            LegendreFitFunction bkg_func(mass_hypothesis, window_end_ - window_start_, bin_width_, bkg_order_model, FitFunction::SignalFitModel::NONE, isExp);
            bkg = new TF1("bkg", bkg_func, -1, 1, poly_order_ + 1);
        }
        else if(isL3L6) {
            las3pluslas6_FitFunction bkg_func(mass_hypothesis, window_end_ - window_start_, bin_width_, bkg_order_model, FitFunction::SignalFitModel::NONE, isExp);
            bkg = new TF1("bkg", bkg_func, window_start_, window_end_, poly_order_ );

            //For the global bkg only model. Function= las3_plus_las6. Using the 10 parameters found on [45,200] MeV that found a pvalue of 5.8e-2 as initial parameters for fitting.
            //batch4 == normalization factor so must multiply the values of p2/pz in the p7 spot to effectively facor parameter out
            //in vatch15, fixing parameters found using 10% stats and leaving rest floating, following parameters found using 100% 
	    //bkg->FixParameter(0, 0.02655677447001521);
	    //bkg->FixParameter(1, 0.09575583442743552);
            //bkg->SetParameter(2, 1.6087608867103269e-06);
            //bkg->SetParameter(3, -12.14155381679078);
            //bkg->SetParameter(4, -9.88122176150782);
            //bkg->SetParameter(5, -0.015730267362833915);
            //bkg->SetParameter(6, 0.11327528231496534);
            //bkg->SetParameter(7, -14701589.955451723 / 1.6087608867103269e-06);
            //bkg->SetParameter(8, 117.94823473423622);
            //bkg->SetParameter(9, 423.73510122988904);
	    //batch15
	   
//	    bkg->FixParameter(0, 0.029668437523186);//FIXED
//	    bkg->FixParameter(1, 0.0922408532764407);//FIXED
//            bkg->SetParameter(2, 1.54 * 0.0253709666560375); //didnt change for batch 15
//           bkg->SetParameter(3, 6.48437396149796);
//            bkg->SetParameter(4, -6.39615451477062);//fix cause param convergence in trials
//            bkg->SetParameter(5, -0.138545315472865);//FIXED
//            bkg->SetParameter(6, -0.0901415989807506);//FIXED 
//            bkg->SetParameter(7, -996916.824047899 /0.00253709666560375);//changed and scaled for  batch 15, may need to change if it doesnt work well
//            bkg->SetParameter(8, 62.1873792846988); 
//            bkg->SetParameter(9, 205.916571373631);


//batch 17 fix param 0,1,4 due to change in params
	   //for chi2 prob ~0.094846 
	    //bkg->FixParameter(0, 0.030074818877556784);//FIXED batch 15 //set otherwise
	    //bkg->FixParameter(1, 0.0922332252618532);//FIXED
            //bkg->SetParameter(2, 1.54 * 0.09270220202361732); //didnt change for batch 15
            //bkg->SetParameter(3, 10.19565177187765);
//            bkg->SetParameter(4, -5.999178486707799);//fixed for batch 16
//            bkg->FixParameter(5, -0.03555422309280177);//FIXED batch 15
//            bkg->FixParameter(6, -0.030448353969406552);//FIXED 
            //bkg->SetParameter(7, -1516706.7987999977 / 0.009270220202361732);//changed and scaled for  batch 15, may need to change if it doesnt work well
            //bkg->SetParameter(8, 12.301046515690146); 
            //bkg->SetParameter(9, 903.1742716581055);



//parameters for 2015 dataset
            bkg->SetParameter(0,0.014814933712322965);
            bkg->SetParameter(1, 0.04671130069437565);
            bkg->SetParameter(2, 4.163195400663928e-05);
            bkg->SetParameter(3, 19.946858289834015);
            bkg->SetParameter(4, -7.121171310011385);
            bkg->SetParameter(5, -0.015759484411453376);
            bkg->SetParameter(6, 0.04480879026813586);
            bkg->SetParameter(7, -6249834.189121319 / 4.163195400663928e-05);
            bkg->SetParameter(8, 219.5505731792011);
            bkg->SetParameter(9, 2117.5604367053106);











        } else if(isUA23L1) {    
            //for the ua23nolinpluslas1 global fit bkg model
            ua23nolinpluslas1_FitFunction bkg_func(mass_hypothesis, window_end_ - window_start_, bin_width_, bkg_order_model, FitFunction::SignalFitModel::NONE, isExp);
            bkg = new TF1("bkg", bkg_func, window_start_, window_end_, poly_order_);

            //setting parameters to equal ones found through round 2 testing
            bkg->SetParameter(0, 129634.98200470296);
            bkg->SetParameter(1, -12174541.73288088);
            bkg->SetParameter(2, -1225773499.545076);//global normalization constant
            bkg->SetParameter(3, -300.6625940555103);
            bkg->SetParameter(4, 3204.4146073560496);
            bkg->SetParameter(5, -13964.461541062823);
            bkg->SetParameter(6, 0.023788450274793472);
            bkg->SetParameter(7, 0.09466552532518363);
            bkg->SetParameter(8, 2.7023479034248536 / -1225773499.545076);
            bkg->SetParameter(9, 2.9879442300854024);
            bkg->SetParameter(10, 1.9088571089322375);
        }

        //For the first two bkg-only models
        if(isChebyshev || isLegendre) {

            bkg->SetParameter(0, initNorm);
            bkg->SetParName(0, "pol0");
            for(int i = 1; i < poly_order_ + 1; i++) {
                bkg->SetParameter(i, 0);
                bkg->SetParName(i, Form("pol%i", i));
            }
        }
        // Define the toy generator fit model.
        if(isChebyshev) {
            ChebyshevFitFunction bkg_toy_func(mass_hypothesis, window_end_ - window_start_, bin_width_, toy_order_model, FitFunction::SignalFitModel::NONE, isExp);
            bkg_toys = new TF1("bkg_toys", bkg_toy_func, -1, 1, toy_poly_order_ + 1);
        } else if(isLegendre) {
            LegendreFitFunction bkg_toy_func(mass_hypothesis, window_end_ - window_start_, bin_width_, toy_order_model, FitFunction::SignalFitModel::NONE, isExp);
            bkg_toys = new TF1("bkg_toys", bkg_toy_func, -1, 1, toy_poly_order_ + 1);
        } else if(isL3L6) {
            //for the las3pluslas6 global fit toy model
            las3pluslas6_FitFunction bkg_toy_func(mass_hypothesis, window_end_ - window_start_, bin_width_, toy_order_model, FitFunction::SignalFitModel::NONE, isExp);
            bkg_toys = new TF1("bkg_toys", bkg_toy_func, window_start_, window_end_, toy_poly_order_);

            //setting parameters to equal ones found through round 2 testing
//            bkg_toys->FixParameter(0, 0.02655677447001521);
//            bkg_toys->FixParameter(1, 0.09575583442743552);
//            bkg_toys->SetParameter(2, 1.6087608867103269e-06);
//            bkg_toys->SetParameter(3, -12.14155381679078);
//            bkg_toys->SetParameter(4, -9.88122176150782);
//            bkg_toys->SetParameter(5, -0.015730267362833915);
//            bkg_toys->SetParameter(6, 0.11327528231496534);
//            bkg_toys->SetParameter(7, -14701589.955451723 / 1.6087608867103269e-06);
//            bkg_toys->SetParameter(8, 117.94823473423622);
//            bkg_toys->SetParameter(9, 423.73510122988904);
	//batch15 below, batch 17 will use a mix of parameters from different tests
//	    bkg_toys->FixParameter(0, 0.029668437523186);//FIXED
//	    bkg_toys->FixParameter(1, 0.0922408532764407);//FIXED
//            bkg_toys->SetParameter(2, 1.54 * 0.0253709666560375); //didnt change for batch 15
//            bkg_toys->SetParameter(3, 6.48437396149796);
//            bkg_toys->SetParameter(4, -6.39615451477062); //Fix because of parameter convergence maybe
//	    bkg_toys->SetParameter(5, -0.138545315472865);//FIXED
//            bkg_toys->SetParameter(6, -0.0901415989807506);//FIXED 
//            bkg_toys->SetParameter(7, -996916.824047899 /0.00253709666560375);//changed and scaled for  batch 15, may need to change if it doesnt work well
//            bkg_toys->SetParameter(8, 62.1873792846988); 
//            bkg_toys->SetParameter(9, 205.916571373631);
//batch 20 all but the two scaling jawns fixed cause y not
	    //batch 16 where i am now fixing 4 /// then i am trying fix 1, 4, 6
	    //bkg_toys->FixParameter(0, 0.030074818877556784);//
	    //bkg_toys->FixParameter(1, 0.0922332252618532);//FIXED
            //bkg_toys->SetParameter(2, 1.54 * 0.09270220202361732);//scaled to account for 6.5%
            //bkg_toys->SetParameter(3, 10.19565177187765);
            //bkg_toys->SetParameter(4, -5.999178486707799);//fixed as this param doesnt change a lot in tests
            //bkg_toys->SetParameter(5, -0.03555422309280177);//
            //bkg_toys->SetParameter(6, -0.030448353969406552);//FIXED 
            //bkg_toys->SetParameter(7, -1516706.7987999977 / 0.009270220202361732);
            //bkg_toys->SetParameter(8, 12.301046515690146); 
            //bkg_toys->SetParameter(9, 903.1742716581055);
//From chi2-prob ~0.0948461
	    //bkg_toys->FixParameter(0, 0.030074818877556784);//FIXED
	    //bkg_toys->FixParameter(1, 0.09484618731139761);//FIXED
            //bkg_toys->SetParameter(2, 0.0009270220202361732);
            //bkg_toys->SetParameter(3, 10.19565177187765);
            //bkg_toys->SetParameter(4, -5.999178486707799);
            //bkg_toys->FixParameter(5, -0.03555422309280177);//FIXED
            //bkg_toys->FixParameter(6, -0.030448353969406552);//FIXED 
            //bkg_toys->SetParameter(7, -1516706.7987999977 / 0.009270220202361732);
            //bkg_toys->SetParameter(8, 12.301046515690146); 
            //bkg_toys->SetParameter(9, 903.1742716581055);

//for 2015 data

	    bkg_toys->SetParameter(0,0.014814933712322965);
            bkg_toys->SetParameter(1, 0.04671130069437565);
            bkg_toys->SetParameter(2, 4.163195400663928e-05);
            bkg_toys->SetParameter(3, 19.946858289834015);
            bkg_toys->SetParameter(4, -7.121171310011385);
            bkg_toys->SetParameter(5, -0.015759484411453376);
            bkg_toys->SetParameter(6, 0.04480879026813586);
	    bkg_toys->SetParameter(7, -6249834.189121319 / 4.163195400663928e-05); 
            bkg_toys->SetParameter(8, 219.5505731792011);     
            bkg_toys->SetParameter(9, 2117.5604367053106);     
            //for the ua23nolinpluslas1 global fit toy model
	   } else if(isUA23L1) {    
	    ua23nolinpluslas1_FitFunction bkg_toy_func(mass_hypothesis, window_end_ - window_start_, bin_width_, toy_order_model, FitFunction::SignalFitModel::NONE, isExp);
	    bkg_toys = new TF1("bkg_toys", bkg_toy_func, window_start_, window_end_, toy_poly_order_);

            //setting parameters to those found through round 2 testing
            bkg_toys->SetParameter(0, 129634.98200470296);
            bkg_toys->SetParameter(1, -12174541.73288088);
            bkg_toys->SetParameter(2, -1225773499.545076);//global normalization constant
            bkg_toys->SetParameter(3, -300.6625940555103);
            bkg_toys->SetParameter(4, 3204.4146073560496);
            bkg_toys->SetParameter(5, -13964.461541062823);
            bkg_toys->SetParameter(6, 0.023788450274793472);
            bkg_toys->SetParameter(7, 0.09466552532518363);
            bkg_toys->SetParameter(8, 2.7023479034248536 / -1225773499.545076);
            bkg_toys->SetParameter(9, 2.9879442300854024);
            bkg_toys->SetParameter(10, 1.9088571089322375);
        }
        if(isChebyshev || isLegendre) {
            bkg_toys->SetParameter(0, initNorm);
            bkg_toys->SetParName(0, "pol0");
            for(int i = 1; i < toy_poly_order_ + 1; i++) {
                bkg_toys->SetParameter(i, 0);
                bkg_toys->SetParName(i, Form("pol%i", i));
            }
        }
        // Perform the background-only fit and store the result.

        TFitResultPtr result = histogram->Fit("bkg", "QLES+", "", window_start_, window_end_);
        fit_result->setBkgFitResult(result);

        std::cout << "*************************************************" << std::endl;
        std::cout << "*************************************************" << std::endl;
        std::cout << "[ BumpHunter ]:  Performing background Toy fit." << std::endl;
        std::cout << "*************************************************" << std::endl;
        std::cout << "*************************************************" << std::endl;

        // Perform the toy model fit and store the result.
        TFitResultPtr result_toys = histogram->Fit("bkg_toys", "QLES+", "", window_start_, window_end_);
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
        //setting up gaussian signal info 
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
        //need to remove the following conditional as this is for global fitting
        for(int parI = 0; parI < poly_order_ + 1; parI++) {
            //normalization left floating Batch4
            if(parI = 2) {
                full->SetParameter(parI, bkg->GetParameter(parI));
            }
            //fixing the rest of the shape Batch4
            else {
                full->SetParameter(parI, bkg->GetParameter(parI));
                //full->FixParameter(parI, bkg->GetParameter(parI));//used for fixing bkgshape
            }    
        }

    } else if(isLegendre){
        LegendreFitFunction full_func(mass_hypothesis, window_end_ - window_start_, bin_width_, bkg_order_model, FitFunction::SignalFitModel::GAUSSIAN, isExp);
        full = new TF1("full", full_func, -1, 1, poly_order_ + 4);    
        //signal model info    
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
    } else if(isL3L6){ //global case
        las3pluslas6_FitFunction full_func(mass_hypothesis, window_end_ - window_start_, bin_width_, bkg_order_model, FitFunction::SignalFitModel::GAUSSIAN, isExp);       
        full = new TF1("full", full_func, window_start_, window_end_, poly_order_ + 3);
        //signal model info    
        //full->SetParameter(0, initNorm);
        //full->SetParName(0, "pol0");
        full->SetParameter(poly_order_ , 0.0);
        full->SetParName(poly_order_ , "signal norm");
        full->SetParName(poly_order_ + 1, "mean");
        full->SetParName(poly_order_ + 2, "sigma");
        for(int i = 0; i < poly_order_ ; i++) {
            full->SetParameter(i, 0);
        //    full->SetParName(i, Form("pol%i", i));
        }
        //
        full->FixParameter(poly_order_ + 1, mass_hypothesis);
        full->FixParameter(poly_order_ + 2, mass_resolution_);

        //if(mass_hypothesis < 0.06){
	//	for(int parI = 0; parI < poly_order_ ; parI++) {
	//	    //for normalization factor
	//	    if(parI == 2){
	//		full->SetParameter(parI, bkg->GetParameter(parI));
	//	    }
	//	    //fixing falling function components 
	//	    
	//	    //else if(parI == 5 || parI == 8){ //|| parI == 0 || parI == 3){ // parI == 0 || parI == 5 || parI == 6){
	//	    else if(parI == 1 ||parI == 4 || parI == 6){
	//		full->FixParameter(parI, bkg->GetParameter(parI));
	//	    }
	//	    else{ 
	//		full->SetParameter(parI, bkg->GetParameter(parI));
	//		//full->FixParameter(parI, bkg->GetParameter(parI));//use for fixing bkgshape
	//	    }
	//	    //could insert following comment into the if else above, but removed for now
	//	    //std::cout << "Parameter :" << parI << ": " <<bkg->GetParameter(parI) << std::endl;
	//	}
	//	}    
        //else {
	//	for(int parI = 0; parI < poly_order_ ; parI++) {
	//	    //for normalization factor
	//	    if(parI == 2){
	//		full->SetParameter(parI, bkg->GetParameter(parI));
	//	    }
	//	    //fixing error function rise components 

	//	    //else if(parI == 0 || parI == 3){ //|| parI == 5 || parI == 8){ // parI == 0 || parI == 5 || parI == 6){
	//	    else if(parI == 1 || parI == 4 || parI == 6){
	//		full->FixParameter(parI, bkg->GetParameter(parI));
	//	    }
	//	    else{ 
	//		full->SetParameter(parI, bkg->GetParameter(parI));
	//		//full->FixParameter(parI, bkg->GetParameter(parI));//use for fixing bkgshape
	//	    }
	//	    //could insert following comment into the if else above, but removed for now
	//	    //std::cout << "Parameter :" << parI << ": " <<bkg->GetParameter(parI) << std::endl;
	//	}
	//	}	 


        for(int parI = 0; parI < poly_order_ ; parI++) {
            //for normalization factor
            if(parI == 2){
                full->SetParameter(parI, bkg->GetParameter(parI));
            //}
	    //if(parI == 0 || parI == 1 || parI == 5 || parI == 6){
            //    full->FixParameter(parI, bkg->GetParameter(parI));
            }
            else{ 
                //full->SetParameter(parI, bkg->GetParameter(parI));
                full->FixParameter(parI, bkg->GetParameter(parI));//use for fixing bkgshape
            } 
        }
        

    	} 
	
	else if(isUA23L1) {
        ua23nolinpluslas1_FitFunction full_func(mass_hypothesis, window_end_ - window_start_, bin_width_, bkg_order_model, FitFunction::SignalFitModel::GAUSSIAN, isExp);       
        full = new TF1("full", full_func, window_start_, window_end_, poly_order_ + 3);
        //signal model info    
        //full->SetParameter(0, initNorm);
        //full->SetParName(0, "pol0");
        full->SetParameter(poly_order_ , 0.0);
        full->SetParName(poly_order_ , "signal norm");
        full->SetParName(poly_order_ + 1, "mean");
        full->SetParName(poly_order_ + 2, "sigma");
        for(int i = 0; i < poly_order_ ; i++) {
            full->SetParameter(i, 0);
        //    full->SetParName(i, Form("pol%i", i));
        }
        //
        full->FixParameter(poly_order_ + 1, mass_hypothesis);
        full->FixParameter(poly_order_ + 2, mass_resolution_);

        for(int parI = 0; parI < poly_order_ ; parI++) {
            //for normalization factor
            if(parI == 2){
                full->SetParameter(parI, bkg->GetParameter(parI));
            }
            else{
		full->SetParameter(parI, bkg->GetParameter(parI));    
                //full->FixParameter(parI, bkg->GetParameter(parI)); //use for fixing bkgshape
            }
            //could insert following comment into the if else above, but removed for now
            //std::cout << "Parameter :" << parI << ": " <<bkg->GetParameter(parI) << std::endl;
        }
    }    
    
    //in the following, remove Q when trouble shooting errors in fittin	
    TFitResultPtr full_result = histogram->Fit("full", "LES+", "", window_start_, window_end_);
    //TFitResultPtr full_result = histogram->Fit("full", "QLES+", "", window_start_, window_end_);
    fit_result->setCompFitResult(full_result);

    calculatePValue(fit_result);
    std::cout << "[ BumpHunter ]: Bkg Fit Status: " << fit_result->getBkgFitResult()->IsValid() <<  std::endl;
    std::cout << "[ BumpHunter ]: Bkg Toys Fit Status: " << fit_result->getBkgToysFitResult()->IsValid() <<  std::endl;
    std::cout << "[ BumpHunter ]: Full Fit Status: " << fit_result->getCompFitResult()->IsValid() <<  std::endl;
    //TODO: add a flag such that if full_result is not valid and error matrix is not pos-def it lets us know and fits the point anyway
    //if trouble shooting N signal upper limits comment following line and uncomment get upperlimit 
    //if((!skip_ul) && full_result->IsValid()) { getUpperLimit(histogram, fit_result); }
    getUpperLimit(histogram, fit_result); 

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
    
    bool isChebyshev = (bkg_model_ == FitFunction::BkgModel::CHEBYSHEV || bkg_model_ == FitFunction::BkgModel::EXP_CHEBYSHEV);
    bool isExp = (bkg_model_ == FitFunction::BkgModel::EXP_CHEBYSHEV || bkg_model_ == FitFunction::BkgModel::EXP_LEGENDRE);
    bool isLegendre = (bkg_model_ == FitFunction::BkgModel::LEGENDRE|| bkg_model_ == FitFunction::BkgModel::EXP_LEGENDRE); 
    
    
    bool isL3L6 = (bkg_model_ == FitFunction::BkgModel::LAS3PLUSLAS6);
    bool isUA23L1 = (bkg_model_ == FitFunction::BkgModel::UA23NOLINPLUSLAS1);    
    
    std::cout << "[ BumpHunter ]: Calculating p-value: " << std::endl;
    double signal_yield;// = -1.0;
    if(isChebyshev || isLegendre){
        signal_yield = result->getCompFitResult()->Parameter(poly_order_ + 1);
        //return; 
    } 
    else {
        signal_yield = result->getCompFitResult()->Parameter(poly_order_ ); 
        //std::cout << "dah poly order: " << poly_order_ <<std::
        //std::cout << "Spot Sig Yield: " <<  signal_yield  << std::endl;
        //return;

        //if(signal_yield <= 0) {
        //    result->setPValue(1);
        //    printDebug("Signal yield is negative ... setting p-value = 1");
        //    return;
        //}
    }

    //std::cout << "Sig Yield: " <<  signal_yield  << std::endl;


    printDebug("Signal yield: " + std::to_string(signal_yield));

    // In searching for a resonance, a signal is expected to lead to an 
    // excess of events.  In this case, a signal yield of < 0 is  
    // meaningless so we set the p-value = 1.  This follows the formulation
    // put forth by Cowen et al. in https://arxiv.org/pdf/1007.1727.pdf. 
    //if(signal_yield <= 0) {
    //    result->setPValue(1);
    //    printDebug("Signal yield is negative ... setting p-value = 1");
        //return;
    //}

    // Get the NLL obtained by minimizing the composite model with the signal
    // yield floating.
    double mle_nll = result->getCompFitResult()->MinFcnValue();
    printDebug("NLL when mu = " + std::to_string(signal_yield) + ": " + std::to_string(mle_nll));

    // Get the NLL obtained from the Bkg only fit.
    double bkg_nll = result->getBkgFitResult()->MinFcnValue();
    printDebug("NLL when mu = 0: " + std::to_string(bkg_nll));

    // 1) Calculate the likelihood ratio which is chi2 distributed.
    // 2) From the chi2, calculate the p-value.
    double q0 = 0;
    double p_value = 0;
    getChi2Prob(bkg_nll, mle_nll, q0, p_value);

    // Calculate r0 stat
    double r0 = -2.0*(mle_nll - bkg_nll);
    if(signal_yield < 0.0 ) {r0 = 2.0*(mle_nll - bkg_nll);}
    printDebug("r0: " + std::to_string(r0));
    printDebug("Z0: " + std::to_string(r0*TMath::Sqrt(fabs(r0))/fabs(r0)));
    p_value = 1 - ROOT::Math::gaussian_cdf( r0*TMath::Sqrt(fabs(r0))/fabs(r0) );

    std::cout << "[ BumpHunter ]: p-value: " << p_value << std::endl;

    // Update the result
    result->setPValue(p_value);
    result->setQ0(q0);
    
    //double checking yield isnt <0 and changing pvalue to 1 if so.
    if(signal_yield <= 0) {
        result->setPValue(1);
        printDebug("Signal yield is negative ... setting p-value = 1");
        //return;
    }
    
}

void BumpHunter::printDebug(std::string message) {
    std::cout << "[ BumpHunter ]: " << message << std::endl;
}

void BumpHunter::getUpperLimit(TH1* histogram, HpsFitResult* result) {
    if(asymptotic_limit_) {
        BumpHunter::getUpperLimitAsymCLs(histogram, result);
    } else {
        BumpHunter::getUpperLimitPower(histogram, result);
    }
}

void BumpHunter::getUpperLimitAsymptotic(TH1* histogram, HpsFitResult* result) {
    std::cout << "[ BumpHunter ]: Calculating upper limit." << std::endl;
    double signal_yield = -1.0;
    double signal_yield_error = -1.0;
    // Get the signal yield and signal yield error.
    if(bkg_model_ < 4){
        signal_yield = result->getCompFitResult()->Parameter(poly_order_ + 1);
        if(signal_yield < 0) { signal_yield = 0; }
        signal_yield_error = result->getCompFitResult()->ParError(poly_order_ + 1);
       //return;
    } else { 
        signal_yield = result->getCompFitResult()->Parameter(poly_order_ );
        if(signal_yield < 0) { signal_yield = 0; }
        signal_yield_error = result->getCompFitResult()->ParError(poly_order_ );
        std::cout << "Sig Yield Maybe       :: " << signal_yield << std::endl;
        //return;
    }
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

    //return;
}

void BumpHunter::getUpperLimitAsymCLs(TH1* histogram, HpsFitResult* result) {
    // Determine whether to use an exponential polynomial or normal polynomial.
    bool isChebyshev = (bkg_model_ == FitFunction::BkgModel::CHEBYSHEV || bkg_model_ == FitFunction::BkgModel::EXP_CHEBYSHEV);
    bool isExp = (bkg_model_ == FitFunction::BkgModel::EXP_CHEBYSHEV || bkg_model_ == FitFunction::BkgModel::EXP_LEGENDRE);

    bool isLegendre = (bkg_model_ == FitFunction::BkgModel::LEGENDRE|| bkg_model_ == FitFunction::BkgModel::EXP_LEGENDRE); 

    bool isL3L6 = (bkg_model_ == FitFunction::BkgModel::LAS3PLUSLAS6);
    bool isUA23L1 = (bkg_model_ == FitFunction::BkgModel::UA23NOLINPLUSLAS1);    
    
    double initNorm = log10(integral_);

    // Instantiate a fit function for the appropriate polynomial order.
    TF1* comp{nullptr};
    FitFunction::ModelOrder bkg_order_model;
    if(poly_order_ == 1) { bkg_order_model = FitFunction::ModelOrder::FIRST; }
    else if(poly_order_ == 3) { bkg_order_model = FitFunction::ModelOrder::THIRD; }
    else if(poly_order_ == 5) { bkg_order_model = FitFunction::ModelOrder::FIFTH; }

    else if(isL3L6) { bkg_order_model = FitFunction::ModelOrder::GLOBAL_L3L6; }
    else if(isUA23L1) { bkg_order_model = FitFunction::ModelOrder::GLOBAL_UA23L1; }
    
    if(isChebyshev) {
        ChebyshevFitFunction comp_func(mass_hypothesis_, window_end_ - window_start_, bin_width_, bkg_order_model, FitFunction::SignalFitModel::GAUSSIAN, isExp);
        comp = new TF1("comp_ul", comp_func, -1, 1, poly_order_ + 4);
    } else if(isLegendre) {
        LegendreFitFunction comp_func(mass_hypothesis_, window_end_ - window_start_, bin_width_, bkg_order_model, FitFunction::SignalFitModel::GAUSSIAN, isExp);
        comp = new TF1("comp_ul", comp_func, -1, 1, poly_order_ + 4);
    } else if(isL3L6) {
        las3pluslas6_FitFunction comp_func(mass_hypothesis_, window_end_ - window_start_, bin_width_, bkg_order_model, FitFunction::SignalFitModel::GAUSSIAN, isExp);       
        comp = new TF1("comp_ul", comp_func, window_start_, window_end_, poly_order_ + 3);
    } else if(isUA23L1) {
        ua23nolinpluslas1_FitFunction comp_func(mass_hypothesis_, window_end_ - window_start_, bin_width_, bkg_order_model, FitFunction::SignalFitModel::GAUSSIAN, isExp);       
        comp = new TF1("comp_ul", comp_func, window_start_, window_end_, poly_order_ + 3);
    }




    if(isChebyshev || isLegendre){

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
        //for(int parI = 1; parI < poly_order_ + 1; parI++) {
        //    comp->FixParameter(parI, result->getCompFitResult()->Parameter(parI));
        //}
    } else {

        //comp->SetParameter(0, initNorm);
        //comp->SetParName(0, "pol0");
        comp->SetParameter(poly_order_ , 0.0);
        comp->SetParName(poly_order_ , "signal norm");
        comp->SetParName(poly_order_ + 1, "mean");
        comp->SetParName(poly_order_ + 2, "sigma");
        for(int i = 0; i < poly_order_ ; i++) {
           comp->SetParameter(i, 0);
        //    comp->SetParName(i, Form("pol%i", i));
        }
        comp->FixParameter(poly_order_ + 1, mass_hypothesis_);
        comp->FixParameter(poly_order_ + 2, mass_resolution_);


//        if(mass_hypothesis_ < 0.06){
//		for(int parI = 0; parI < poly_order_ ; parI++) {
//		    //for normalization factor
//		    if(parI == 2){
//			comp->SetParameter(parI, result->getCompFitResult()->Parameter(parI));
//		    }
//		    //fixing falling function components 
//		    //else if(parI == 5 || parI == 8){ //|| parI == 0 || parI == 3){ // parI == 0 || parI == 5 || parI == 6){
//		
//		    else if(parI == 1 || parI == 4 || parI == 6){
//	    	        comp->FixParameter(parI, result->getCompFitResult()->Parameter(parI)); 
//		    }
//		    else{ 
//			comp->SetParameter(parI, result->getCompFitResult()->Parameter(parI));
//			//full->FixParameter(parI, bkg->GetParameter(parI));//use for fixing bkgshape
//		    }
//		    //could insert following comment into the if else above, but removed for now
//		    //std::cout << "Parameter :" << parI << ": " <<bkg->GetParameter(parI) << std::endl;
//		}
//		}    
//        else {
//		for(int parI = 0; parI < poly_order_ ; parI++) {
//		    //for normalization factor
//		    if(parI == 2){
//			comp->SetParameter(parI, result->getCompFitResult()->Parameter(parI));// bkg->GetParameter(parI));
//		    }
//		    //fixing error function rise components 
//		    //else if(parI == 0 || parI == 3){ //|| parI == 5 || parI == 8){ // parI == 0 || parI == 5 || parI == 6){
//		
//		    else if(parI == 1 || parI == 4 || parI == 6){
//	       	        comp->FixParameter(parI, result->getCompFitResult()->Parameter(parI));//bkg->GetParameter(parI));
//		    }
//		    else{ 
//			comp->SetParameter(parI, result->getCompFitResult()->Parameter(parI));//bkg->GetParameter(parI));
//			//full->FixParameter(parI, bkg->GetParameter(parI));//use for fixing bkgshape
//		    }
//		    //could insert following comment into the if else above, but removed for now
//		    //std::cout << "Parameter :" << parI << ": " <<bkg->GetParameter(parI) << std::endl;
//		}
//		}	    
//



        for(int parI = 0; parI < poly_order_ ; parI++) {
            //for normalization parameter 
            if (parI == 2|| parI == 7){
                comp->SetParameter(parI, result->getCompFitResult()->Parameter(parI));
            //}
	    //fixing error function components
            //if(parI == 0 || parI == 1 || parI == 5 || parI == 6) {
            //	comp->FixParameter(parI, result->getCompFitResult()->Parameter(parI));  
            }
            else{ 
		//comp->SetParameter(parI, result->getCompFitResult()->Parameter(parI));
    		comp->FixParameter(parI, result->getCompFitResult()->Parameter(parI));//used for fixing bkgshape
            }
        }
    }   

    std::cout << "Mass resolution: " << mass_resolution_ << std::endl;
    std::cout << "[ BumpHunter ]: Calculating upper limit." << std::endl;
    double mu_hat = -1.0;
    double sigma = -1.0;
    if(isChebyshev || isLegendre){
        //  Get the signal yield obtained from the signal+bkg fit
        mu_hat = result->getCompFitResult()->Parameter(poly_order_ + 1);
        sigma = result->getCompFitResult()->ParError(poly_order_ + 1);
        //break;
    } else {
        mu_hat = result->getCompFitResult()->Parameter(poly_order_ );
        sigma = result->getCompFitResult()->ParError(poly_order_ );
        //break;
    }

    printDebug("Signal yield: " + std::to_string(mu_hat));
    printDebug("Sigma: " + std::to_string(sigma));

    // Get the minimum NLL value that will be used for testing upper limits.
    double mle_nll = result->getCompFitResult()->MinFcnValue();

    // Get the NLL obtained assuming the background only hypothesis
    double bkg_nll = result->getBkgFitResult()->MinFcnValue();

    printDebug("MLE NLL: " + std::to_string(mle_nll));
    printDebug("mu=0 NLL: " + std::to_string(bkg_nll));

    //double mu95up = fabs(mu_hat + 1.64*sigma); //This should give us something close to start
    double mu95up = 4.0 *TMath::Sqrt(fabs(mu_hat + 1.64*sigma));//using the 4 to inch it up a little bit firstattempting sqrt first
    //double mu95up = 0.5*fabs(mu_hat + 1.64*sigma);


    double CLs = 1.0;
    double p_mu = 1;
    double p_b = 1;
    double r_mu = -9999999.9;
    int tryN = 0;
    while(true) {
        tryN++;
        if(isChebyshev || isLegendre){
            comp->FixParameter(poly_order_ + 1, mu95up);
            //continue;
        } else {
            comp->FixParameter(poly_order_ , mu95up); //maybe set on dec11 
            //continue;
        }
        TFitResultPtr full_result = histogram->Fit("comp_ul", "NQLES", "", window_start_, window_end_);
        double mu_nll = full_result->MinFcnValue();

        double nllamb_mu = mu_nll - mle_nll;
        if(mu_hat < 0.0) 
        {    
            nllamb_mu = mu_nll - bkg_nll;
        }
        
        r_mu = 2.0*nllamb_mu;
        if(mu_hat > mu95up) 
        {
            r_mu = -2.0*nllamb_mu;
        }

        if(r_mu < 0.0)
        {
            p_mu = 1.0 - ROOT::Math::gaussian_cdf( -1.0*TMath::Sqrt(-1.0*r_mu) );
            p_b =  1.0 - ROOT::Math::gaussian_cdf( -1.0*TMath::Sqrt(-1.0*r_mu) - (mu95up/sigma) );
        }
        else if(r_mu > mu95up*mu95up/(sigma*sigma))
        {
            p_mu =  1.0 - ROOT::Math::gaussian_cdf( (r_mu + mu95up*mu95up/(sigma*sigma))/(2.0*mu95up/sigma) );
            p_b =   1.0 - ROOT::Math::gaussian_cdf( (r_mu - mu95up*mu95up/(sigma*sigma))/(2.0*mu95up/sigma) );
        }
        else
        {
            p_mu = 1.0 - ROOT::Math::gaussian_cdf( TMath::Sqrt(r_mu) );
            p_b =  1.0 - ROOT::Math::gaussian_cdf( TMath::Sqrt(r_mu) - (mu95up/sigma) );
        }


        CLs = p_mu/p_b;

        std::cout << "[ BumpHunter ]: mu: " << mu95up << std::endl;
        std::cout << "[ BumpHunter ]: mle_nll: " << mle_nll << std::endl;
        std::cout << "[ BumpHunter ]: bkg_nll: " << bkg_nll << std::endl;
        std::cout << "[ BumpHunter ]: mu_nll: " << mu_nll << std::endl;
        std::cout << "[ BumpHunter ]: nllamb_mu: " << nllamb_mu << std::endl;
        std::cout << "[ BumpHunter ]: r_mu: " << r_mu << std::endl;
        std::cout << "[ BumpHunter ]: p_mu: " << p_mu << std::endl;
        std::cout << "[ BumpHunter ]: p_b: " << p_b << std::endl;
        std::cout << "[ BumpHunter ]: CLs: " << CLs << std::endl;
        if(tryN > 1000) 
        {
            std::cout << "[ BumpHunter ]: Upper limit aborting, too many tries" << std::endl;

            result->setUpperLimit(1.64*sigma);
            result->setUpperLimitPValue(-9.0);

            break;
        } 
        if((CLs <= 0.051 && CLs > 0.049)) 
        {
            std::cout << "[ BumpHunter ]: Upper limit: " << mu95up << std::endl;
            std::cout << "[ BumpHunter ]: CLs: " << CLs << std::endl;

            result->setUpperLimit(mu95up);
            result->setUpperLimitPValue(CLs);

            break;
        } 
        else if(CLs <= 1e-10) { mu95up = mu95up*0.1; }
        else if(CLs <= 1e-8) { mu95up = mu95up*0.5; }
        else if(CLs <= 1e-4) { mu95up = mu95up*0.8; }
        else if(CLs <= 0.01) { mu95up = mu95up*0.9; }
        else if(CLs <= 0.04) { mu95up = mu95up*0.99; }
        else if(CLs <= 0.049) { mu95up = mu95up*0.999; }
        else if(CLs <= 0.1) { mu95up = mu95up*1.01; }
        else { mu95up = mu95up*1.1; }
        printDebug("Setting mu to: " + std::to_string(mu95up));
    }
}

void BumpHunter::getUpperLimitPower(TH1* histogram, HpsFitResult* result) {
    // Determine whether to use an exponential polynomial or normal polynomial.
    bool isChebyshev = (bkg_model_ == FitFunction::BkgModel::CHEBYSHEV || bkg_model_ == FitFunction::BkgModel::EXP_CHEBYSHEV);
    bool isExp = (bkg_model_ == FitFunction::BkgModel::EXP_CHEBYSHEV || bkg_model_ == FitFunction::BkgModel::EXP_LEGENDRE);
    bool isLegendre = (bkg_model_ == FitFunction::BkgModel::LEGENDRE|| bkg_model_ == FitFunction::BkgModel::EXP_LEGENDRE);
    double initNorm = log10(integral_);

    bool isL3L6 = (bkg_model_ == FitFunction::BkgModel::LAS3PLUSLAS6);
    bool isUA23L1 = (bkg_model_ == FitFunction::BkgModel::UA23NOLINPLUSLAS1);    

    // Instantiate a fit function for the appropriate polynomial order.
    TF1* comp{nullptr};
    FitFunction::ModelOrder bkg_order_model;
    if(poly_order_ == 1) { bkg_order_model = FitFunction::ModelOrder::FIRST; }
    else if(poly_order_ == 3) { bkg_order_model = FitFunction::ModelOrder::THIRD; }
    else if(poly_order_ == 5) { bkg_order_model = FitFunction::ModelOrder::FIFTH; }
    //global bkg models
    else if(isL3L6) { bkg_order_model = FitFunction::ModelOrder::GLOBAL_L3L6; }
    else if(isUA23L1) { bkg_order_model = FitFunction::ModelOrder::GLOBAL_UA23L1; }

    //defining the composite function here
    if(isChebyshev) {
        ChebyshevFitFunction comp_func(mass_hypothesis_, window_end_ - window_start_, bin_width_, bkg_order_model, FitFunction::SignalFitModel::GAUSSIAN, isExp);
        comp = new TF1("comp_ul", comp_func, -1, 1, poly_order_ + 4);
    } else if(isLegendre){
        LegendreFitFunction comp_func(mass_hypothesis_, window_end_ - window_start_, bin_width_, bkg_order_model, FitFunction::SignalFitModel::GAUSSIAN, isExp);
        comp = new TF1("comp_ul", comp_func, -1, 1, poly_order_ + 4);
    } else if(isL3L6) {
        las3pluslas6_FitFunction comp_func(mass_hypothesis_, window_end_ - window_start_, bin_width_, bkg_order_model, FitFunction::SignalFitModel::GAUSSIAN, isExp);       
        comp = new TF1("comp_ul", comp_func, window_start_, window_end_, poly_order_ + 3);
    } else if(isUA23L1) {
        ua23nolinpluslas1_FitFunction comp_func(mass_hypothesis_, window_end_ - window_start_, bin_width_, bkg_order_model, FitFunction::SignalFitModel::GAUSSIAN, isExp);       
        comp = new TF1("comp_ul", comp_func, window_start_, window_end_, poly_order_ + 3);
    }


    double signal_yield = -1.0;
    if (isChebyshev || isLegendre){
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
        signal_yield = result->getCompFitResult()->Parameter(poly_order_ + 1);
        printDebug("Signal yield: " + std::to_string(signal_yield));
        //return;
    } else {
        //comp->SetParameter(0, initNorm);
        //comp->SetParName(0, "pol0");
        comp->SetParameter(poly_order_ , 0.0);
        comp->SetParName(poly_order_ , "signal norm");
        comp->SetParName(poly_order_ + 1, "mean");
        comp->SetParName(poly_order_ + 2, "sigma");
        for(int i = 0; i < poly_order_ ; i++) {
            comp->SetParameter(i, 0);
        //    comp->SetParName(i, Form("pol%i", i));
        }
        comp->FixParameter(poly_order_ + 1, mass_hypothesis_);
        comp->FixParameter(poly_order_ + 2, mass_resolution_);

        std::cout << "Mass resolution: " << mass_resolution_ << std::endl;
        std::cout << "[ BumpHunter ]: Calculating upper limit." << std::endl;

        //  Get the signal yield obtained from the signal+bkg fit
        signal_yield = result->getCompFitResult()->Parameter(poly_order_ );
        printDebug("Signal yield: " + std::to_string(signal_yield));
        //return;
    }

    // Get the minimum NLL value that will be used for testing upper limits.
    // If the signal yield (mu estimator) at the min NLL is < 0, use the NLL
    // obtained when mu = 0.
    double mle_nll = result->getCompFitResult()->MinFcnValue();

    if(signal_yield < 0) {
        printDebug("Signal yield @ min NLL is < 0. Using NLL when signal yield = 0");

        // Get the NLL obtained assuming the background only hypothesis
        mle_nll = result->getBkgFitResult()->MinFcnValue();

        signal_yield = 0;
        //return;
    }
    printDebug("MLE NLL: " + std::to_string(mle_nll));

    signal_yield = floor(signal_yield) + 1;

    double p_value = 1;
    double q0 = 0;
    while(true) {
        printDebug("Setting signal yield to: " + std::to_string(signal_yield));
        //std::cout << "[ BumpHunter ]: Current p-value: " << p_value << std::endl;

        if (isChebyshev || isLegendre){
            comp->FixParameter(poly_order_ + 1, signal_yield);
            //continue;
        } else 
        {
            //setting the parameter here other attempts had it fixed
            comp->SetParameter(poly_order_ , signal_yield); //redo with it set after december 11
            //continue;
        }

        TFitResultPtr full_result = histogram->Fit("comp_ul", "QLES+", "", window_start_, window_end_);
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
    //printDebug("Cond NLL: " + std::to_string(cond_nll));
    //printDebug("Uncod NLL: " + std::to_string(mle_nll));
    double diff = cond_nll - mle_nll;
    //printDebug("Delta NLL: " + std::to_string(diff));

    q0 = 2 * diff;
    //printDebug("q0: " + std::to_string(q0));

    p_value = ROOT::Math::chisquared_cdf_c(q0, 1);
    //printDebug("p-value before dividing: " + std::to_string(p_value));
    p_value *= 0.5;
    //printDebug("p-value: " + std::to_string(p_value));
    //return; 
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
