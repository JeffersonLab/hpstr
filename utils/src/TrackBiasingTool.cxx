#include "TrackBiasingTool.h"
#include "TFile.h"
#include "TH1D.h"
#include "TVector3.h"

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


double TrackBiasingTool::getCorrection(const double& p,
				       const double tanL,
				       const int q) {

  bool isTop = tanL > 0. ? true : false;
  TH1D* bias_histo_ = isTop ?  eop_h_top_ : eop_h_bot_;
  int binN = bias_histo_->GetXaxis()->FindBin(q);
  
  return bias_histo_->GetBinContent(binN);

}
  

double TrackBiasingTool::biasTrackP(const Track& trk) {

  double p = trk.getP();
  bool isTop = trk.getTanLambda() > 0. ? true : false;
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

void TrackBiasingTool::updateVertexWithBiasP(Vertex* vtx) {

  //Correct the vertex
  double corr1 = getCorrection(vtx->getP1().Mag(), vtx->getP1Y(), -1); //ele
  double corr2 = getCorrection(vtx->getP2().Mag(), vtx->getP2Y(), 1); //pos


  TVector3 p1_corr, p2_corr;
  double m_corr;

  p1_corr.SetX(vtx->getP1X()*corr1);
  p1_corr.SetY(vtx->getP1Y()*corr1);
  p1_corr.SetZ(vtx->getP1Z()*corr1);
  
  p2_corr.SetX(vtx->getP2X()*corr2);
  p2_corr.SetY(vtx->getP2Y()*corr2);
  p2_corr.SetZ(vtx->getP2Z()*corr2);
  
  m_corr = vtx->getInvMass() * sqrt(corr1*corr2);
  
  vtx->setVtxParameters(p1_corr, p2_corr, m_corr);
  
}
						

