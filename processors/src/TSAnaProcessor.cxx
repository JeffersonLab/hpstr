/**
 *@file TSAnaProcessor.cxx
 *@author Tongtong, UNH
 */

#include "TSAnaProcessor.h"

#include <iostream>

TSAnaProcessor::TSAnaProcessor(const std::string& name, Process& process) : Processor(name,process) {

}

//TODO Check this destructor

TSAnaProcessor::~TSAnaProcessor(){}

void TSAnaProcessor::configure(const ParameterSet& parameters) {
    std::cout << "Configuring VertexAnaProcessor" <<std::endl;
    try
    {
        debug_           = parameters.getInteger("debug");
        anaName_         = parameters.getString("anaName");
        histCfgFilename_      = parameters.getString("histCfg",histCfgFilename_);
        beamE_  = parameters.getDouble("beamE",beamE_);
    }
    catch (std::runtime_error& error)
    {
        std::cout << error.what() << std::endl;
    }

}

void TSAnaProcessor::initialize(TTree* tree) {
    tree_= tree;
    // init histos
    histos = new TSAnaHistos(anaName_.c_str());
    histos->loadHistoConfig(histCfgFilename_);
    histos->DefineHistos();

    // init TTree
    tree_->SetBranchAddress(tsColl_.c_str(), &tsData_ , &btsData_);
}

bool TSAnaProcessor::process(IEvent* ievent) {
    histos->FillTSData(tsData_);

    return true;
}

void TSAnaProcessor::finalize() {


    histos->saveHistos(outF_, anaName_.c_str());
    delete histos;
    histos = nullptr;

}

DECLARE_PROCESSOR(TSAnaProcessor);
