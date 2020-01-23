/**
 * @file BhToysHistoProcessor.cxx
 * @brief Processor used to throw toys to study BH bkg model complexity
 * @author Cameron Bravo, SLAC National Accelerator Laboratory
 */

#include "BhToysHistoProcessor.h"

BhToysHistoProcessor::BhToysHistoProcessor(const std::string& name, Process& process)
    : Processor(name, process) { 
    }

BhToysHistoProcessor::~BhToysHistoProcessor() { 
}

void BhToysHistoProcessor::configure(const ParameterSet& parameters) {

    std::cout << "Configuring BhToysHistoProcessor" << std::endl;
    try
    {
        debug_          = parameters.getInteger("debug");
        massSpectrum_   = parameters.getString("massSpectrum");
        mass_hypo_      = parameters.getDouble("mass_hypo");
        win_factor_     = parameters.getInteger("win_factor");
        poly_order_     = parameters.getInteger("poly_order");
        seed_           = parameters.getInteger("seed");
        nToys_          = parameters.getInteger("nToys");
        // MODIFIED
        toy_sig_samples_ = parameters.getInteger("toy_sig_samples");
        // MODIFIED
    }
    catch (std::runtime_error& error)
    {
        std::cout << error.what() << std::endl;
    }


}

void BhToysHistoProcessor::initialize(std::string inFilename, std::string outFilename) {
    // Init Files
    inF_ = new TFile(inFilename.c_str());

    // Get mass spectrum from file
    mass_spec_h = (TH1*) inF_->Get(massSpectrum_.c_str());

    // Init bump hunter manager
    bump_hunter_ = new BumpHunter(bkg_model_, poly_order_, win_factor_);
    bump_hunter_->setBounds(mass_spec_h->GetXaxis()->GetBinUpEdge(mass_spec_h->FindFirstBinAbove()),
            mass_spec_h->GetXaxis()->GetBinLowEdge(mass_spec_h->FindLastBinAbove()));
    if(debug_ > 0) bump_hunter_->enableDebug();

    // Init FlatTupleMaker
    flat_tuple_ = new FlatTupleMaker(outFilename.c_str(), "fit_toys");

    // Setup flat tuple branches
    flat_tuple_->addVariable("bkg_total");
    flat_tuple_->addVariable("corr_mass");
    flat_tuple_->addVariable("mass_hypo");
    flat_tuple_->addVariable("poly_order");
    flat_tuple_->addVariable("win_factor");
    flat_tuple_->addVariable("window_size");

    flat_tuple_->addVariable("bkg_chi2_prob");
    flat_tuple_->addVariable("bkg_edm");
    flat_tuple_->addVariable("bkg_minuit_status");
    flat_tuple_->addVariable("bkg_nll");

    flat_tuple_->addVariable("edm");
    flat_tuple_->addVariable("minuit_status");
    flat_tuple_->addVariable("nll");
    flat_tuple_->addVariable("p_value");
    flat_tuple_->addVariable("q0");
    flat_tuple_->addVariable("sig_yield");
    flat_tuple_->addVariable("sig_yield_err");
    flat_tuple_->addVariable("upper_limit");
    flat_tuple_->addVariable("ul_p_value");
    flat_tuple_->addVariable("ul_minuit_status");
    flat_tuple_->addVector("ul_nlls");
    flat_tuple_->addVector("ul_sig_yields");

    flat_tuple_->addVariable("seed");
    // MODIFIED
    flat_tuple_->addVariable("toy_sig_samples");
    // MODIFIED
    flat_tuple_->addVector("toy_bkg_chi2_prob");
    flat_tuple_->addVector("toy_bkg_edm");
    flat_tuple_->addVector("toy_bkg_minuit_status");
    flat_tuple_->addVector("toy_bkg_nll");
    flat_tuple_->addVector("toy_minuit_status");
    flat_tuple_->addVector("toy_nll");
    flat_tuple_->addVector("toy_p_value");
    flat_tuple_->addVector("toy_q0");
    flat_tuple_->addVector("toy_sig_yield");
    flat_tuple_->addVector("toy_sig_yield_err");
    flat_tuple_->addVector("toy_upper_limit");

}

