#include <math.h>
#include "BhMassResSystematicsProcessor.h"

BhMassResSystematicsProcessor::BhMassResSystematicsProcessor(const std::string& name, Process& process)
    : Processor(name, process) { 
    }

BhMassResSystematicsProcessor::~BhMassResSystematicsProcessor() { }

void BhMassResSystematicsProcessor::configure(const ParameterSet& parameters) {
    std::cout << "Configuring BhMassResSystematicsProcessor" << std::endl;
    std::string temp_file_name = "";
    std::string temp_func_name = "";
    try {
        debug_               = parameters.getInteger("debug");
        massSpectrum_        = parameters.getString("massSpectrum");
        mass_hypo_           = parameters.getDouble("mass_hypo");
        win_factor_          = parameters.getInteger("win_factor");
        poly_order_          = parameters.getInteger("poly_order");
        seed_                = parameters.getInteger("seed");
        bkg_model_           = parameters.getInteger("bkg_model");
        res_runs_            = parameters.getInteger("num_iters");
        res_width_           = parameters.getDouble("res_sigma");
        temp_file_name       = parameters.getString("function_file");
        function_name_       = parameters.getString("function_name");
		toy_res_runs_        = parameters.getInteger("toy_num_iters");
		nToys_               = parameters.getInteger("num_toys");
    } catch(std::runtime_error& error) {
        std::cout << error.what() << std::endl;
    }
    
    std::cout << "[ DEBUG ] :: Creating ROOT file \"" << temp_file_name << "\"." << std::endl;
    if(temp_file_name != "") { function_file_ = new TFile(temp_file_name.c_str()); }
}

