/**
 * @author Tongtong Cao, UNH
 */
#include "BeamRotationTargetOffsettingAnaProcessor.h"
#include <iostream>

BeamRotationTargetOffsettingAnaProcessor::BeamRotationTargetOffsettingAnaProcessor(const std::string& name, Process& process) : Processor(name,process){}
//TODO CHECK THIS DESTRUCTOR
BeamRotationTargetOffsettingAnaProcessor::~BeamRotationTargetOffsettingAnaProcessor(){}


void BeamRotationTargetOffsettingAnaProcessor::configure(const ParameterSet& parameters) {
    std::cout << "Configuring BeamRotationTargetOffsettingAnaProcessor" << std::endl;
    try
    {
        debug_           = parameters.getInteger("debug");
        anaName_         = parameters.getString("anaName");

        vtxColl_     = parameters.getString("vtxColl");

        fspCollRoot_    = parameters.getString("fspCollRoot");
        histCfgFilenameParticle_ = parameters.getString("histCfgParticle");
    }
    catch (std::runtime_error& error)
    {
        std::cout << error.what() << std::endl;
    }

}

void BeamRotationTargetOffsettingAnaProcessor::initialize(TTree* tree) {
    tree_= tree;

    // init TTree for track and vertex
    tree_->SetBranchAddress(vtxColl_.c_str()  , &vtxs_    , &bvtxs_    );

    // init histos for final-state particle
    histosParticle = new RecoParticleAnaHistos("recoParticleAna");
    histosParticle->loadHistoConfig(histCfgFilenameParticle_);
    histosParticle->DefineHistos();

    // init TTree for final-state particle
    tree_->SetBranchAddress(fspCollRoot_.c_str() , &fsps_, &bfsps_);

}

bool BeamRotationTargetOffsettingAnaProcessor::process(IEvent* ievent) {


    histosParticle->FillFSPs(fsps_);

    return true;
}

void BeamRotationTargetOffsettingAnaProcessor::finalize() {

    histosParticle->saveHistos(outF_, histosParticle->getName());
    delete histosParticle;
    histosParticle = nullptr;
}

DECLARE_PROCESSOR(BeamRotationTargetOffsettingAnaProcessor);
