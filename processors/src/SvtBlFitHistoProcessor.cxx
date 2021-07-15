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
        histCfgFilename_ = parameters.getString("histCfg");
        rawhitsHistCfgFilename_ = parameters.getString("rawhitsHistCfg");
        layer_ = parameters.getString("layer");
        rebin_ = parameters.getInteger("rebin");
        nPointsDer_ = parameters.getInteger("nPoints");
        xmin_ = parameters.getInteger("xmin");
        minStats_ = parameters.getInteger("minStats");
        noisyRMS_ = parameters.getInteger("noisy");
        deadRMS_ = parameters.getInteger("deadRMS");
        simpleGausFit_ = parameters.getString("simpleGausFit");
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

    //init tuple to store fit values
    flat_tuple_ = new FlatTupleMaker(outFilename.c_str(), "gaus_fit");

    //Initialize fit histos
    fitHistos_ = new BlFitHistos();
    //To fit channels with a simple gaussian, set configurable param to true
    fitHistos_->setSimpleGausFit(simpleGausFit_);
    std::cout << "[BlFitHistos] Loading 2D Histos" << std::endl;
    fitHistos_->loadHistoConfig(rawhitsHistCfgFilename_);
    fitHistos_->getHistosFromFile(inF_,layer_);


   //Setup flat tuple branches
    //Name of each hybrid
    flat_tuple_->addString("halfmodule_hh");
    //number of entries in channel
    flat_tuple_->addString("n_entries");
    //minimum required counts in bin to start fit
    flat_tuple_->addVariable("minbinThresh");
    //minimum required entries in channel to attempt fit
    flat_tuple_->addVariable("minStats");
    //channel rebinning setting
    flat_tuple_->addVariable("rebin");
    //channel number
    flat_tuple_->addVariable("channel");
    //svt_id
    flat_tuple_->addVariable("svt_id");
    //if 1.0, channel stats too low to perform fit
    flat_tuple_->addVariable("minStatsFailure");
    //channel rms. Use to flag dead channels
    flat_tuple_->addVariable("rms");
    //if fit mean + N*sigma > xmax, flag as lowdaq
    flat_tuple_->addVariable("lowdaq");
    //if rms < threshold, channel is dead
    flat_tuple_->addVariable("dead");

  //Fit values
    flat_tuple_->addVariable("BlFitMean");
    flat_tuple_->addVariable("BlFitNorm");
    flat_tuple_->addVariable("BlFitSigma");
    flat_tuple_->addVariable("BlFitRangeLower");
    flat_tuple_->addVariable("BlFitRangeUpper");
    flat_tuple_->addVariable("BlFitChi2");
    flat_tuple_->addVariable("BlFitNdf");

 //Intermediate fit values...perhaps not interested in keeping anymore
    flat_tuple_->addVector("iterChi2NDF");
    flat_tuple_->addVector("iterFitRangeEnd");
    flat_tuple_->addVector("iterMean");
    flat_tuple_->addVector("iterChi2NDF_2der");
    flat_tuple_->addVector("iterChi2NDF_1der");
    flat_tuple_->addVector("iterChi2NDF_derRange");
    flat_tuple_->addVariable("ogxmax");
    flat_tuple_->addVariable("ogxmin");
    
  //This flags if there is a TFitResult Error returned during a fit
  //flag value set by multiplying unique prime numbers at different stages of fitting
  //Can pinpoint fit failure in code by prime number factorization
    flat_tuple_->addVariable("TFitResultError");
}


bool SvtBlFitHistoProcessor::process() { 
    std::map<std::string,TH2F*> histos2d = fitHistos_->get2dHistos();
    std::map<std::string, TH2F*>::iterator it;
    for(it = histos2d.begin(); it != histos2d.end(); it++)
        std::cout << "2d Histo Loaded: " << it->first << std::endl;
    fitHistos_->Chi2GausFit(histos2d,nPointsDer_,rebin_,xmin_,minStats_, noisyRMS_, deadRMS_,flat_tuple_);
    return true;
}

void SvtBlFitHistoProcessor::finalize() {

    std::cout << "finalizing SvtBlFitHistoProcessor" << std::endl;
    flat_tuple_->close();
    outF_->Close();
}

DECLARE_PROCESSOR(SvtBlFitHistoProcessor);
