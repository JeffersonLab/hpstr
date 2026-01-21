#include "TrackSmearingTool.h"
#include "TFile.h"
#include "TH1D.h"

#include <stdexcept>

// Helper function to check file extension
static bool endsWith(const std::string& str, const std::string& suffix) {
  if (suffix.size() > str.size()) return false;
  return str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

TrackSmearingTool::TrackSmearingTool(const std::string& smearingfile,
                                     const bool relSmearing,
                                     const int seed,
                                     const std::string& tracks){

  // Auto-detect file type based on extension
  if (endsWith(smearingfile, ".json")) {
    // JSON config file with top/bot smearing values
    std::ifstream cfgFile(smearingfile);
    if (!cfgFile)
      throw std::invalid_argument("Smearing JSON config file not found: " + smearingfile);

    json cfg;
    cfgFile >> cfg;

    pSmearingValueTop_ = cfg["pSmearing"]["top"];
    pSmearingValueBot_ = cfg["pSmearing"]["bot"];
    z0SmearingValueTop_ = cfg["z0Smearing"]["top"];
    z0SmearingValueBot_ = cfg["z0Smearing"]["bot"];
    relSmearing_ = cfg.value("relSmearing", false);
    useFixedSmearing_ = true;
    useSeparateTopBot_ = true;

    if (debug_) {
      std::cout<<"Using JSON smearing config: "<<smearingfile<<std::endl;
      std::cout<<"  pSmearing top: "<<pSmearingValueTop_<<" bot: "<<pSmearingValueBot_<<std::endl;
      std::cout<<"  z0Smearing top: "<<z0SmearingValueTop_<<" bot: "<<z0SmearingValueBot_<<std::endl;
      std::cout<<"  relSmearing: "<<relSmearing_<<std::endl;
    }

  } else {
    // ROOT file with smearing histograms
    relSmearing_ = relSmearing;
    std::string hsuffix = relSmearing_ ? "_rel" : "";
    smearingfile_ = std::make_shared<TFile>(smearingfile.c_str());

    if (!smearingfile_)
      throw std::invalid_argument("Provided input smearing file does not exist");

    //cache the smearing histograms
    smearing_histo_top_ = (TH1D*) smearingfile_->Get((tracks+"_p_vs_nHits_top_hh_smearing"+hsuffix).c_str());
    smearing_histo_bot_ = (TH1D*) smearingfile_->Get((tracks+"_p_vs_nHits_bot_hh_smearing"+hsuffix).c_str());

    if (!smearing_histo_top_ || !smearing_histo_bot_)
      throw std::invalid_argument("Top and Bottom smearing histograms not found in smearing file");

   }

  //setup random engine
  if (debug_)
    std::cout<<"Setting up random engine with seed "<<seed<<std::endl;
  generator_ = std::make_shared<std::default_random_engine>(seed);

  normal_ = std::make_shared<std::normal_distribution<double>>(0.,1.);

}

TrackSmearingTool::TrackSmearingTool(const double pSmearingValue,
                                     const double z0SmearingValue,
                                     const bool relSmearing,
                                     const int seed) {

  relSmearing_ = relSmearing;
  pSmearingValue_ = pSmearingValue;
  z0SmearingValue_ = z0SmearingValue;
  useFixedSmearing_ = true;

  //setup random engine
  if (debug_)
    std::cout<<"Setting up random engine with seed "<<seed<<std::endl;
  generator_ = std::make_shared<std::default_random_engine>(seed);

  normal_ = std::make_shared<std::normal_distribution<double>>(0.,1.);

  if (debug_) {
    std::cout<<"Using fixed p smearing value: "<<pSmearingValue_<<std::endl;
    std::cout<<"Using fixed z0 smearing value: "<<z0SmearingValue_<<std::endl;
  }

}

double TrackSmearingTool::smearTrackP(const double p) {

  double rel_smear = (*normal_)(*generator_);
  double sp = rel_smear * pSmearingValue_;

  double psmear = 0.;

  if (relSmearing_)
    psmear = p + sp*p;
  else
    psmear = p + sp;

  if (debug_) {
    std::cout<<"p: "<<p<<" deltaP="<<sp<<" p'="<<psmear<<std::endl;
  }

  return psmear;

}

double TrackSmearingTool::smearTrackP(const Track& track) {

  double p = track.getP();

  // If using fixed smearing with separate top/bot values
  if (useFixedSmearing_ && useSeparateTopBot_) {
    bool isTop = track.getTanLambda() > 0.;
    double smearingValue = isTop ? pSmearingValueTop_ : pSmearingValueBot_;

    double rel_smear = (*normal_)(*generator_);
    double sp = rel_smear * smearingValue;

    double psmear = 0.;
    if (relSmearing_)
      psmear = p + sp * p;
    else
      psmear = p + sp;

    if (debug_) {
      std::cout<<"Track isTop: "<<isTop<<" p: "<<p<<" deltaP="<<sp<<" p'="<<psmear<<std::endl;
    }

    return psmear;
  }

  // If using fixed smearing (single value), delegate to the simpler function
  if (useFixedSmearing_) {
    return smearTrackP(p);
  }

  double nhits = track.getTrackerHitCount();
  bool   isTop = track.getTanLambda() > 0. ? true : false;
  int    binN  = smearing_histo_top_->FindBin(nhits);
  
  if (debug_)
    std::cout<<"Track nhits="<<nhits<<" bin="<<binN<<std::endl;
  
  if (binN < 1) {
    if (debug_)
      std::cout<<"Track nhits="<<nhits<<" bin="<<binN<<" rounding to bin=1"<< std::endl;
    binN=1;
  } else if (binN > smearing_histo_top_->GetXaxis()->GetNbins()) {
    throw std::invalid_argument("Bin not found in smearing histogram");
  }
  
  double rel_smear = (*normal_)(*generator_);
  double  sp = 0.;
  
  if (isTop) 
    sp = rel_smear * smearing_histo_top_->GetBinContent(binN);
  else
    sp = rel_smear * smearing_histo_bot_->GetBinContent(binN);
  
  double psmear = 0.;

  if (relSmearing_)
    psmear = p + sp*p;
  else
    psmear = p + sp;
  
  
  if (debug_) {
    std::cout<<"Track isTop: "<<isTop<<" nHits: "<<nhits<<" p: "<<p<<" deltaP=" << sp<<" p'="<<psmear<<std::endl;
  }
  
  return psmear;
  
}

double TrackSmearingTool::updateWithSmearP(Track& trk) {
  double smeared_magnitude = smearTrackP(trk);
  // updated momentum by scaling each coordinate by smeared/unsmeared
  // this takes the direction of the unsmeared momentum and applies
  // the smeared magnitude
  std::vector<double> momentum = trk.getMomentum();
  double unsmeared_magnitude = trk.getP();
  for (double& coordinate : momentum)
    coordinate *= (smeared_magnitude/unsmeared_magnitude);
  trk.setMomentum(momentum);
  return (smeared_magnitude/unsmeared_magnitude);
}

void TrackSmearingTool::updateVertexWithSmearP(Vertex* vtx, double ele_smear_factor, double pos_smear_factor) {
    TVector3 p1_corr, p2_corr;
    double m_corr;

    double p1_uncorr_p = vtx->getP1().Mag();
    double p2_uncorr_p = vtx->getP2().Mag();

    p1_corr.SetX(vtx->getP1X()*ele_smear_factor);
    p1_corr.SetY(vtx->getP1Y()*ele_smear_factor);
    p1_corr.SetZ(vtx->getP1Z()*ele_smear_factor);

    p2_corr.SetX(vtx->getP2X()*pos_smear_factor);
    p2_corr.SetY(vtx->getP2Y()*pos_smear_factor);
    p2_corr.SetZ(vtx->getP2Z()*pos_smear_factor);
    // smear invariant mass as if it was a Moller (i.e. by sqrt(ele_smear*pos_smear))
    // using the corrected momenta directly to ensure correctness
    m_corr = vtx->getInvMass() * sqrt((p1_corr.Mag()/p1_uncorr_p)*(p2_corr.Mag()/p2_uncorr_p));
    vtx->setVtxParameters(p1_corr, p2_corr, m_corr);
}

double TrackSmearingTool::smearTrackZ0(const double z0) {

  double rel_smear = (*normal_)(*generator_);
  double sz0 = rel_smear * z0SmearingValue_;

  double z0smear = 0.;

  if (relSmearing_)
    z0smear = z0 + sz0*z0;
  else
    z0smear = z0 + sz0;

  if (debug_) {
    std::cout<<"z0: "<<z0<<" deltaZ0="<<sz0<<" z0'="<<z0smear<<std::endl;
  }

  return z0smear;

}

double TrackSmearingTool::smearTrackZ0(const Track& track) {

  double z0 = track.getZ0();

  // If using fixed smearing with separate top/bot values
  if (useFixedSmearing_ && useSeparateTopBot_) {
    bool isTop = track.getTanLambda() > 0.;
    double smearingValue = isTop ? z0SmearingValueTop_ : z0SmearingValueBot_;

    double rel_smear = (*normal_)(*generator_);
    double sz0 = rel_smear * smearingValue;

    double z0smear = 0.;
    if (relSmearing_)
      z0smear = z0 + sz0 * z0;
    else
      z0smear = z0 + sz0;

    if (debug_) {
      std::cout<<"Track isTop: "<<isTop<<" z0: "<<z0<<" deltaZ0="<<sz0<<" z0'="<<z0smear<<std::endl;
    }

    return z0smear;
  }

  return smearTrackZ0(z0);

}

void TrackSmearingTool::updateWithSmearZ0(Track& trk) {
  double smeared_z0 = smearTrackZ0(trk);
  trk.setZ0(smeared_z0);
}
