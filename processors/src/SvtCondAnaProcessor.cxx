#include "SvtCondAnaProcessor.h"
#include "TBranch.h"

SvtCondAnaProcessor::SvtCondAnaProcessor(const std::string& name, Process& process) : Processor(name,process){std::cout << "SvtCondAnaProcessor Constructor" << std::endl;
	std::cout << "name:" << name << std::endl;
}
//TODO CHECK THIS DESTRUCTOR
SvtCondAnaProcessor::~SvtCondAnaProcessor(){ std::cout << "SvtCondAnaProcessor Destructor" << std::endl;}


void SvtCondAnaProcessor::configure(const ParameterSet& parameters) {
}

void SvtCondAnaProcessor::initialize(TTree* tree) {
  std::cout << "SvtCondAnaProcessor has been initialized" << std::endl;
  svtCondHistos = new SvtCondHistos("hitOnTrack_2D");
  
//  if (!baselineFits_.empty() && !baselineRun_.empty()) {
  //  svtCondHistos->setBaselineFitsDir(baselineFits_);
    //if (!svtCondHistos->LoadBaselineHistos(baselineRun_))
      //std::cout<<"WARNING: baselines not loaded in Cluster on Track histos."<<std::endl;}
  
  svtCondHistos->Define1DHistos();
//  svtCondHistos->Define2DHistos();
  tree_= tree;
  //TODO Change this to Svt Raw blah blah
  tree_->SetBranchAddress(Collections::RAW_SVT_HITS,&rawSvtHits_,&brawSvtHits_);
  //TODO Change this.
  outF_ = new TFile("shazam.root","recreate");
  
}

bool SvtCondAnaProcessor::process(IEvent* ievent) {

 	int nlines = rawSvtHits_->GetEntriesFast();
	for (int i=0; i<nlines; i++) {
         RawSvtHit* rawSvtHit = (RawSvtHit*)rawSvtHits_->At(i);
	 svtCondHistos->FillHistograms(rawSvtHit, 1.); 
	 
}
  
   return true;
}

void SvtCondAnaProcessor::finalize() {

  svtCondHistos->saveHistos(outF_,"");
  outF_->Close();
  delete svtCondHistos;
  svtCondHistos = nullptr;
}

DECLARE_PROCESSOR(SvtCondAnaProcessor);
