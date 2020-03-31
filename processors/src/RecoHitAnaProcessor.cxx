/**
 * @file RecoHitAnaProcessor.cxx
 * @brief AnaProcessor used fill histograms to compare simulations
 * @author Cameron Bravo, SLAC National Accelerator Laboratory
 */     
#include "RecoHitAnaProcessor.h"
#include <iostream>

RecoHitAnaProcessor::RecoHitAnaProcessor(const std::string& name, Process& process) : Processor(name,process){}
//TODO CHECK THIS DESTRUCTOR
RecoHitAnaProcessor::~RecoHitAnaProcessor(){}


void RecoHitAnaProcessor::configure(const ParameterSet& parameters) {
    std::cout << "Configuring RecoHitAnaProcessor" << std::endl;
    try
    {
        debug_           = parameters.getInteger("debug");
        anaName_         = parameters.getString("anaName");
        trkColl_         = parameters.getString("trkColl");
        trkrHitColl_     = parameters.getString("trkrHitColl");
        ecalHitColl_     = parameters.getString("ecalHitColl");
        ecalClusColl_    = parameters.getString("ecalClusColl");
        histCfgFilename_ = parameters.getString("histCfg");
    }
    catch (std::runtime_error& error)
    {
        std::cout << error.what() << std::endl;
    }

}

void RecoHitAnaProcessor::initialize(TTree* tree) {
    tree_= tree;
    // init histos
    histos = new RecoHitAnaHistos(anaName_.c_str());
    histos->loadHistoConfig(histCfgFilename_);
    histos->DefineHistos();

    // init TTree
    tree_->SetBranchAddress(trkrHitColl_.c_str()  , &trkrHits_    , &btrkrHits_    );
    tree_->SetBranchAddress(trkColl_.c_str()      , &tracks_      , &btracks_      );
    tree_->SetBranchAddress(ecalHitColl_.c_str()  , &ecalHits_    , &becalHits_    );
    tree_->SetBranchAddress(ecalClusColl_.c_str() , &ecalClusters_, &becalClusters_);

}

bool RecoHitAnaProcessor::process(IEvent* ievent) {

    histos->FillTrackerHits(trkrHits_);
    histos->FillTracks(tracks_);
    histos->FillEcalHits(ecalHits_);
    histos->FillEcalClusters(ecalClusters_);

    return true;
}

void RecoHitAnaProcessor::finalize() {

    histos->saveHistos(outF_, anaName_.c_str());
    delete histos;
    histos = nullptr;
}

DECLARE_PROCESSOR(RecoHitAnaProcessor);
