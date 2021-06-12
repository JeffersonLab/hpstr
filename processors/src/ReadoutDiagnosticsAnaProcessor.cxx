/**
 *@file ReadoutDiagnosticsAnaProcessor.cxx
 *@author Tongtong, UNH
 */

#include "ReadoutDiagnosticsAnaProcessor.h"

#include <iostream>

ReadoutDiagnosticsAnaProcessor::ReadoutDiagnosticsAnaProcessor(const std::string& name, Process& process) : Processor(name,process) {

}

//TODO Check this destructor

ReadoutDiagnosticsAnaProcessor::~ReadoutDiagnosticsAnaProcessor(){}

void ReadoutDiagnosticsAnaProcessor::configure(const ParameterSet& parameters) {
    std::cout << "Configuring ReadoutDiagnosticsAnaProcessor" <<std::endl;
    try
    {
        debug_           = parameters.getInteger("debug");
        anaName_         = parameters.getString("anaName");
        histCfgFilename_      = parameters.getString("histCfg",histCfgFilename_);
        ecalClusColl_    = parameters.getString("ecalClusColl");
        beamE_  = parameters.getDouble("beamE",beamE_);
    }
    catch (std::runtime_error& error)
    {
        std::cout << error.what() << std::endl;
    }

}

void ReadoutDiagnosticsAnaProcessor::initialize(TTree* tree) {
    tree_= tree;
    // init histos
    histos = new ReadoutDiagnosticsAnaHistos(anaName_.c_str());
    histos->loadHistoConfig(histCfgFilename_);
    histos->DefineHistos();

    // init TTree
    tree_->SetBranchAddress(ecalClusColl_.c_str() , &ecalClusters_, &becalClusters_);

}

bool ReadoutDiagnosticsAnaProcessor::process(IEvent* ievent) {
    double weight = 1.;

    histos->FillEcalClusters(ecalClusters_);

    return true;
}

void ReadoutDiagnosticsAnaProcessor::finalize() {


    histos->saveHistos(outF_, anaName_.c_str());
    delete histos;
    histos = nullptr;

}

DECLARE_PROCESSOR(ReadoutDiagnosticsAnaProcessor);