void BhMassResSystematicsProcessor::initialize(std::string inFilename, std::string outFilename) {
    // Initiate the input file.
    inF_ = new TFile(inFilename.c_str());
    
    // Get the mass spectrum from file.
    mass_spec_h = (TH1*) inF_->Get(massSpectrum_.c_str());
    
    // Get the appropriate background model.
    FitFunction::BkgModel bkg_fit_model;
    std::cout << "Background Model ID: " << bkg_model_ << std::endl;
    switch(bkg_model_) {
        case 0: bkg_fit_model = FitFunction::BkgModel::EXP_CHEBYSHEV;
                break;
        case 1: bkg_fit_model = FitFunction::BkgModel::EXP_CHEBYSHEV;
                break;
        case 2: bkg_fit_model = FitFunction::BkgModel::LEGENDRE;
                break;
        case 3: bkg_fit_model = FitFunction::BkgModel::EXP_LEGENDRE;
                break;
        default: bkg_fit_model = FitFunction::BkgModel::EXP_LEGENDRE;
    }
    
    // Initiate the bump hunter.
    bump_hunter_ = new BumpHunter(bkg_fit_model, poly_order_, poly_order_ + 2, win_factor_, 1.00, asymptotic_limit_);
    bump_hunter_->setWindowSizeUsesResScale(false);
    bump_hunter_->setBounds(mass_spec_h->GetXaxis()->GetBinUpEdge(mass_spec_h->FindFirstBinAbove()),
            mass_spec_h->GetXaxis()->GetBinLowEdge(mass_spec_h->FindLastBinAbove()));
    
    // Get the mass resolution parameterization, if it is defined.
    std::cout << "[ DEBUG ] :: Attempting to acquire parameterization function." << std::endl;
    if(function_file_ != nullptr && function_name_ != "") {
        std::cout << "[ DEBUG ] :: Function file and name are defined!" << std::endl;
        std::cout << "[ DEBUG ] :: Function name is \"" << function_name_ << "\"." << std::endl;
        TF1* massResSysFunc = (TF1*) function_file_->Get(function_name_.c_str());
        std::cout << "[ DEBUG ] :: Acquired function object." << std::endl;
        res_width_ = massResSysFunc->Eval(mass_hypo_) / 100.0;
        std::cout << "[ DEBUG ] :: Resolution width for " << mass_hypo_ << " MeV is " << res_width_ << "." << std::endl;
        std::cout << "Using mass resolution error parameterization function " << function_name_ << " from file." << std::endl;
    } else {
        std::cout << "Using constant mass resolution error " << res_width_ << "." << std::endl;
    }

    // Enabled debug output if relevant.
    if(debug_ > 0) { bump_hunter_->enableDebug(); }
    
    // Initiate the tuple maker.
    flat_tuple_ = new FlatTupleMaker(outFilename.c_str(), "fit_toys");
    
    // Setup tuple branches
    flat_tuple_->addVariable("seed");
    flat_tuple_->addVariable("poly_order");
    flat_tuple_->addVariable("win_factor");
    flat_tuple_->addVariable("bkg_model");
    flat_tuple_->addVariable("iterations");
    flat_tuple_->addVariable("res_width");
    flat_tuple_->addVariable("num_toys");
    flat_tuple_->addVariable("toy_iterations");
    
    // Create the nominal value variables.
    flat_tuple_->addVariable("nominal_bkg_total");
    flat_tuple_->addVariable("nominal_corr_mass");
    flat_tuple_->addVariable("nominal_mass_hypo");
    flat_tuple_->addVariable("nominal_window_size");
    flat_tuple_->addVariable("nominal_resolution_scale");
    flat_tuple_->addVariable("nominal_mass_resolution");
    flat_tuple_->addVariable("nominal_bkg_chi2_prob");
    flat_tuple_->addVariable("nominal_bkgsig_chi2_prob");
    flat_tuple_->addVariable("nominal_bkg_edm");
    flat_tuple_->addVariable("nominal_bkg_minuit_status");
    flat_tuple_->addVariable("nominal_bkg_nll");
    
    flat_tuple_->addVariable("nominal_edm");
    flat_tuple_->addVariable("nominal_minuit_status");
    flat_tuple_->addVariable("nominal_nll");
    flat_tuple_->addVariable("nominal_p_value");
    flat_tuple_->addVariable("nominal_q0");
    flat_tuple_->addVariable("nominal_bkg_rate_mass_hypo");
    flat_tuple_->addVariable("nominal_bkg_rate_mass_hypo_err");
    flat_tuple_->addVariable("nominal_sig_yield");
    flat_tuple_->addVariable("nominal_sig_yield_err");
    flat_tuple_->addVariable("nominal_upper_limit");
    flat_tuple_->addVariable("nominal_ul_p_value");
    
    // Create the iteration variable vectors.
    flat_tuple_->addVector("bkg_total");
    flat_tuple_->addVector("corr_mass");
    flat_tuple_->addVector("mass_hypo");
    flat_tuple_->addVector("window_size");
    flat_tuple_->addVector("resolution_scale");
    flat_tuple_->addVector("mass_resolution");
    flat_tuple_->addVector("bkg_chi2_prob");
    flat_tuple_->addVector("bkgsig_chi2_prob");
    flat_tuple_->addVector("bkg_edm");
    flat_tuple_->addVector("bkg_minuit_status");
    flat_tuple_->addVector("bkg_nll");
    
    flat_tuple_->addVector("edm");
    flat_tuple_->addVector("minuit_status");
    flat_tuple_->addVector("nll");
    flat_tuple_->addVector("p_value");
    flat_tuple_->addVector("q0");
    flat_tuple_->addVector("bkg_rate_mass_hypo");
    flat_tuple_->addVector("bkg_rate_mass_hypo_err");
    flat_tuple_->addVector("sig_yield");
    flat_tuple_->addVector("sig_yield_err");
    flat_tuple_->addVector("upper_limit");
    flat_tuple_->addVector("ul_p_value");
}

