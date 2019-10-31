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

}

void RecoHitAnaProcessor::initialize(TTree* tree) {
    tree_= tree;
    // init histos
    histos = new RecoHitAnaHistos("recoHitAna");
    histos->Define1DHistos();
    //histos->Define2DHistos();

    // init TTree
    tree_->SetBranchAddress(Collections::TRACKER_HITS , &trkrHits_    , &btrkrHits_    );
    tree_->SetBranchAddress(Collections::GBL_TRACKS   , &tracks_      , &btracks_      );
    tree_->SetBranchAddress(Collections::ECAL_HITS    , &ecalHits_    , &becalHits_    );
    tree_->SetBranchAddress(Collections::ECAL_CLUSTERS, &ecalClusters_, &becalClusters_);

}

bool RecoHitAnaProcessor::process(IEvent* ievent) {

    histos->FillTrackerHits(trkrHits_);
    histos->FillTracks(tracks_);
    histos->FillEcalHits(ecalHits_);
    histos->FillEcalClusters(ecalClusters_);

    return true;
}

void RecoHitAnaProcessor::finalize() {

    histos->saveHistos(outF_,"");
    delete histos;
    histos = nullptr;
}

DECLARE_PROCESSOR(RecoHitAnaProcessor);
