#include "TrackBiasingTool.h"
#include "TFile.h"
#include "TH1D.h"
#include "TVector3.h"

#include <stdexcept>
#include <fstream>

TrackBiasingTool::TrackBiasingTool(const std::string& biasingfile,
				   const std::string& tracks) {

  if (biasingfile.empty()) {
    // z0-calibration-only mode: no momentum biasing ROOT file
    eop_h_top_ = nullptr;
    eop_h_bot_ = nullptr;
    return;
  }

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
  
  TH1D* bias_histo_ = (tanL > 0.) ?  eop_h_top_ : eop_h_bot_;
  int binN = bias_histo_->GetXaxis()->FindBin(q);
  
  return bias_histo_->GetBinContent(binN);

}
  

double TrackBiasingTool::biasTrackP(const Track& trk) {

  double p = trk.getP();
  double q = trk.getCharge();
  
  TH1D* bias_histo_ = (trk.getTanLambda() > 0.) ?  eop_h_top_ : eop_h_bot_;
  
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

void TrackBiasingTool::loadZ0Calibration(const std::string& jsonFile) {
    std::ifstream ifs(jsonFile);
    if (!ifs.is_open())
        throw std::invalid_argument("TrackBiasingTool: cannot open z0 calibration file: " + jsonFile);
    json cfg;
    ifs >> cfg;

    auto loadParams = [](const json& node, const std::string& key) -> Z0CalibParams {
        Z0CalibParams p;
        if (node.contains(key)) {
            p.slope     = node[key].value("slope",     0.0);
            p.intercept = node[key].value("intercept", 0.0);
        }
        return p;
    };

    for (auto& [runStr, runCfg] : cfg.items()) {
        // "default" key is treated as run 0 (always selected by largest-key-<=run logic)
        int runKey = (runStr == "default") ? 0 : std::stoi(runStr);
        Z0CalibSet s;
        s.ele_top = loadParams(runCfg, "ele_top");
        s.ele_bot = loadParams(runCfg, "ele_bot");
        s.pos_top = loadParams(runCfg, "pos_top");
        s.pos_bot = loadParams(runCfg, "pos_bot");
        z0CalibByRun_[runKey] = s;
    }

    z0CalibLoaded_ = true;
    std::cout << "TrackBiasingTool: loaded z0 calibration from " << jsonFile
              << "  (" << z0CalibByRun_.size() << " run entries)" << std::endl;
}

void TrackBiasingTool::setRun(int run) {
    if (!z0CalibLoaded_ || run == z0CalibCurrentRun_) return;

    // Find largest key <= run
    int best = -1;
    for (auto& [key, set] : z0CalibByRun_) {
        if (key <= run) best = key;
    }

    if (best < 0) {
        std::cout << "TrackBiasingTool: WARNING no z0 calibration entry found for run "
                  << run << " (all keys > run), z0 calibration will be skipped." << std::endl;
        z0CalibLoaded_ = false;
        return;
    }

    z0CalibActive_    = z0CalibByRun_.at(best);
    z0CalibCurrentRun_ = run;

    if (debug_) {
        std::cout << "TrackBiasingTool::setRun(" << run << "): using calibration key=" << best << std::endl;
        std::cout << "  ele_top: slope=" << z0CalibActive_.ele_top.slope << "  intercept=" << z0CalibActive_.ele_top.intercept << std::endl;
        std::cout << "  ele_bot: slope=" << z0CalibActive_.ele_bot.slope << "  intercept=" << z0CalibActive_.ele_bot.intercept << std::endl;
        std::cout << "  pos_top: slope=" << z0CalibActive_.pos_top.slope << "  intercept=" << z0CalibActive_.pos_top.intercept << std::endl;
        std::cout << "  pos_bot: slope=" << z0CalibActive_.pos_bot.slope << "  intercept=" << z0CalibActive_.pos_bot.intercept << std::endl;
    }
}

void TrackBiasingTool::updateWithCalibrateZ0(Track& trk) {
    if (!z0CalibLoaded_) return;

    bool isTop = trk.getTanLambda() > 0.0;
    bool isEle = trk.getCharge() < 0;

    const Z0CalibParams& p =
        isEle ? (isTop ? z0CalibActive_.ele_top : z0CalibActive_.ele_bot)
              : (isTop ? z0CalibActive_.pos_top : z0CalibActive_.pos_bot);

    double tanL  = trk.getTanLambda();
    double z0    = trk.getZ0();
    double delta = p.slope * tanL + p.intercept;
    double z0new = z0 - delta;

    if (debug_) {
        std::cout << "TrackBiasingTool::updateWithCalibrateZ0:"
                  << " type=" << (isEle ? "ele" : "pos") << "_" << (isTop ? "top" : "bot")
                  << "  |tanL|=" << tanL
                  << "  slope=" << p.slope << "  intercept=" << p.intercept
                  << "  delta=" << delta
                  << "  z0=" << z0 << " -> " << z0new << std::endl;
    }

    trk.setZ0(z0new);
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
						

