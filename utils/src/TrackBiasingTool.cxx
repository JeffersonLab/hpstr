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

double TrackBiasingTool::biasTrackP(const Track& trk) {

  double p = trk.getP();
  double isTop = trk.getTanLambda() > 0. ? true : false;
  double q = trk.getCharge();
    
  TH1D* bias_histo_ = isTop ?  eop_h_top_ : eop_h_bot_;
    
  int binN = bias_histo_->GetXaxis()->FindBin(q);
  if (debug_)
    std::cout<<"Track charge="<<q<<" bin="<<binN<<std::endl;
  
  double eop_bias = bias_histo_->GetBinContent(binN);

  double pcorr = p * eop_bias;
  
  if (debug_)
    std::cout<<"Original p = " << p <<" corrected p="<< pcorr <<std::endl;
  
  return pcorr;
}

void TrackBiasingTool::updateWithBiasP(Track& trk) {

  double biased_p = biasTrackP(trk);
  
  std::vector<double> momentum = trk.getMomentum();
  double unbiasedp = trk.getP();
  
  for (double& coordinate : momentum)
    coordinate *= (biased_p / unbiasedp);

  trk.setMomentum(momentum);
  
}

// This will recompute the track momentum from curvature and store it in the track
void TrackBiasingTool::updateWithBiasP(Track& trk, double scaleFactor) {
  
  std::vector<double> momentum = trk.getMomentum();
  
  for (double& coordinate : momentum)
    coordinate *= scaleFactor ;
  
  trk.setMomentum(momentum);    
  
}
