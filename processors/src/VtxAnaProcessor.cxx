/**
 *@file VtxAnaProcessor.cxx
 *@author Tongtong, UNH
 */

#include "VtxAnaProcessor.h"

#include <iostream>

VtxAnaProcessor::VtxAnaProcessor(const std::string& name, Process& process) : Processor(name,process) {

}

//TODO Check this destructor

VtxAnaProcessor::~VtxAnaProcessor(){}

void VtxAnaProcessor::configure(const ParameterSet& parameters) {
    std::cout << "Configuring VertexAnaProcessor" <<std::endl;
    try
    {
        debug_           = parameters.getInteger("debug");
        anaName_         = parameters.getString("anaName");
        histCfgFilename_      = parameters.getString("histCfg",histCfgFilename_);
        vtxColl_ = parameters.getString("vtxColl",vtxColl_);
        tcvtxColl_ = parameters.getString("tcvtxColl",tcvtxColl_);
        beamE_  = parameters.getDouble("beamE",beamE_);
    }
    catch (std::runtime_error& error)
    {
        std::cout << error.what() << std::endl;
    }

}

void VtxAnaProcessor::initialize(TTree* tree) {
    tree_= tree;
    // init histos
    histos = new VtxAnaHistos(anaName_.c_str());
    histos->loadHistoConfig(histCfgFilename_);
    histos->DefineHistos();

    // init TTree
    tree_->SetBranchAddress(tsColl_.c_str(), &tsData_ , &btsData_);
    tree_->SetBranchAddress(vtxColl_.c_str(), &vtxs_ , &bvtxs_);
    tree_->SetBranchAddress(tcvtxColl_.c_str(), &tcvtxs_ , &btcvtxs_);
}

bool VtxAnaProcessor::process(IEvent* ievent) {
    histos->FillTSData(tsData_);

    if(tsData_->prescaled.Single_3_Top == true && tsData_->prescaled.Single_3_Bot == true){
    	std::cout << "Warning: Single3 trigger for both top and bot are registered." << std::endl;
    	std::cout << "Number of vertices: " << vtxs_->size() << std::endl;
    }

    if(tsData_->prescaled.Single_3_Top == true || tsData_->prescaled.Single_3_Bot == true){
    	histos->FillUnconstrainedV0s(vtxs_);
    }



    return true;
}

void VtxAnaProcessor::finalize() {


    histos->saveHistos(outF_, anaName_.c_str());
    delete histos;
    histos = nullptr;

}

DECLARE_PROCESSOR(VtxAnaProcessor);
