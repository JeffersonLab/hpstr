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
    flat_tuple_->addString("n_entries");
    flat_tuple_->addVariable("minbinThresh");
    flat_tuple_->addVariable("minStats");
    flat_tuple_->addVariable("rebin");
    flat_tuple_->addVariable("channel");
    flat_tuple_->addVariable("minbinFail");
    flat_tuple_->addVariable("rms");
    flat_tuple_->addVariable("noisy");
    flat_tuple_->addVariable("lowdaq");

    flat_tuple_->addVariable("BlFitMean");
    flat_tuple_->addVariable("BlFitNorm");
    flat_tuple_->addVariable("BlFitSigma");
    flat_tuple_->addVariable("BlFitRangeLower");
    flat_tuple_->addVariable("BlFitRangeUpper");
    flat_tuple_->addVariable("BlFitChi2");
    flat_tuple_->addVariable("BlFitNdf");


    flat_tuple_->addVector("iterChi2NDF");
    flat_tuple_->addVector("iterFitRangeEnd");
    flat_tuple_->addVector("iterMean");
    flat_tuple_->addVector("iterChi2NDF_2der");
    flat_tuple_->addVector("iterChi2NDF_1der");
    flat_tuple_->addVector("ratio2derChi2");
    flat_tuple_->addVector("iterChi2NDF_derRange");

    flat_tuple_->addVariable("ogxmax");
    flat_tuple_->addVariable("ogxmin");
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
