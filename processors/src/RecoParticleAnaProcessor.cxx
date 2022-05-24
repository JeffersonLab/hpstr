/**
 * @author Tongtong Cao, UNH
 */
#include "RecoParticleAnaProcessor.h"
#include <iostream>

RecoParticleAnaProcessor::RecoParticleAnaProcessor(const std::string& name, Process& process) : Processor(name,process){}
//TODO CHECK THIS DESTRUCTOR
RecoParticleAnaProcessor::~RecoParticleAnaProcessor(){}


void RecoParticleAnaProcessor::configure(const ParameterSet& parameters) {
    std::cout << "Configuring RecoParticleAnaProcessor" << std::endl;
    try
    {
        debug_           = parameters.getInteger("debug");
        anaName_         = parameters.getString("anaName");
        fspCollRoot_    = parameters.getString("fspCollRoot");
        histCfgFilename_ = parameters.getString("histCfg");
    }
    catch (std::runtime_error& error)
    {
        std::cout << error.what() << std::endl;
    }

}

void RecoParticleAnaProcessor::initialize(TTree* tree) {
    tree_= tree;
    // init histos
    histos = new RecoParticleAnaHistos(anaName_.c_str());
    histos->loadHistoConfig(histCfgFilename_);
    histos->DefineHistos();

    // init TTree
    tree_->SetBranchAddress(fspCollRoot_.c_str() , &fsps_, &bfsps_);

}

bool RecoParticleAnaProcessor::process(IEvent* ievent) {

    double weight = 1.;

    histos->FillFSPs(fsps_);



    return true;
}

void RecoParticleAnaProcessor::finalize() {

    histos->saveHistos(outF_, anaName_.c_str());
    delete histos;
    histos = nullptr;
}

DECLARE_PROCESSOR(RecoParticleAnaProcessor);
