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
        polOrder_       = parameters.getInteger("pol_order");
        massHypo_       = parameters.getDouble("mass_hypo");
    }
    catch (std::runtime_error& error)
    {
        std::cout << error.what() << std::endl;
    }


}

void BhToysHistoProcessor::initialize(std::string inFilename, std::string outFilename) {
    inF_ = new TFile(inFilename.c_str());
    outF_ = new TFile(outFilename.c_str(),"RECREATE");
    //TODO: Init BumpHunter

}

bool BhToysHistoProcessor::process() {

    std::cout << "Running on mass spectrum: " << massSpectrum_ << std::endl;
    std::cout << "Running with pol order: " << polOrder_ << std::endl;
    std::cout << "Running on mass hypo: " << massHypo_ << std::endl;

    return true;
}

void BhToysHistoProcessor::finalize() { 
    inF_->Close();
    outF_->Close();
    delete inF_;
    delete outF_;
}

DECLARE_PROCESSOR(BhToysHistoProcessor); 
