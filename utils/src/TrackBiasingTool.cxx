#include "TrackBiasingTool.h"
#include "TFile.h"
#include "TH1D.h"

#include <stdexcept>

TrackBiasingTool::TrackBiasingTool(const std::string& biasingfile,
				   const std::string& tracks) {



  biasingfile_ = std::make_shared<TFile>(biasingfile.c_str());

  if (!biasingfile_)
    throw std::invalid_argument("Provided input biasing file doesn't exists");
  
  eop_h_top_  = (TH1D*) biasingfile_->Get((tracks+"_eop_vs_charge_top").c_str());
  eop_h_bot_  = (TH1D*) biasingfile_->Get((tracks+"_eop_vs_charge_bot").c_str());

  if (!eop_h_top_ || !eop_h_bot_)
    throw std::invalid_argument("Top and Bottom biasing histograms not found in smearing file");

}


Track TrackBiasingTool::biasTrack(const Track& track) {
  
  Track trk;
  
  return trk;
}
