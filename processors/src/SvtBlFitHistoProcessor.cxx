#include "SvtBlFitHistoProcessor.h"
#include <FlatTupleMaker.h>
#include <string>
#include <algorithm>
#include <cstdlib>
SvtBlFitHistoProcessor::SvtBlFitHistoProcessor(const std::string& name, Process& process)
    : Processor(name, process) {
    }

SvtBlFitHistoProcessor::~SvtBlFitHistoProcessor() {
}

void SvtBlFitHistoProcessor::configure(const ParameterSet& parameters) {

    std::cout << "[SvtBlFitHistoProcessor] Configuring" << std::endl;
    try
    {
        outDir_ = parameters.getString("outDir");
        histCfgFilename_ = parameters.getString("histCfg");
        hybrid_ = parameters.getVString("hybrid");
        IterativeGaussFitCut_ = parameters.getInteger("nhitsFitCut");
        rebin_ = parameters.getInteger("rebin");
        nPointsDer_ = parameters.getInteger("nPoints");
        xmin_ = parameters.getInteger("xmin");
        minStats_ = parameters.getInteger("minStats");
    }
    catch (std::runtime_error& error)
    {
        std::cout << error.what() << std::endl;
    }

}

void SvtBlFitHistoProcessor::initialize(std::string inFilename, std::string outFilename) {

    std::cout << "initializing SvtBlFitHistoProcessor" << std::endl;
    //InFile containing 2D histograms from the SvtCondAnaProcessor
    inF_ = new TFile(inFilename.c_str());
    outF_ = new TFile(outFilename.c_str(),"RECREATE");
    flat_tuple_ = new FlatTupleMaker(outFilename.c_str(), "gaus_fit");

    inputHistos_ = new HistoManager("");
    std::cout << "[BlFitHistos] Loading 2D Histos" << std::endl;
    inputHistos_->GetHistosFromFile(inF_, hybrid_);

   //Setup flat tuple branches
    flat_tuple_->addString("SvtAna2DHisto_key");
    flat_tuple_->addVariable("minimum_bin_threshold");
    flat_tuple_->addVariable("minimum_entry_requirement_per_channel");
    flat_tuple_->addVariable("rebin_factor");
    flat_tuple_->addVariable("channel");
    flat_tuple_->addVariable("minStats_dead_channel");
    flat_tuple_->addVariable("baseline_gausFit_mean");
    flat_tuple_->addVariable("baseline_gausFit_norm");
    flat_tuple_->addVariable("baseline_gausFit_sigma");
    flat_tuple_->addVariable("baseline_gausFit_range_lower");
    flat_tuple_->addVariable("baseline_gausFit_range_upper");
    flat_tuple_->addVector("iterativeFit_chi2_NDF");
    flat_tuple_->addVector("iterativeFit_range_end");
    flat_tuple_->addVector("iterativeFit_mean");
    flat_tuple_->addVector("iterativeFit_chi2_2ndDerivative");
    flat_tuple_->addVector("iterativeFit_chi2_2Der_range");
}


bool SvtBlFitHistoProcessor::process() { 
    outputHistos_->Chi2GausFit(inputHistos_,nPointsDer_,rebin_,xmin_,minStats_, flat_tuple_);
    //outputHistos_->Mean2DHistoOverlay(inputHistos_,outputHistos_);
    return true;
    

    
}

void SvtBlFitHistoProcessor::finalize() {

    std::cout << "finalizing SvtBlFitHistoProcessor" << std::endl;
    //outputHistos_->saveHistos(outF_,"");
    flat_tuple_->close();
    outF_->Close();
    //delete outputHistos_;
    //outputHistos_ = nullptr;
    delete inputHistos_;
    inputHistos_ = nullptr;
    //delete flat_tuple_;
    //flat_tuple_ = nullptr;
}

DECLARE_PROCESSOR(SvtBlFitHistoProcessor);