bool BhMassResSystematicsProcessor::process() {
    std::cout << "Running on mass spectrum: " << massSpectrum_ << std::endl;
    std::cout << "Running with polynomial order: " << poly_order_ << std::endl;
    std::cout << "Running with window factor: " << win_factor_ << std::endl;
    std::cout << "Running on mass hypothesis: " << mass_hypo_ << std::endl;
    
    // Set the tuple values which are the same for all runs.
    flat_tuple_->setVariableValue("seed",           seed_);
    flat_tuple_->setVariableValue("bkg_model",      bkg_model_);
    flat_tuple_->setVariableValue("poly_order",     poly_order_);
    flat_tuple_->setVariableValue("win_factor",     win_factor_);
    flat_tuple_->setVariableValue("iterations",     res_runs_);
    flat_tuple_->setVariableValue("res_width",      res_width_);
    flat_tuple_->setVariableValue("num_toys",       nToys_);
    flat_tuple_->setVariableValue("toy_iterations", toy_res_runs_);
    
    // Run one fit with the nominal resolution.
    HpsFitResult* nominal_result = bump_hunter_->performSearch(mass_spec_h, mass_hypo_, false, false);
    TFitResultPtr nominal_bkg_result = nominal_result->getBkgFitResult();
    TFitResultPtr nominal_sig_result = nominal_result->getCompFitResult();
    
    // Write the invariant mass histogram to the output file.
    mass_spec_h->Write();
    
    // Set the tuple values.
    flat_tuple_->setVariableValue("nominal_bkg_total",              nominal_result->getIntegral());
    flat_tuple_->setVariableValue("nominal_corr_mass",              nominal_result->getCorrectedMass());
    flat_tuple_->setVariableValue("nominal_mass_hypo",              nominal_result->getMass());
    flat_tuple_->setVariableValue("nominal_window_size",            nominal_result->getWindowSize());
    flat_tuple_->setVariableValue("nominal_resolution_scale",       1.00);
    flat_tuple_->setVariableValue("nominal_mass_resolution",        bump_hunter_->getMassResolution(mass_hypo_));
    
    flat_tuple_->setVariableValue("nominal_bkg_chi2_prob",          nominal_bkg_result->Prob());
    flat_tuple_->setVariableValue("nominal_bkgsig_chi2_prob",       nominal_sig_result->Prob());
    flat_tuple_->setVariableValue("nominal_bkg_edm",                nominal_bkg_result->Edm());
    flat_tuple_->setVariableValue("nominal_bkg_minuit_status",      nominal_bkg_result->Status());
    flat_tuple_->setVariableValue("nominal_bkg_nll",                nominal_bkg_result->MinFcnValue());
    
    flat_tuple_->setVariableValue("nominal_edm",                    nominal_sig_result->Edm());
    flat_tuple_->setVariableValue("nominal_minuit_status",          nominal_sig_result->Status());
    flat_tuple_->setVariableValue("nominal_nll",                    nominal_sig_result->MinFcnValue());
    flat_tuple_->setVariableValue("nominal_p_value",                nominal_result->getPValue());
    flat_tuple_->setVariableValue("nominal_q0",                     nominal_result->getQ0());
    flat_tuple_->setVariableValue("nominal_bkg_rate_mass_hypo",     nominal_result->getFullBkgRate());
    flat_tuple_->setVariableValue("nominal_bkg_rate_mass_hypo_err", nominal_result->getFullBkgRateError());
    flat_tuple_->setVariableValue("nominal_sig_yield",              nominal_result->getSignalYield());
    flat_tuple_->setVariableValue("nominal_sig_yield_err",          nominal_result->getSignalYieldErr());
    flat_tuple_->setVariableValue("nominal_upper_limit",            nominal_result->getUpperLimit());
    flat_tuple_->setVariableValue("nominal_ul_p_value",             nominal_result->getUpperLimitPValue());

    // Make a random number generator.
    TRandom *rng = new TRandom();
    
    // Iterate for a number of loops equal to the desired number of
    // resolution scale samples.
    for(int i = 0; i < res_runs_; i++) {
        // Set the resolution scale to a random value according to a
        // normal distribution around the actual resolution with a
        // width as specified by the user.
        double res_scale = rng->Gaus(1.00, res_width_);
        bump_hunter_->setResolutionScale(res_scale);
        
        // Search for a resonance at the given mass hypothesis.
        HpsFitResult* result = bump_hunter_->performSearch(mass_spec_h, mass_hypo_, false, false);
        
        // Get the result of the background fit.
        TFitResultPtr bkg_result = result->getBkgFitResult();
        
        std::cout << "Filling Fit Results " << std::endl;
        // Get the result of the signal+background fit.
        TFitResultPtr sig_result = result->getCompFitResult();
        
        // Set the fit parameters in the tuple.
        flat_tuple_->addToVector("bkg_total",              result->getIntegral());
        flat_tuple_->addToVector("corr_mass",              result->getCorrectedMass());
        flat_tuple_->addToVector("mass_hypo",              result->getMass());
        flat_tuple_->addToVector("window_size",            result->getWindowSize());
        flat_tuple_->addToVector("resolution_scale",       res_scale);
        flat_tuple_->addToVector("mass_resolution",        bump_hunter_->getMassResolution(mass_hypo_));
        
        // Set the fit results in the tuple.
        flat_tuple_->addToVector("bkg_chi2_prob",          bkg_result->Prob());
        flat_tuple_->addToVector("bkgsig_chi2_prob",       sig_result->Prob());
        flat_tuple_->addToVector("bkg_edm",                bkg_result->Edm());
        flat_tuple_->addToVector("bkg_minuit_status",      bkg_result->Status());
        flat_tuple_->addToVector("bkg_nll",                bkg_result->MinFcnValue());
        
        flat_tuple_->addToVector("edm",                    sig_result->Edm());
        flat_tuple_->addToVector("minuit_status",          sig_result->Status());
        flat_tuple_->addToVector("nll",                    sig_result->MinFcnValue());
        flat_tuple_->addToVector("p_value",                result->getPValue());
        flat_tuple_->addToVector("q0",                     result->getQ0());
        flat_tuple_->addToVector("bkg_rate_mass_hypo",     result->getFullBkgRate());
        flat_tuple_->addToVector("bkg_rate_mass_hypo_err", result->getFullBkgRateError());
        flat_tuple_->addToVector("sig_yield",              result->getSignalYield());
        flat_tuple_->addToVector("sig_yield_err",          result->getSignalYieldErr());
        flat_tuple_->addToVector("upper_limit",            result->getUpperLimit());
        flat_tuple_->addToVector("ul_p_value",             result->getUpperLimitPValue());
    }
    
    // Get the quantile vectors.
    std::vector<double> vec_sigYield = flat_tuple_->getVector("sig_yield");
    std::vector<double> vec_sigYieldErr = flat_tuple_->getVector("sig_yield_err");
    std::vector<double> vec_bkgRate = flat_tuple_->getVector("bkg_rate_mass_hypo");
    std::vector<double> vec_upperLimit = flat_tuple_->getVector("upper_limit");
    std::vector<double> vec_pValue = flat_tuple_->getVector("p_value");
    
    // Sort the vectors.
    std::sort(vec_sigYield.begin(), vec_sigYield.end());
    std::sort(vec_sigYieldErr.begin(), vec_sigYieldErr.end());
    std::sort(vec_bkgRate.begin(), vec_bkgRate.end());
    std::sort(vec_upperLimit.begin(), vec_upperLimit.end());
    std::sort(vec_pValue.begin(), vec_pValue.end());
    
    // Calculate the median and 1 sigma indices.
    int median = (int) rint(vec_sigYield.size() / 2.0);
    int sigma_1_u = (int) rint((vec_sigYield.size() / 2.0) + (0.341 * vec_sigYield.size()));
    int sigma_1_l = (int) rint((vec_sigYield.size() / 2.0) - (0.341 * vec_sigYield.size()));
    
    // Create variables in the tuple to store the quantile values.
    flat_tuple_->addVariable("sig_yield_median");
    flat_tuple_->addVariable("sig_yield_sigma1u");
    flat_tuple_->addVariable("sig_yield_sigma1l");
    
    flat_tuple_->addVariable("sig_yield_err_median");
    flat_tuple_->addVariable("sig_yield_err_sigma1u");
    flat_tuple_->addVariable("sig_yield_err_sigma1l");
    
    flat_tuple_->addVariable("bkg_rate_mass_hypo_median");
    flat_tuple_->addVariable("bkg_rate_mass_hypo_sigma1u");
    flat_tuple_->addVariable("bkg_rate_mass_hypo_sigma1l");
    
    flat_tuple_->addVariable("upper_limit_median");
    flat_tuple_->addVariable("upper_limit_sigma1u");
    flat_tuple_->addVariable("upper_limit_sigma1l");
    
    flat_tuple_->addVariable("p_value_median");
    flat_tuple_->addVariable("p_value_sigma1u");
    flat_tuple_->addVariable("p_value_sigma1l");
    
    // Store the quantile values.
    flat_tuple_->setVariableValue("sig_yield_median", vec_sigYield[median]);
    flat_tuple_->setVariableValue("sig_yield_sigma1u", vec_sigYield[sigma_1_u]);
    flat_tuple_->setVariableValue("sig_yield_sigma1l", vec_sigYield[sigma_1_l]);
    
    flat_tuple_->setVariableValue("sig_yield_err_median", vec_sigYieldErr[median]);
    flat_tuple_->setVariableValue("sig_yield_err_sigma1u", vec_sigYieldErr[sigma_1_u]);
    flat_tuple_->setVariableValue("sig_yield_err_sigma1l", vec_sigYieldErr[sigma_1_l]);
    
    flat_tuple_->setVariableValue("bkg_rate_mass_hypo_median", vec_bkgRate[median]);
    flat_tuple_->setVariableValue("bkg_rate_mass_hypo_sigma1u", vec_bkgRate[sigma_1_u]);
    flat_tuple_->setVariableValue("bkg_rate_mass_hypo_sigma1l", vec_bkgRate[sigma_1_l]);
    
    flat_tuple_->setVariableValue("upper_limit_median", vec_upperLimit[median]);
    flat_tuple_->setVariableValue("upper_limit_sigma1u", vec_upperLimit[sigma_1_u]);
    flat_tuple_->setVariableValue("upper_limit_sigma1l", vec_upperLimit[sigma_1_l]);
    
    flat_tuple_->setVariableValue("p_value_median", vec_pValue[median]);
    flat_tuple_->setVariableValue("p_value_sigma1u", vec_pValue[sigma_1_u]);
    flat_tuple_->setVariableValue("p_value_sigma1l", vec_pValue[sigma_1_l]);
    
    
    // Set the parameters for toy generation. Most of these should be
    // defaults. Systematic studies should not require the more
    // advanced options.
    int seed_ = 0;
    int toy_sig_samples_ = 0;
    double bkg_mult_ = 1.00;
    TH1* signal_shape_h_ = nullptr;
    
    // Generate the toys and perform the toy fits.
    if(nToys_ > 0) {
        // Generate toy models.
        std::cout << "Generating " << nToys_ << " Toys" << std::endl;
        std::vector<TH1*> toys_hist = bump_hunter_->generateToys(mass_spec_h, nToys_, seed_, toy_sig_samples_, bkg_mult_, signal_shape_h_);
        
        // Perform the fits for each toy model.
        int toyFitN = 0;
        for(TH1* hist : toys_hist) {
			// Generate the vector entries for the toy results. A
			// different set of tuples must be generated for each
			// toy distribution.
			std::string prefix = std::to_string(toyFitN).append("_");
			flat_tuple_->addVector(prefix + "toy_bkg_total");
			flat_tuple_->addVector(prefix + "toy_corr_mass");
			flat_tuple_->addVector(prefix + "toy_mass_hypo");
			flat_tuple_->addVector(prefix + "toy_window_size");
			flat_tuple_->addVector(prefix + "toy_resolution_scale");
			flat_tuple_->addVector(prefix + "toy_mass_resolution");
			flat_tuple_->addVector(prefix + "toy_bkg_chi2_prob");
			flat_tuple_->addVector(prefix + "toy_bkgsig_chi2_prob");
			flat_tuple_->addVector(prefix + "toy_bkg_edm");
			flat_tuple_->addVector(prefix + "toy_bkg_minuit_status");
			flat_tuple_->addVector(prefix + "toy_bkg_nll");
			
			flat_tuple_->addVector(prefix + "toy_edm");
			flat_tuple_->addVector(prefix + "toy_minuit_status");
			flat_tuple_->addVector(prefix + "toy_nll");
			flat_tuple_->addVector(prefix + "toy_p_value");
			flat_tuple_->addVector(prefix + "toy_q0");
			flat_tuple_->addVector(prefix + "toy_bkg_rate_mass_hypo");
			flat_tuple_->addVector(prefix + "toy_bkg_rate_mass_hypo_err");
			flat_tuple_->addVector(prefix + "toy_sig_yield");
			flat_tuple_->addVector(prefix + "toy_sig_yield_err");
			flat_tuple_->addVector(prefix + "toy_upper_limit");
			flat_tuple_->addVector(prefix + "toy_ul_p_value");
			
            // Store the results of the mass resolution variance for
            // this toy.
            std::cout << "Fitting Toy " << toyFitN << std::endl;
            
            // Fit the toy multiple times with different mass
            // resolution values.
            for(int i = 0; i < toy_res_runs_; i++) {
                // Set the resolution scale to a random value according to a
                // normal distribution around the actual resolution with a
                // width as specified by the user.
                double res_scale = rng->Gaus(1.00, res_width_);
                bump_hunter_->setResolutionScale(res_scale);
                
                // Perform the resonance search.
                HpsFitResult* result = bump_hunter_->performSearch(hist, mass_hypo_, false, false);
        
				// Get the result of the background fit.
				TFitResultPtr bkg_result = result->getBkgFitResult();
				
				// Get the result of the signal+background fit.
				TFitResultPtr sig_result = result->getCompFitResult();
        
				// Set the fit parameters in the tuple.
				flat_tuple_->addToVector(prefix + "toy_bkg_total",              result->getIntegral());
				flat_tuple_->addToVector(prefix + "toy_corr_mass",              result->getCorrectedMass());
				flat_tuple_->addToVector(prefix + "toy_mass_hypo",              result->getMass());
				flat_tuple_->addToVector(prefix + "toy_window_size",            result->getWindowSize());
				flat_tuple_->addToVector(prefix + "toy_resolution_scale",       res_scale);
				flat_tuple_->addToVector(prefix + "toy_mass_resolution",        bump_hunter_->getMassResolution(mass_hypo_));
				
				// Set the fit results in the tuple.
				flat_tuple_->addToVector(prefix + "toy_bkg_chi2_prob",          bkg_result->Prob());
				flat_tuple_->addToVector(prefix + "toy_bkgsig_chi2_prob",       sig_result->Prob());
				flat_tuple_->addToVector(prefix + "toy_bkg_edm",                bkg_result->Edm());
				flat_tuple_->addToVector(prefix + "toy_bkg_minuit_status",      bkg_result->Status());
				flat_tuple_->addToVector(prefix + "toy_bkg_nll",                bkg_result->MinFcnValue());
				
				flat_tuple_->addToVector(prefix + "toy_edm",                    sig_result->Edm());
				flat_tuple_->addToVector(prefix + "toy_minuit_status",          sig_result->Status());
				flat_tuple_->addToVector(prefix + "toy_nll",                    sig_result->MinFcnValue());
				flat_tuple_->addToVector(prefix + "toy_p_value",                result->getPValue());
				flat_tuple_->addToVector(prefix + "toy_q0",                     result->getQ0());
				flat_tuple_->addToVector(prefix + "toy_bkg_rate_mass_hypo",     result->getFullBkgRate());
				flat_tuple_->addToVector(prefix + "toy_bkg_rate_mass_hypo_err", result->getFullBkgRateError());
				flat_tuple_->addToVector(prefix + "toy_sig_yield",              result->getSignalYield());
				flat_tuple_->addToVector(prefix + "toy_sig_yield_err",          result->getSignalYieldErr());
				flat_tuple_->addToVector(prefix + "toy_upper_limit",            result->getUpperLimit());
				flat_tuple_->addToVector(prefix + "toy_ul_p_value",             result->getUpperLimitPValue());
            }
            
            // Step to the next toy.
            toyFitN++;
        }
    }
    
    // Fill and write the tuple
    flat_tuple_->fill();
    flat_tuple_->close();
    
    return true;
}

void BhMassResSystematicsProcessor::finalize() {
    inF_->Close();
    delete inF_;
    delete bump_hunter_;
}

DECLARE_PROCESSOR(BhMassResSystematicsProcessor);