bool BhToysHistoProcessor::process() {

    std::cout << "Running on mass spectrum: " << massSpectrum_ << std::endl;
    std::cout << "Running with polynomial order: " << poly_order_ << std::endl;
    std::cout << "Running with window factor: " << win_factor_ << std::endl;
    std::cout << "Running on mass hypo: " << mass_hypo_ << std::endl;

    // Search for a resonance at the given mass hypothesis
    HpsFitResult* result = bump_hunter_->performSearch(mass_spec_h, mass_hypo_, false, true);
    mass_spec_h->Write();

    // Get the result of the background fit
    TFitResultPtr bkg_result = result->getBkgFitResult();

    std::cout << "Filling Fit Results " << std::endl;
    // Get the result of the signal+background fit
    TFitResultPtr sig_result = result->getCompFitResult();

    // Set the Fit Parameters in the flat tuple
    flat_tuple_->setVariableValue("bkg_total",              result->getIntegral());
    flat_tuple_->setVariableValue("corr_mass",              result->getCorrectedMass());
    flat_tuple_->setVariableValue("mass_hypo",              result->getMass());
    flat_tuple_->setVariableValue("poly_order",             poly_order_);
    flat_tuple_->setVariableValue("win_factor",             win_factor_);
    flat_tuple_->setVariableValue("window_size",            result->getWindowSize());

    // Set the Fit Results in the flat tuple
    flat_tuple_->setVariableValue("bkg_chi_prob",           bkg_result->Prob());
    flat_tuple_->setVariableValue("bkg_edm",                bkg_result->Edm());
    flat_tuple_->setVariableValue("bkg_minuit_status",      bkg_result->Status());
    flat_tuple_->setVariableValue("bkg_nll",                bkg_result->MinFcnValue());

    flat_tuple_->setVariableValue("edm",                    sig_result->Edm());
    flat_tuple_->setVariableValue("minuit_status",          sig_result->Status());
    flat_tuple_->setVariableValue("nll",                    sig_result->MinFcnValue());
    flat_tuple_->setVariableValue("p_value",                result->getPValue());
    flat_tuple_->setVariableValue("q0",                     result->getQ0());
    flat_tuple_->setVariableValue("sig_yield",              result->getSignalYield());
    flat_tuple_->setVariableValue("sig_yield_err",          result->getSignalYieldErr());
    flat_tuple_->setVariableValue("upper_limit",            result->getUpperLimit());
    flat_tuple_->setVariableValue("upper_limit_p_value",    result->getUpperLimitPValue());

    for (auto& likelihood : result->getLikelihoods()) {
        flat_tuple_->addToVector("nlls", likelihood);
    }

    for (auto& yield : result->getSignalYields()) {
        flat_tuple_->addToVector("sig_yields", yield);
    }

    std::vector<HpsFitResult*> toy_results;
    flat_tuple_->setVariableValue("seed", seed_);
    // MODIFIED
    flat_tuple_->setVariableValue("toy_sig_samples", toy_sig_samples_);
    // MODIFIED
    
    if (nToys_ > 0) {

        std::cout << "Generating " << nToys_ << " Toys" <<std::endl;
        std::vector<TH1*> toys_hist = bump_hunter_->generateToys(mass_spec_h, nToys_, seed_, toy_sig_samples_);

        int toyFitN = 0;
        for (TH1* hist : toys_hist) {
            std::cout << "Fitting Toy " << toyFitN << std::endl;
            toy_results.push_back(bump_hunter_->performSearch(hist, mass_hypo_, false, true));
            toyFitN++;
        }
    }

    for (auto& toy_result : toy_results) {

        // Get the result of the background fit
        TFitResultPtr toy_bkg_result = toy_result->getBkgFitResult();

        flat_tuple_->addToVector("toy_bkg_chi2_prob",     toy_bkg_result->Prob());
        flat_tuple_->addToVector("toy_bkg_edm",           toy_bkg_result->Edm());
        flat_tuple_->addToVector("toy_bkg_minuit_status", toy_bkg_result->Status());
        flat_tuple_->addToVector("toy_bkg_nll",           toy_bkg_result->MinFcnValue());

        // Get the result of the signal+background fit
        TFitResultPtr toy_sig_result = toy_result->getCompFitResult();

        // Retrieve all of the result of interest. 
        flat_tuple_->addToVector("toy_minuit_status", toy_sig_result->Status());
        flat_tuple_->addToVector("toy_nll",           toy_sig_result->MinFcnValue());
        flat_tuple_->addToVector("toy_p_value",       toy_result->getPValue());
        flat_tuple_->addToVector("toy_q0",            toy_result->getQ0());
        flat_tuple_->addToVector("toy_sig_yield",     toy_result->getSignalYield());
        flat_tuple_->addToVector("toy_sig_yield_err", toy_result->getSignalYieldErr());
        flat_tuple_->addToVector("toy_upper_limit",   toy_result->getUpperLimit());
    }

    // Fill and write the flat tuple
    flat_tuple_->fill();
    flat_tuple_->close();

    return true;
}

void BhToysHistoProcessor::finalize() { 
    inF_->Close();
    delete inF_;
    //delete flat_tuple_;
    delete bump_hunter_;
}

DECLARE_PROCESSOR(BhToysHistoProcessor); 
