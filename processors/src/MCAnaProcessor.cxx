/**
 * @file MCAnaProcessor.cxx
 * @brief AnaProcessor used fill histograms to compare simulations
 * @author Cameron Bravo, SLAC National Accelerator Laboratory
 */     
#include "MCAnaProcessor.h"
#include <iostream>

MCAnaProcessor::MCAnaProcessor(const std::string& name, Process& process) : Processor(name,process){}
//TODO CHECK THIS DESTRUCTOR
MCAnaProcessor::~MCAnaProcessor(){}


void MCAnaProcessor::configure(const ParameterSet& parameters) {
    std::cout << "Configuring MCAnaProcessor" << std::endl;
    try
    {
        debug_           = parameters.getInteger("debug");
        anaName_         = parameters.getString("anaName");
        partColl_        = parameters.getString("partColl");
        trkrHitColl_     = parameters.getString("trkrHitColl");
        ecalHitColl_     = parameters.getString("ecalHitColl");
        histCfgFilename_ = parameters.getString("histCfg");
    }
    catch (std::runtime_error& error)
    {
        std::cout << error.what() << std::endl;
    }
}

void MCAnaProcessor::initialize(TTree* tree) {
    tree_= tree;
    // init histos
    histos = new MCAnaHistos(anaName_);
    histos->loadHistoConfig(histCfgFilename_);
    histos->DefineHistos();
    //histos->Define2DHistos();

    // init TTree
    tree_->SetBranchAddress(partColl_.c_str(), &mcParts_, &bmcParts_);
    tree_->SetBranchAddress(trkrHitColl_.c_str(), &mcTrkrHits_, &bmcTrkrHits_);
    tree_->SetBranchAddress(ecalHitColl_.c_str(), &mcEcalHits_, &bmcEcalHits_);

}

bool MCAnaProcessor::process(IEvent* ievent) {

    histos->FillMCParticles(mcParts_);
    histos->FillMCTrackerHits(mcTrkrHits_);
    histos->FillMCEcalHits(mcEcalHits_);

    return true;
}

void MCAnaProcessor::finalize() {

    histos->saveHistos(outF_, anaName_);
    delete histos;
    histos = nullptr;
}

DECLARE_PROCESSOR(MCAnaProcessor);
