#include "RawSvtHitMCAnaProcessor.h"
#include "TBranch.h"

RawSvtHitMCAnaProcessor::RawSvtHitMCAnaProcessor(const std::string& name, Process& process) : Processor(name,process){
}

RawSvtHitMCAnaProcessor::~RawSvtHitMCAnaProcessor(){ 
}


void RawSvtHitMCAnaProcessor::configure(const ParameterSet& parameters) {
    std::cout << "Configuring RawSvtHitMCAnaProcessor" << std::endl;
    try
    {
        debug_           = parameters.getInteger("debug");
        rawSvtHitsColl_  = parameters.getString("rawSvtHitsColl");
        histCfgFilename_  = parameters.getString("histCfg");
    }
    catch (std::runtime_error& error)
    {
        std::cout << error.what() << std::endl;
    }
}

void RawSvtHitMCAnaProcessor::initialize(TTree* tree) {
    std::cout << "[RawSvtHitMCAnaProcessor] Initializing" << std::endl;
    rawSvtHitHistos_ = new RawSvtHitMCHistos("rawMC");
    std::cout << "[RawSvtHitMCAnaProcessor] Load JSON" << std::endl;
    rawSvtHitHistos_->loadHistoConfig(histCfgFilename_);
    if (debug_ > 0) std::cout << "[RawSvtHitMCAnaProcessor] Define 2DHistos" << std::endl;
    rawSvtHitHistos_->DefineHistos();
    if (debug_ > 0) std::cout << "[RawSvtHitMCAnaProcessor] Defined 2DHistos" << std::endl;

    tree_ = tree;
    tree_->SetBranchAddress(rawSvtHitsColl_.c_str(), &rawSvtHits_, &brawSvtHits_);
    if (debug_ > 0) std::cout << "[RawSvtHitMCAnaProcessor] TTree Initialized" << std::endl;

}

bool RawSvtHitMCAnaProcessor::process(IEvent* ievent) {
    
    rawSvtHitHistos_->FillHistograms(rawSvtHits_,1.);
    return true;
}

void RawSvtHitMCAnaProcessor::finalize() {

    rawSvtHitHistos_->saveHistos(outF_,"");
    delete rawSvtHitHistos_;
    rawSvtHitHistos_ = nullptr;
}

DECLARE_PROCESSOR(RawSvtHitMCAnaProcessor);
