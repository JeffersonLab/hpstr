/**
 * @author Tongtong Cao, UNH
 */
#include "RecoTrackAnaProcessor.h"
#include <iostream>

RecoTrackAnaProcessor::RecoTrackAnaProcessor(const std::string& name, Process& process) : Processor(name,process){}
//TODO CHECK THIS DESTRUCTOR
RecoTrackAnaProcessor::~RecoTrackAnaProcessor(){}


void RecoTrackAnaProcessor::configure(const ParameterSet& parameters) {
    std::cout << "Configuring RecoTrackAnaProcessor" << std::endl;
    try
    {
        debug_           = parameters.getInteger("debug");
        anaName_         = parameters.getString("anaName");
        trackHitColl_     = parameters.getString("trackHitColl");
        trackClusColl_    = parameters.getString("trackClusColl");
        histCfgFilename_ = parameters.getString("histCfg");
    }
    catch (std::runtime_error& error)
    {
        std::cout << error.what() << std::endl;
    }

}

void RecoTrackAnaProcessor::initialize(TTree* tree) {
    tree_= tree;
    // init histos
    histos = new RecoTrackAnaHistos(anaName_.c_str());
    histos->loadHistoConfig(histCfgFilename_);
    histos->DefineHistos();

    // init TTree
    tree_->SetBranchAddress(trackHitColl_.c_str()  , &trackHits_    , &btrackHits_    );
    tree_->SetBranchAddress(trackClusColl_.c_str() , &trackClusters_, &btrackClusters_);

}

bool RecoTrackAnaProcessor::process(IEvent* ievent) {

    histos->FillTrackHits(trackHits_);
    histos->FillTrackClusters(trackClusters_);

    return true;
}

void RecoTrackAnaProcessor::finalize() {

    histos->saveHistos(outF_, anaName_.c_str());
    delete histos;
    histos = nullptr;
}

DECLARE_PROCESSOR(RecoTrackAnaProcessor);
