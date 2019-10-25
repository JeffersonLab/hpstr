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

}

void MCAnaProcessor::initialize(TTree* tree) {
    tree_= tree;
    // init histos
    histos = new MCAnaHistos("hitOnTrack_2D");
    histos->Define1DHistos();
    //histos->Define2DHistos();

    // init TClonesArrays
    mcParts_    = new TClonesArray("MCParticle"  , 1000000);
    mcTrkrHits_ = new TClonesArray("MCTrackerHit", 1000000);
    mcEcalHits_ = new TClonesArray("MCEcalHit"   , 1000000);

    // init TTree
    tree_->SetBranchAddress(Collections::MC_PARTICLES, &mcParts_, &bmcParts_);
    tree_->SetBranchAddress(Collections::MC_TRACKER_HITS, &mcTrkrHits_, &bmcTrkrHits_);
    tree_->SetBranchAddress(Collections::MC_ECAL_HITS, &mcEcalHits_, &bmcEcalHits_);

}

bool MCAnaProcessor::process(IEvent* ievent) {

    histos->FillMCParticles(mcParts_);
    histos->FillMCTrackerHits(mcTrkrHits_);
    histos->FillMCEcalHits(mcEcalHits_);

    return true;
}

void MCAnaProcessor::finalize() {

    histos->saveHistos(outF_,"");
    delete histos;
    histos = nullptr;
}

DECLARE_PROCESSOR(MCAnaProcessor);
