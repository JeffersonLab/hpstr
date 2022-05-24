/**
 * @author Tongtong Cao, UNH
 */
#include "RecoHodoAnaProcessor.h"
#include <iostream>

RecoHodoAnaProcessor::RecoHodoAnaProcessor(const std::string& name, Process& process) : Processor(name,process){}
//TODO CHECK THIS DESTRUCTOR
RecoHodoAnaProcessor::~RecoHodoAnaProcessor(){}


void RecoHodoAnaProcessor::configure(const ParameterSet& parameters) {
    std::cout << "Configuring RecoHodoAnaProcessor" << std::endl;
    try
    {
        debug_           = parameters.getInteger("debug");
        anaName_         = parameters.getString("anaName");
        hodoHitColl_     = parameters.getString("hodoHitColl");
        hodoClusColl_    = parameters.getString("hodoClusColl");
        histCfgFilename_ = parameters.getString("histCfg");
    }
    catch (std::runtime_error& error)
    {
        std::cout << error.what() << std::endl;
    }

}

void RecoHodoAnaProcessor::initialize(TTree* tree) {
    tree_= tree;
    // init histos
    histos = new RecoHodoAnaHistos(anaName_.c_str());
    histos->loadHistoConfig(histCfgFilename_);
    histos->DefineHistos();

    // init TTree
    tree_->SetBranchAddress(hodoHitColl_.c_str()  , &hodoHits_    , &bhodoHits_    );
    tree_->SetBranchAddress(hodoClusColl_.c_str() , &hodoClusters_, &bhodoClusters_);

}

bool RecoHodoAnaProcessor::process(IEvent* ievent) {

    histos->FillHodoHits(hodoHits_);
    histos->FillHodoClusters(hodoClusters_);

    return true;
}

void RecoHodoAnaProcessor::finalize() {

    histos->saveHistos(outF_, anaName_.c_str());
    delete histos;
    histos = nullptr;
}

DECLARE_PROCESSOR(RecoHodoAnaProcessor);
