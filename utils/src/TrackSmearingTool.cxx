#include "TrackSmearingTool.h"
#include "TFile.h"
#include "TH1D.h"

#include <stdexcept>

TrackSmearingTool::TrackSmearingTool(const std::string& smearingfile,
                                     const int seed,
                                     const std::string& tracks){
  smearingfile_ = std::make_shared<TFile>(smearingfile.c_str());
  if (!smearingfile_)
    throw std::invalid_argument( "Provided input smearing file does not exists");
  
  //cache the smearing terms
  smearing_histo_top_ = (TH1D*) smearingfile_->Get((tracks+"_p_vs_nHits_hh_smearing").c_str());
  smearing_histo_bot_ = (TH1D*) smearingfile_->Get((tracks+"_p_vs_nHits_hh_smearing").c_str());

  if (!smearing_histo_top_ || !smearing_histo_bot_) 
    throw std::invalid_argument("Top and Bottom smearing histograms not found in smearing file");
  
  //setup random engine
  if (debug_) 
    std::cout<<"Setting up random engine with seed "<<seed<<std::endl;
  generator_ = std::make_shared<std::default_random_engine>(seed);
  
  for (unsigned int ibin = 0; ibin < smearing_histo_top_->GetXaxis()->GetNbins();ibin++) {
    double sft = smearing_histo_top_->GetBinContent(ibin+1);
    double sfb = smearing_histo_bot_->GetBinContent(ibin+1);
    if (debug_) {
      std::cout<<"Setting up top normal distribution with sigma " <<sft<<" from bin"<< ibin<<std::endl;
      std::cout<<"Setting up bot normal distribution with sigma " <<sfb<<" from bin"<< ibin<<std::endl;
    }
    
    normals_top_.push_back(std::make_shared<std::normal_distribution<float>>(0.,sft));
    normals_bot_.push_back(std::make_shared<std::normal_distribution<float>>(0.,sfb));
    
  }
  
}

double TrackSmearingTool::smearTrackP(const Track& track) {

  double p     = track.getP();
  double nhits = track.getTrackerHitCount();
  bool isTop   = track.getTanLambda() > 0. ? true : false;
  int binN     = smearing_histo_top_->FindBin(nhits);

  if (debug_)
    std::cout<<"Track nhits="<<nhits<<" bin="<<binN<<std::endl;
  
  if (binN < 1 || binN > smearing_histo_top_->GetXaxis()->GetNbins()) {
    throw std::invalid_argument("Bin not found in smearing histogram");
  }
  float sp = 0.;
  if (isTop) {
    sp = (*(normals_top_.at(binN-1)))(*generator_);
    
  } else {
    sp = (*(normals_bot_.at(binN-1)))(*generator_); 
  }
  
  double psmear = p + sp;
  
  if (debug_) {
    std::cout<<"Track isTop: "<<isTop<<" nHits: "<<nhits<<" p: "<<p<<" deltaP=" << sp<<" p'="<<psmear<<std::endl;
  }
  
  return psmear;
  
}



