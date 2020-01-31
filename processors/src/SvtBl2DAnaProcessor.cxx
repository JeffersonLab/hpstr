#include "SvtBl2DAnaProcessor.h"
#include "TBranch.h"

SvtBl2DAnaProcessor::SvtBl2DAnaProcessor(const std::string& name, Process& process) : Processor(name,process){
}

SvtBl2DAnaProcessor::~SvtBl2DAnaProcessor(){ 
}


void SvtBl2DAnaProcessor::configure(const ParameterSet& parameters) {
    std::cout << "Configuring SvtBl2DAnaProcessor" << std::endl;
    try
    {
        debug_           = parameters.getInteger("debug");
        rmBl_            = parameters.getInteger("removeBaseline");
        blFitFile_       = parameters.getString("baselineFits");
        runNum_          = parameters.getInteger("runNumber");
        rawSvtHitsColl_  = parameters.getString("rawSvtHitsColl");
        histCfgFilename_  = parameters.getString("histCfg");
    }
    catch (std::runtime_error& error)
    {
        std::cout << error.what() << std::endl;
    }
}

void SvtBl2DAnaProcessor::initialize(TTree* tree) {
    std::cout << "[SvtBl2DAnaProcessor] Initializing" << std::endl;
    svtCondHistos = new Svt2DBlHistos("raw_hits");
    std::cout << "[SvtBl2DAnaProcessor] Load JSON" << std::endl;
    svtCondHistos->loadHistoConfig(histCfgFilename_);
    if (debug_ > 0) std::cout << "[SvtBl2DAnaProcessor] Define 2DHistos" << std::endl;
    svtCondHistos->DefineHistos();
    if (debug_ > 0) std::cout << "[SvtBl2DAnaProcessor] Defined 2DHistos" << std::endl;

    tree_ = tree;
    tree_->SetBranchAddress(rawSvtHitsColl_.c_str(), &rawSvtHits_, &brawSvtHits_);
    if (debug_ > 0) std::cout << "[SvtBl2DAnaProcessor] TTree Initialized" << std::endl;

}

bool SvtBl2DAnaProcessor::process(IEvent* ievent) {
    
    svtCondHistos->FillHistograms(rawSvtHits_,1.);

    return true;
}

void SvtBl2DAnaProcessor::finalize() {
    std::cout << "[SvtBl2DAnaProcessor] Finalizing" << std::endl;

    for(std::vector<std::string>::iterator it = svtCondHistos->histos2dNamesfromJson.begin(); it != svtCondHistos->histos2dNamesfromJson.end(); ++it) {
        std::cout << *it << std::endl;
    }
    svtCondHistos->get2DHistoOccupancy(svtCondHistos->histos2dNamesfromJson);
    std::cout << "svtCondHistos get2dHistoOccupancy" << std::endl;
    svtCondHistos->saveHistos(outF_,"");
    delete svtCondHistos;
    svtCondHistos = nullptr;
}

DECLARE_PROCESSOR(SvtBl2DAnaProcessor);
