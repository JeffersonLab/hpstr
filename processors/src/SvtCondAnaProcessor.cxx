#include "SvtCondAnaProcessor.h"
#include "TBranch.h"

SvtCondAnaProcessor::SvtCondAnaProcessor(const std::string& name, Process& process) : Processor(name,process){
}

SvtCondAnaProcessor::~SvtCondAnaProcessor(){ 
}


void SvtCondAnaProcessor::configure(const ParameterSet& parameters) {
    std::cout << "Configure" << std::endl;
}

void SvtCondAnaProcessor::initialize(TTree* tree) {
    std::cout << "Initializing" << std::endl;
    svtCondHistos = new SvtCondHistos("raw_hits");
    svtCondHistos->Define2DHistos();
    std::cout << "Define2DHistos" << std::endl;

    tree_= tree;
    std::cout << "tree defined" << std::endl;
    tree_->SetBranchAddress(Collections::RAW_SVT_HITS,&rawSvtHits_,&brawSvtHits_);
    std::cout << "Branch Address Set" << std::endl;

}

bool SvtCondAnaProcessor::process(IEvent* ievent) {

    int nlines = rawSvtHits_->GetEntries();
    if(Event_number%1000 == 0) std::cout << "Event: " << Event_number << " Number of Entries is: " << nlines << std::endl;
    for (int i=0; i<nlines; i++) {
        RawSvtHit* rawSvtHit = (RawSvtHit*)rawSvtHits_->At(i);
        svtCondHistos->FillHistograms(rawSvtHit, 1.); 
    }
    Event_number++;  
    return true;
}

void SvtCondAnaProcessor::finalize() {

    svtCondHistos->saveHistos(outF_,"");
    delete svtCondHistos;
    svtCondHistos = nullptr;
}

DECLARE_PROCESSOR(SvtCondAnaProcessor);
