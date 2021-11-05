/**
 * @author Tongtong Cao, UNH
 */
#include "RecoEcalAnaProcessor.h"
#include <iostream>

RecoEcalAnaProcessor::RecoEcalAnaProcessor(const std::string& name, Process& process) : Processor(name,process){}
//TODO CHECK THIS DESTRUCTOR
RecoEcalAnaProcessor::~RecoEcalAnaProcessor(){}


void RecoEcalAnaProcessor::configure(const ParameterSet& parameters) {
    std::cout << "Configuring RecoEcalAnaProcessor" << std::endl;
    try
    {
        debug_           = parameters.getInteger("debug");
        anaName_         = parameters.getString("anaName");
        ecalHitColl_     = parameters.getString("ecalHitColl");
        ecalClusColl_    = parameters.getString("ecalClusColl");
        histCfgFilename_ = parameters.getString("histCfg");
    }
    catch (std::runtime_error& error)
    {
        std::cout << error.what() << std::endl;
    }

}

void RecoEcalAnaProcessor::initialize(TTree* tree) {
    tree_= tree;
    // init histos
    histos = new RecoEcalAnaHistos(anaName_.c_str());
    histos->loadHistoConfig(histCfgFilename_);
    histos->DefineHistos();

    // init TTree
    tree_->SetBranchAddress(ecalHitColl_.c_str()  , &ecalHits_    , &becalHits_    );
    tree_->SetBranchAddress(ecalClusColl_.c_str() , &ecalClusters_, &becalClusters_);

}

bool RecoEcalAnaProcessor::process(IEvent* ievent) {

    histos->FillEcalHits(ecalHits_);
    histos->FillEcalClusters(ecalClusters_);

    return true;
}

void RecoEcalAnaProcessor::finalize() {

    histos->saveHistos(outF_, anaName_.c_str());
    delete histos;
    histos = nullptr;
}

DECLARE_PROCESSOR(RecoEcalAnaProcessor);
