/**
 * @file Apv25RoXtalkAnaProcessor.cxx
 * @brief AnaProcessor used fill histograms to compare simulations
 * @author Cameron Bravo, SLAC National Accelerator Laboratory
 */     
#include "Apv25RoXtalkAnaProcessor.h"
#include <iostream>

Apv25RoXtalkAnaProcessor::Apv25RoXtalkAnaProcessor(const std::string& name, Process& process) : Processor(name,process){}
//TODO CHECK THIS DESTRUCTOR
Apv25RoXtalkAnaProcessor::~Apv25RoXtalkAnaProcessor(){}


void Apv25RoXtalkAnaProcessor::configure(const ParameterSet& parameters) {
    std::cout << "Configuring Apv25RoXtalkAnaProcessor" << std::endl;
    try
    {
        debug_           = parameters.getInteger("debug");
        anaName_         = parameters.getString("anaName");
        rawHitColl_      = parameters.getString("rawHitColl");
        initSyncPhase_   = parameters.getInteger("debug");
        histCfgFilename_ = parameters.getString("histCfg");
    }
    catch (std::runtime_error& error)
    {
        std::cout << error.what() << std::endl;
    }

}

void Apv25RoXtalkAnaProcessor::initialize(TTree* tree) {
    tree_= tree;

    // init TTree
    tree_->SetBranchAddress(rawHitColl_.c_str() , &rawHits_ , &brawHits_ );
    tree_->SetBranchAddress("EventHeader"       , &evth_    , &bevth_    );

}

bool Apv25RoXtalkAnaProcessor::process(IEvent* ievent) {

    std::cout << "[Apv25RoXtalkAnaProcessor] Event Time: " << evth_->getEventTime() << std::endl;
    std::cout << "[Apv25RoXtalkAnaProcessor] Event Hit Multi: " << rawHits_->size() << std::endl;

    return true;
}

void Apv25RoXtalkAnaProcessor::finalize() {
    
    std::cout << "[Apv25RoXtalkAnaProcessor] Finalizing" << std::endl;

}

DECLARE_PROCESSOR(Apv25RoXtalkAnaProcessor);
