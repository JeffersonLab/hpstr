#include "TrackSmearingTool.h"
#include "TruthMatchingUtils.h"
#include "TFile.h"
#include "TH1D.h"

#include <stdexcept>
#include <cstdlib>
#include <map>

// Helper function to check file extension
static bool endsWith(const std::string& str, const std::string& suffix) {
  if (suffix.size() > str.size()) return false;
  return str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

TrackSmearingTool::TrackSmearingTool(const std::string& smearingfile,
                                     const bool relSmearingP,
                                     const bool relSmearingZ0,
                                     const int seed,
                                     const std::string& tracks,
                                     const double smearingFactor){

  smearingFactor_ = smearingFactor;

  // Auto-detect file type based on extension
  if (endsWith(smearingfile, ".json")) {
    // JSON config file with top/bot smearing values
    std::ifstream cfgFile(smearingfile);
    if (!cfgFile)
      throw std::invalid_argument("Smearing JSON config file not found: " + smearingfile);

    json cfg;
    cfgFile >> cfg;

    pSmearingValueTop_ = cfg["pSmearing"]["top"].get<double>() * smearingFactor_;
    pSmearingValueBot_ = cfg["pSmearing"]["bot"].get<double>() * smearingFactor_;
    z0SmearingValueTop_ = cfg["z0Smearing"]["top"].get<double>() * smearingFactor_;
    z0SmearingValueBot_ = cfg["z0Smearing"]["bot"].get<double>() * smearingFactor_;

    // Support separate relSmearing for p and z0, with backwards compatibility
    // If relSmearingP/relSmearingZ0 are present, use them; otherwise fall back to relSmearing
    if (cfg.contains("relSmearingP")) {
      relSmearingP_ = cfg["relSmearingP"].get<bool>();
    } else if (cfg.contains("relSmearing")) {
      relSmearingP_ = cfg["relSmearing"].get<bool>();
    } else {
      relSmearingP_ = false;
    }

    if (cfg.contains("relSmearingZ0")) {
      relSmearingZ0_ = cfg["relSmearingZ0"].get<bool>();
    } else if (cfg.contains("relSmearing")) {
      relSmearingZ0_ = cfg["relSmearing"].get<bool>();
    } else {
      relSmearingZ0_ = false;
    }

    // Parse omega smearing parameters (optional)
    if (cfg.contains("omegaSmearing")) {
      omegaSmearingValueTop_ = cfg["omegaSmearing"]["top"].get<double>() * smearingFactor_;
      omegaSmearingValueBot_ = cfg["omegaSmearing"]["bot"].get<double>() * smearingFactor_;
    }

    // Parse smearOmega flag (optional, default false)
    if (cfg.contains("smearOmega")) {
      smearOmega_ = cfg["smearOmega"].get<bool>();
    }

    useFixedSmearing_ = true;
    useSeparateTopBot_ = true;

    if (debug_) {
      std::cout<<"Using JSON smearing config: "<<smearingfile<<std::endl;
      std::cout<<"  smearingFactor: "<<smearingFactor_<<std::endl;
      std::cout<<"  pSmearing top: "<<pSmearingValueTop_<<" bot: "<<pSmearingValueBot_<<std::endl;
      std::cout<<"  z0Smearing top: "<<z0SmearingValueTop_<<" bot: "<<z0SmearingValueBot_<<std::endl;
      std::cout<<"  omegaSmearing top: "<<omegaSmearingValueTop_<<" bot: "<<omegaSmearingValueBot_<<std::endl;
      std::cout<<"  relSmearingP: "<<relSmearingP_<<" relSmearingZ0: "<<relSmearingZ0_<<std::endl;
      std::cout<<"  smearOmega: "<<smearOmega_<<std::endl;
    }

  } else {
    // ROOT file with smearing histograms
    relSmearingP_ = relSmearingP;
    relSmearingZ0_ = relSmearingZ0;
    std::string hsuffix = relSmearingP_ ? "_rel" : "";
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
                                     const bool relSmearingP,
                                     const bool relSmearingZ0,
                                     const int seed,
                                     const double smearingFactor) {

  smearingFactor_ = smearingFactor;
  relSmearingP_ = relSmearingP;
  relSmearingZ0_ = relSmearingZ0;
  pSmearingValue_ = pSmearingValue * smearingFactor_;
  z0SmearingValue_ = z0SmearingValue * smearingFactor_;
  useFixedSmearing_ = true;

  //setup random engine
  if (debug_)
    std::cout<<"Setting up random engine with seed "<<seed<<std::endl;
  generator_ = std::make_shared<std::default_random_engine>(seed);

  normal_ = std::make_shared<std::normal_distribution<double>>(0.,1.);

  if (debug_) {
    std::cout<<"Using smearing factor: "<<smearingFactor_<<std::endl;
    std::cout<<"Using fixed p smearing value: "<<pSmearingValue_<<std::endl;
    std::cout<<"Using fixed z0 smearing value: "<<z0SmearingValue_<<std::endl;
    std::cout<<"Using relSmearingP: "<<relSmearingP_<<" relSmearingZ0: "<<relSmearingZ0_<<std::endl;
  }

}

double TrackSmearingTool::smearTrackP(const double p) {

  double rel_smear = (*normal_)(*generator_);
  double sp = rel_smear * pSmearingValue_;

  double psmear = 0.;

  if (relSmearingP_)
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
    if (relSmearingP_)
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
    sp = rel_smear * smearing_histo_top_->GetBinContent(binN) * smearingFactor_;
  else
    sp = rel_smear * smearing_histo_bot_->GetBinContent(binN) * smearingFactor_;

  double psmear = 0.;

  if (relSmearingP_)
    psmear = p + sp*p;
  else
    psmear = p + sp;


  if (debug_) {
    std::cout<<"Track isTop: "<<isTop<<" nHits: "<<nhits<<" p: "<<p<<" deltaP=" << sp<<" p'="<<psmear<<std::endl;
  }

  return psmear;

}

double TrackSmearingTool::updateWithSmearP(Track& trk) {
  // If truth matching is required and track doesn't have a truth match, skip smearing
  if (requireTruthMatch_ && !hasTruthMatch(trk)) {
    if (debug_) {
      std::cout << "TrackSmearingTool: Skipping momentum smearing - no truth match" << std::endl;
    }
    return 1.0;  // No smearing applied
  }

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

void TrackSmearingTool::updateVertexWithSmearP(Vertex* vtx, double p1_smear_factor, double p2_smear_factor) {
    TVector3 p1_corr, p2_corr;

    if (debug_) {
        double p1_px = vtx->getP1X(), p1_py = vtx->getP1Y(), p1_pz = vtx->getP1Z();
        double p2_px = vtx->getP2X(), p2_py = vtx->getP2Y(), p2_pz = vtx->getP2Z();
        double p1_p = sqrt(p1_px*p1_px + p1_py*p1_py + p1_pz*p1_pz);
        double p2_p = sqrt(p2_px*p2_px + p2_py*p2_py + p2_pz*p2_pz);
        double psum_px = p1_px + p2_px, psum_py = p1_py + p2_py, psum_pz = p1_pz + p2_pz;
        double psum = sqrt(psum_px*psum_px + psum_py*psum_py + psum_pz*psum_pz);
        std::cout << "Before smearing:" << std::endl;
        std::cout << "  P1: px=" << p1_px << " py=" << p1_py << " pz=" << p1_pz << " |p|=" << p1_p << std::endl;
        std::cout << "  P2: px=" << p2_px << " py=" << p2_py << " pz=" << p2_pz << " |p|=" << p2_p << std::endl;
        std::cout << "  Psum:     px=" << psum_px << " py=" << psum_py << " pz=" << psum_pz << " |psum|=" << psum << std::endl;
    }


    // We don't know whether p1 or p2 is the electron
    // TODO: Introduce matching between the electron Track (not in this function), and p1/p2 to determine which is which
    // This is most easily done by comparing the py values of p1/2_corr and the track, since they should be in different regions of the detector (top/bottom)
    p1_corr.SetX(vtx->getP1X()*p1_smear_factor);
    p1_corr.SetY(vtx->getP1Y()*p1_smear_factor);
    p1_corr.SetZ(vtx->getP1Z()*p1_smear_factor);

    p2_corr.SetX(vtx->getP2X()*p2_smear_factor);
    p2_corr.SetY(vtx->getP2Y()*p2_smear_factor);
    p2_corr.SetZ(vtx->getP2Z()*p2_smear_factor);

    if (debug_) {
        double p1_p_corr = p1_corr.Mag();
        double p2_p_corr = p2_corr.Mag();
        TVector3 psum_corr = p1_corr + p2_corr;
        std::cout << "After smearing (p1_factor=" << p1_smear_factor << ", p2_factor=" << p2_smear_factor << "):" << std::endl;
        std::cout << "  P1: px=" << p1_corr.X() << " py=" << p1_corr.Y() << " pz=" << p1_corr.Z() << " |p|=" << p1_p_corr << std::endl;
        std::cout << "  P2: px=" << p2_corr.X() << " py=" << p2_corr.Y() << " pz=" << p2_corr.Z() << " |p|=" << p2_p_corr << std::endl;
        std::cout << "  Psum:     px=" << psum_corr.X() << " py=" << psum_corr.Y() << " pz=" << psum_corr.Z() << " |psum|=" << psum_corr.Mag() << std::endl;
    }

    // smear invariant mass by sqrt(p1_smear*p2_smear)
    double m_corr = vtx->getInvMass() * sqrt(p1_smear_factor * p2_smear_factor);
    vtx->setVtxParameters(p1_corr, p2_corr, m_corr);
}

double TrackSmearingTool::smearTrackZ0(const double z0) {

  double rel_smear = (*normal_)(*generator_);
  double sz0 = rel_smear * z0SmearingValue_;

  double z0smear = 0.;

  if (relSmearingZ0_)
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
    if (relSmearingZ0_)
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
  // If truth matching is required and track doesn't have a truth match, skip smearing
  if (requireTruthMatch_ && !hasTruthMatch(trk)) {
    if (debug_) {
      std::cout << "TrackSmearingTool: Skipping z0 smearing - no truth match" << std::endl;
    }
    return;
  }

  double smeared_z0 = smearTrackZ0(trk);
  trk.setZ0(smeared_z0);
}

double TrackSmearingTool::updateWithSmearOmega(Track& trk, double bfield) {
  // If truth matching is required and track doesn't have a truth match, skip smearing
  if (requireTruthMatch_ && !hasTruthMatch(trk)) {
    if (debug_) {
      std::cout << "TrackSmearingTool: Skipping omega smearing - no truth match" << std::endl;
    }
    return 1.0;  // No smearing applied
  }

  // Store original momentum for scale factor calculation
  double original_p = trk.getP();

  // Get current omega (curvature)
  double omega = trk.getOmega();

  // Determine smearing value based on top/bottom
  bool isTop = trk.getTanLambda() > 0.;
  double smearingValue = isTop ? omegaSmearingValueTop_ : omegaSmearingValueBot_;

  // Generate Gaussian random and apply relative smearing to omega
  double rel_smear = (*normal_)(*generator_);
  double omega_smeared = omega * (1 + rel_smear * smearingValue);

  // Recalculate momentum from smeared omega
  // pt = |1/omega| * B * c, where c = 2.99792458e-04 GeV/(T*mm)
  double mom_param = 2.99792458e-04;
  double pt = fabs(1. / omega_smeared) * bfield * mom_param;

  // Calculate momentum components preserving track direction
  double px = pt * sin(trk.getPhi());
  double pz = pt * cos(trk.getPhi());
  double py = pt * trk.getTanLambda();

  // Update track momentum
  trk.setMomentum(px, py, pz);

  if (debug_) {
    std::cout << "TrackSmearingTool::updateWithSmearOmega:" << std::endl;
    std::cout << "  isTop: " << isTop << " smearingValue: " << smearingValue << std::endl;
    std::cout << "  omega: " << omega << " rel_smear: " << rel_smear << " omega': " << omega_smeared << std::endl;
    std::cout << "  original_p: " << original_p << " smeared_p: " << trk.getP() << std::endl;
  }

  return trk.getP() / original_p;  // Return scale factor
}

void TrackSmearingTool::setMCParticles(const std::vector<MCParticle*>* mc_particles) {
  mcParticles_ = mc_particles;
}

bool TrackSmearingTool::hasTruthMatch(Track& trk) {
  return utils::hasTruthMatch(trk, mcParticles_, debug_);
}

int TrackSmearingTool::getTruthPDG(Track& trk) {
  return utils::getTruthPDG(trk, mcParticles_, debug_);
}
