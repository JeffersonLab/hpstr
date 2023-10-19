/**
 * @file SimPartProcessor.cxx
 * @brief SimPartProcessor used fill histograms to check acceptance of simulated particle source
 * @author Abhisek Datta, University of California, Los Angeles
 */     
#include "SimPartProcessor.h"
#include <iostream>

SimPartProcessor::SimPartProcessor(const std::string& name, Process& process) : Processor(name,process){}
//TODO CHECK THIS DESTRUCTOR
SimPartProcessor::~SimPartProcessor(){}


void SimPartProcessor::configure(const ParameterSet& parameters) {
    std::cout << "Configuring SimPartProcessor" << std::endl;
    try
    {
        debug_           = parameters.getInteger("debug");
        anaName_         = parameters.getString("anaName");
        MCParticleColl_        = parameters.getString("MCParticleColl");
        MCTrackerHitColl_     = parameters.getString("MCTrackerHitColl");
        MCEcalHitColl_     = parameters.getString("MCEcalHitColl");
        RecoTrackColl_     = parameters.getString("RecoTrackColl");
        RecoTrackerClusterColl_     = parameters.getString("RecoTrackerClusterColl");
        RecoEcalClusterColl_     = parameters.getString("RecoEcalClusterColl");
        histCfgFilename_ = parameters.getString("histCfg");
        analysis_        = parameters.getString("analysis");
    }
    catch (std::runtime_error& error)
    {
        std::cout << error.what() << std::endl;
    }
}

void SimPartProcessor::initialize(TTree* tree) {
    tree_= tree;
    // init histos
    histos = new SimPartHistos(anaName_);
    histos->loadHistoConfig(histCfgFilename_);
    histos->DefineHistos();

    // init TTree
    tree_->SetBranchAddress(MCParticleColl_.c_str(), &MCParticles_, &bMCParticles_);

    if (tree_->FindBranch(MCTrackerHitColl_.c_str()))
        tree_->SetBranchAddress(MCTrackerHitColl_.c_str(), &MCTrackerHits_, &bMCTrackerHits_);
    else
        std::cout<<"WARNING: No MC tracker hit collection"<<std::endl;
    if ( tree_->FindBranch(MCEcalHitColl_.c_str()))
        tree_->SetBranchAddress(MCEcalHitColl_.c_str(), &MCECalHits_, &bMCECalHits_);
    else
        std::cout<<"WARNING: No MC Ecal hit collection"<<std::endl;

    if (tree_->FindBranch(RecoTrackColl_.c_str()))
        tree_->SetBranchAddress(RecoTrackColl_.c_str(), &RecoTracks_, &bRecoTracks_);
    else
        std::cout<<"WARNING: No Reco track collection"<<std::endl;

    if (tree_->FindBranch(RecoTrackerClusterColl_.c_str()))
        tree_->SetBranchAddress(RecoTrackerClusterColl_.c_str(), &RecoTrackerClusters_, &bRecoTrackerClusters_);
    else
        std::cout<<"WARNING: No Reco tracker hit collection"<<std::endl;

    if (tree_->FindBranch(RecoEcalClusterColl_.c_str()))
        tree_->SetBranchAddress(RecoEcalClusterColl_.c_str(), &RecoEcalClusters_, &bRecoEcalClusters_);
    else
        std::cout<<"WARNING: No Reco Ecal hit collection"<<std::endl;
}

bool SimPartProcessor::process(IEvent* ievent) {

    histos->FillAcceptance(MCParticles_, RecoTracks_, RecoTrackerClusters_, RecoEcalClusters_);
    histos->FillEfficiency(MCParticles_, RecoTracks_, MCTrackerHits_, MCECalHits_, RecoTrackerClusters_, RecoEcalClusters_);

    return true;
}

void SimPartProcessor::finalize() {

    histos->saveHistos(outF_, anaName_);
    delete histos;
    histos = nullptr;
}

DECLARE_PROCESSOR(SimPartProcessor);
