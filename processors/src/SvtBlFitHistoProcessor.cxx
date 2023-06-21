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
        thresholdsFileIn_ = parameters.getString("thresholdsFileIn");
        layer_ = parameters.getString("layer");
        rebin_ = parameters.getInteger("rebin");
        minStats_ = parameters.getInteger("minStats");
        deadRMS_ = parameters.getInteger("deadRMS");
        debug_ = parameters.getInteger("debug");
        year_ = parameters.getInteger("year");
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
    fitHistos_ = new BlFitHistos(year_);
    fitHistos_->setDebug(debug_);
    std::cout << "[BlFitHistos] Loading 2D Histos" << std::endl;
    fitHistos_->loadHistoConfig(rawhitsHistCfgFilename_);
    fitHistos_->getHistosFromFile(inF_,layer_);


   //Setup flat tuple branches
    //Name of each hybrid
    flat_tuple_->addString("halfmodule_hh");
    //number of entries in channel
    flat_tuple_->addString("n_entries");
    //minimum required entries in channel to attempt fit
    flat_tuple_->addVariable("minStats");
    //channel rebinning setting
    flat_tuple_->addVariable("rebin");
    //channel number
    flat_tuple_->addVariable("channel");
    //svt_id
    flat_tuple_->addVariable("svt_id");
    //if 1.0, channel stats too low to perform fit
    flat_tuple_->addVariable("lowStats");
    //channel rms. Use to flag dead channels
    flat_tuple_->addVariable("rms");
    //if rms < threshold, channel is dead
    flat_tuple_->addVariable("dead");
    //if channel fit is grossly poor
    flat_tuple_->addVariable("badfit");
    //if fit mean + N*sigma > xmax, flag as lowdaq
    flat_tuple_->addVariable("lowdaq");
    //if fit mean + N*sigma > xmax, flag as lowdaq
    flat_tuple_->addVariable("suplowDaq");
    //Threshold value setting on apv channel during run
    flat_tuple_->addVariable("threshold");
    flat_tuple_->addVariable("minthreshold");

  //Fit values
    flat_tuple_->addVariable("BlFitMean");
    flat_tuple_->addVariable("BlFitNorm");
    flat_tuple_->addVariable("BlFitSigma");
    flat_tuple_->addVariable("BlFitRangeLower");
    flat_tuple_->addVariable("BlFitRangeUpper");
    flat_tuple_->addVariable("BlFitChi2");
    flat_tuple_->addVariable("BlFitNdf");
}


bool SvtBlFitHistoProcessor::process() { 
    if(debug_)
        std::cout << "RUNNING PROCESS ON 2d HISTOS" << std::endl;
    std::map<std::string,TH2F*> histos2d = fitHistos_->get2dHistos();
    std::map<std::string, TH2F*>::iterator it;
    for(it = histos2d.begin(); it != histos2d.end(); it++)
        std::cout << "2d Histo Loaded: " << it->first << std::endl;
    if(debug_)
        std::cout << "FIT 2d HISTO Baselines" << std::endl;
    fitHistos_->fit2DHistoChannelBaselines(histos2d,rebin_,minStats_, deadRMS_, thresholdsFileIn_, flat_tuple_);
    return true;
}

void SvtBlFitHistoProcessor::finalize() {

    std::cout << "finalizing SvtBlFitHistoProcessor" << std::endl;
    flat_tuple_->close();
    outF_->Close();
}

DECLARE_PROCESSOR(SvtBlFitHistoProcessor);
