/**
 * @file VtxHistoProcessor.cxx
 * @brief Post processor of vtx histos
 * @author PF, SLAC National Accelerator Laboratory
 */

#include "VtxHistoProcessor.h"

VtxHistoProcessor::VtxHistoProcessor(const std::string& name, Process& process)
    : Processor(name, process) { 
    }

VtxHistoProcessor::~VtxHistoProcessor() { 
}

void VtxHistoProcessor::configure(const ParameterSet& parameters) {

    std::cout << "Configuring VtxHistoProcessor" << std::endl;
    try
    {
        debug_          = parameters.getInteger("debug");
    }
    catch (std::runtime_error& error)
    {
        std::cout << error.what() << std::endl;
    }


}

void VtxHistoProcessor::initialize(std::string inFilename, std::string outFilename) {
    // Init Files
    inF_ = new TFile(inFilename.c_str());
    
}

bool VtxHistoProcessor::process() {
    
    return true;
}

void VtxHistoProcessor::finalize() { 
    inF_->Close();
    delete inF_;
}

DECLARE_PROCESSOR(VtxHistoProcessor); 
