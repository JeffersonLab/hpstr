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
    //outF_chi2 = new TFile(Form("chi2_%s",outFilename.c_str()));
    flat_tuple_ = new FlatTupleMaker(outFilename.c_str(), "gaus_fit");

    //outputHistos_ = new BlFitHistos("raw_hits");
    inputHistos_ = new HistoManager("");
    std::cout << "[BlFitHistos] Loading 2D Histos" << std::endl;
    inputHistos_->GetHistosFromFile(inF_, hybrid_);
    //std::cout << "[BlFitHistos] Loading json file" << std::endl;
    //outputHistos_->loadHistoConfig(histCfgFilename_); 
    //std::cout << "[BlFitHistos] Creating Histograms for Fit Parameters" << std::endl;
    //outputHistos_->DefineHistos();

}


bool SvtBlFitHistoProcessor::process() { 
    outputHistos_->Chi2GausFit(inputHistos_,nPointsDer_,rebin_,xmin_, flat_tuple_);
    //outputHistos_->Mean2DHistoOverlay(inputHistos_,outputHistos_);

    return true;
    

    
}

void SvtBlFitHistoProcessor::finalize() {

    std::cout << "finalizing SvtBlFitHistoProcessor" << std::endl;
    //outputHistos_->saveHistos(outF_,"");
    outF_->Close();
    //delete outputHistos_;
    //outputHistos_ = nullptr;
    delete inputHistos_;
    inputHistos_ = nullptr;
    //delete flat_tuple_;
    //flat_tuple_ = nullptr;
}

DECLARE_PROCESSOR(SvtBlFitHistoProcessor);
