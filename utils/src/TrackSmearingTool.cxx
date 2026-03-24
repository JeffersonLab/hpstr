#include "TrackSmearingTool.h"
#include "TruthMatchingUtils.h"
#include "TFile.h"
#include "TH1D.h"

#include <stdexcept>
#include <cstdlib>
#include <iomanip>
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

    if (cfg.contains("pSmearing")) {
      const auto& ps = cfg["pSmearing"];
      pSmearingValueTop_ = ps["top"].get<double>() * smearingFactor_;
      pSmearingValueBot_ = ps["bot"].get<double>() * smearingFactor_;
      if (ps.contains("mu_data") && ps.contains("mu_mc")) {
        pMeanDataTop_ = ps["mu_data"]["top"].get<double>();
        pMeanDataBot_ = ps["mu_data"]["bot"].get<double>();
        pMeanMcTop_   = ps["mu_mc"]["top"].get<double>();
        pMeanMcBot_   = ps["mu_mc"]["bot"].get<double>();
        hasMeanCorrP_ = true;
      }
    }
    if (cfg.contains("z0Smearing")) {
      const auto& zs = cfg["z0Smearing"];
      z0SmearingValueTop_ = zs["top"].get<double>() * smearingFactor_;
      z0SmearingValueBot_ = zs["bot"].get<double>() * smearingFactor_;
      if (zs.contains("mu_data") && zs.contains("mu_mc")) {
        z0MeanDataTop_ = zs["mu_data"]["top"].get<double>();
        z0MeanDataBot_ = zs["mu_data"]["bot"].get<double>();
        z0MeanMcTop_   = zs["mu_mc"]["top"].get<double>();
        z0MeanMcBot_   = zs["mu_mc"]["bot"].get<double>();
        hasMeanCorrZ0_ = true;
      }
    }

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
      const auto& os = cfg["omegaSmearing"];
      omegaSmearingValueTop_ = os["top"].get<double>() * smearingFactor_;
      omegaSmearingValueBot_ = os["bot"].get<double>() * smearingFactor_;
      if (os.contains("mu_data") && os.contains("mu_mc")) {
        omegaMeanDataTop_ = os["mu_data"]["top"].get<double>();
        omegaMeanDataBot_ = os["mu_data"]["bot"].get<double>();
        omegaMeanMcTop_   = os["mu_mc"]["top"].get<double>();
        omegaMeanMcBot_   = os["mu_mc"]["bot"].get<double>();
        hasMeanCorrOmega_ = true;
      }
    }

    // Parse smearOmega flag (optional, default false)
    if (cfg.contains("smearOmega")) {
      smearOmega_ = cfg["smearOmega"].get<bool>();
    }

    // Parse all {section}_binned_{varname} entries from hpsplot tool JSON.
    // Each entry key encodes both the section and the lookup variable name.
    auto parseBinnedSection = [&](const std::string& prefix,
                                  std::map<std::string, BinnedParam>& binMap,
                                  bool applyFactor) {
      for (auto& [k, v] : cfg.items()) {
        if (k.rfind(prefix, 0) != 0) continue;
        std::string var = k.substr(prefix.size());
        if (var.empty() || !v.contains("top") || !v.contains("bot")) continue;
        BinnedParam& bp = binMap[var];
        bp.edgesTop = v["top"]["bin_edges"].get<std::vector<double>>();
        bp.edgesBot = v["bot"]["bin_edges"].get<std::vector<double>>();
        if (v["top"].contains("values")) {
          bp.valTop = v["top"]["values"].get<std::vector<double>>();
          bp.valBot = v["bot"]["values"].get<std::vector<double>>();
          if (applyFactor) {
            for (auto& x : bp.valTop) x *= smearingFactor_;
            for (auto& x : bp.valBot) x *= smearingFactor_;
          }
        }
        if (v["top"].contains("mu_data") && v["top"].contains("mu_mc")) {
          bp.muDatTop = v["top"]["mu_data"].get<std::vector<double>>();
          bp.muMcTop  = v["top"]["mu_mc"].get<std::vector<double>>();
          bp.muDatBot = v["bot"]["mu_data"].get<std::vector<double>>();
          bp.muMcBot  = v["bot"]["mu_mc"].get<std::vector<double>>();
        }
      }
    };
    parseBinnedSection("pSmearing_binned_",     pBinned_,     true);
    parseBinnedSection("omegaSmearing_binned_", omegaBinned_, true);
    parseBinnedSection("z0Smearing_binned_",    z0Binned_,    false);

    // Set hasMeanCorr flags if any binned entry (or scalar) has means
    for (const auto& [v, bp] : pBinned_)
      if (!bp.muDatTop.empty()) { hasMeanCorrP_ = true; break; }
    for (const auto& [v, bp] : omegaBinned_)
      if (!bp.muDatTop.empty()) { hasMeanCorrOmega_ = true; break; }
    for (const auto& [v, bp] : z0Binned_)
      if (!bp.muDatTop.empty()) { hasMeanCorrZ0_ = true; break; }

    useFixedSmearing_ = true;
    useSeparateTopBot_ = true;
    smearingFile_ = smearingfile;

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

double TrackSmearingTool::lookupBinnedValue(const std::vector<double>& edges,
                                             const std::vector<double>& values,
                                             double x) const {
  if (edges.size() < 2 || values.empty()) return 0.;
  if (x <= edges.front()) return values.front();
  if (x >= edges.back())  return values.back();
  auto it = std::upper_bound(edges.begin(), edges.end(), x);
  int bin = static_cast<int>(std::distance(edges.begin(), it)) - 1;
  if (bin < 0) bin = 0;
  if (bin >= static_cast<int>(values.size())) bin = static_cast<int>(values.size()) - 1;
  return values[bin];
}

double TrackSmearingTool::getLookupValue(const Track& track) const {
  if (binnedLookupVariable_ == "phi0" || binnedLookupVariable_ == "phi")
    return track.getPhi();
  if (binnedLookupVariable_ == "nHits" || binnedLookupVariable_ == "nhits")
    return static_cast<double>(track.getTrackerHitCount());
  return track.getTanLambda();  // default: tanLambda
}

void TrackSmearingTool::setForcedVariable(const std::string& var) {
  if (var.empty()) return;  // leave JSON defaults in place

  if (var == "flat") {
    // Force scalar mode — clear all binned maps so lookups fall back to flat values
    pBinned_.clear();
    omegaBinned_.clear();
    z0Binned_.clear();
    std::cout << "TrackSmearingTool: forced to flat (scalar) smearing." << std::endl;
    return;
  }

  // Validate that at least one section has a binned entry for the requested variable
  bool found = pBinned_.count(var) || omegaBinned_.count(var) || z0Binned_.count(var);
  if (!found)
    throw std::invalid_argument(
        "TrackSmearingTool: smearingVariable='" + var +
        "' requested but no binned smearing data was found for this variable in the JSON.");

  binnedLookupVariable_ = var;
  std::cout << "TrackSmearingTool: using binned smearing, variable='" << var << "'." << std::endl;
}

static void printBinTable(const std::string& varName,
                          const std::vector<double>& edgesT,
                          const std::vector<double>& sigT,
                          const std::vector<double>& edgesB,
                          const std::vector<double>& sigB,
                          const std::vector<double>& muDatT,
                          const std::vector<double>& muMcT,
                          const std::vector<double>& muDatB,
                          const std::vector<double>& muMcB) {
  int n = static_cast<int>(edgesT.size()) - 1;
  bool hasSig  = !sigT.empty();
  bool hasMean = !muDatT.empty();
  std::cout << "    binned (" << varName << ", " << n << " bins):\n";
  std::cout << std::scientific << std::setprecision(3);
  std::cout << "      bin   lo           hi          ";
  if (hasSig)  std::cout << " top_sigma    bot_sigma  ";
  if (hasMean) std::cout << "  top_mu_dat   bot_mu_dat   top_mu_mc    bot_mu_mc";
  std::cout << "\n";
  for (int i = 0; i < n; ++i) {
    std::cout << "      [" << std::setw(2) << i << "] "
              << std::setw(11) << edgesT[i] << "  "
              << std::setw(11) << edgesT[i+1];
    if (hasSig)
      std::cout << "  " << std::setw(11) << (i < (int)sigT.size() ? sigT[i] : 0.)
                << "  " << std::setw(11) << (i < (int)sigB.size() ? sigB[i] : 0.);
    if (hasMean)
      std::cout << "  " << std::setw(11) << (i < (int)muDatT.size() ? muDatT[i] : 0.)
                << "  " << std::setw(11) << (i < (int)muDatB.size() ? muDatB[i] : 0.)
                << "  " << std::setw(11) << (i < (int)muMcT.size()  ? muMcT[i]  : 0.)
                << "  " << std::setw(11) << (i < (int)muMcB.size()  ? muMcB[i]  : 0.);
    std::cout << "\n";
  }
}

void TrackSmearingTool::printConfig() const {
  const std::string sep(72, '=');
  std::cout << "\n" << sep << "\n";
  std::cout << "  TrackSmearingTool\n" << sep << "\n";
  std::cout << "  JSON:           " << smearingFile_ << "\n";
  std::cout << "  Mode:           "
            << (isData_ ? "Data  (mean correction only, no Gaussian smearing)"
                        : "MC    (Gaussian smearing applied)") << "\n";
  std::cout << std::fixed << std::setprecision(4);
  std::cout << "  smearingFactor: " << smearingFactor_ << "\n";
  std::cout << "  relSmearingP:   " << (relSmearingP_  ? "relative" : "absolute")
            << "   relSmearingZ0: "  << (relSmearingZ0_ ? "relative" : "absolute") << "\n";
  std::cout << "  smearOmega:     " << (smearOmega_ ? "true" : "false") << "\n";

  // p smearing
  std::cout << "\n  [p smearing]  (" << (relSmearingP_ ? "relative" : "absolute") << ")\n";
  std::cout << std::scientific << std::setprecision(4);
  std::cout << "    flat:     top = " << pSmearingValueTop_
            << "   bot = "            << pSmearingValueBot_ << "\n";
  if (hasMeanCorrP_ && pBinned_.empty()) {
    std::cout << "    mu_data:  top = " << pMeanDataTop_ << "   bot = " << pMeanDataBot_ << "\n";
    std::cout << "    mu_mc:    top = " << pMeanMcTop_   << "   bot = " << pMeanMcBot_   << "\n";
  }
  if (!pBinned_.empty()) {
    for (const auto& [var, bp] : pBinned_)
      printBinTable(var, bp.edgesTop, bp.valTop, bp.edgesBot, bp.valBot,
                    bp.muDatTop, bp.muMcTop, bp.muDatBot, bp.muMcBot);
  } else {
    std::cout << "    (no binned p smearing loaded)\n";
  }

  // omega smearing
  std::cout << "\n  [omega smearing]  (absolute)\n";
  std::cout << "    flat:     top = " << omegaSmearingValueTop_
            << "   bot = "            << omegaSmearingValueBot_ << "\n";
  if (hasMeanCorrOmega_ && omegaBinned_.empty()) {
    std::cout << "    mu_data:  top = " << omegaMeanDataTop_ << "   bot = " << omegaMeanDataBot_ << "\n";
    std::cout << "    mu_mc:    top = " << omegaMeanMcTop_   << "   bot = " << omegaMeanMcBot_   << "\n";
  }
  if (!omegaBinned_.empty()) {
    for (const auto& [var, bp] : omegaBinned_)
      printBinTable(var, bp.edgesTop, bp.valTop, bp.edgesBot, bp.valBot,
                    bp.muDatTop, bp.muMcTop, bp.muDatBot, bp.muMcBot);
  } else {
    std::cout << "    (no binned omega smearing loaded)\n";
  }

  // z0 smearing
  std::cout << "\n  [z0 smearing]  (" << (relSmearingZ0_ ? "relative" : "absolute") << ")\n";
  std::cout << "    flat:     top = " << z0SmearingValueTop_
            << "   bot = "            << z0SmearingValueBot_ << "\n";
  if (hasMeanCorrZ0_ && z0Binned_.empty()) {
    std::cout << "    mu_data:  top = " << z0MeanDataTop_ << "   bot = " << z0MeanDataBot_ << "\n";
    std::cout << "    mu_mc:    top = " << z0MeanMcTop_   << "   bot = " << z0MeanMcBot_   << "\n";
  }
  if (!z0Binned_.empty()) {
    for (const auto& [var, bp] : z0Binned_)
      printBinTable(var, bp.edgesTop, bp.valTop, bp.edgesBot, bp.valBot,
                    bp.muDatTop, bp.muMcTop, bp.muDatBot, bp.muMcBot);
  } else {
    std::cout << "    (no binned z0 smearing loaded)\n";
  }

  std::cout << sep << "\n\n";
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

    // --- Data mode: apply mean correction only, no Gaussian smearing ---
    if (isData_) {
      if (!applyMeanCorr_) return p;
      double mu_data = 0., mu_mc = 0.;
      bool hasMu = false;
      if (pBinned_.count(binnedLookupVariable_)) {
        const auto& bp = pBinned_.at(binnedLookupVariable_);
        if (!bp.muDatTop.empty()) {
          double lv = getLookupValue(track);
          mu_data = isTop ? lookupBinnedValue(bp.edgesTop, bp.muDatTop, lv)
                          : lookupBinnedValue(bp.edgesBot, bp.muDatBot, lv);
          mu_mc   = isTop ? lookupBinnedValue(bp.edgesTop, bp.muMcTop,  lv)
                          : lookupBinnedValue(bp.edgesBot, bp.muMcBot,  lv);
          hasMu = true;
        }
      }
      if (!hasMu) {
        if (!hasMeanCorrP_) return p;
        mu_data = isTop ? pMeanDataTop_ : pMeanDataBot_;
        mu_mc   = isTop ? pMeanMcTop_   : pMeanMcBot_;
      }
      double pcorr = relSmearingP_ ? ((mu_data > 0.) ? p * (mu_mc / mu_data) : p)
                                   : p + (mu_mc - mu_data);
      if (debug_)
        std::cout<<"Data p corr: isTop="<<isTop<<" p="<<p<<" mu_data="<<mu_data<<" mu_mc="<<mu_mc<<" p'="<<pcorr<<std::endl;
      return pcorr;
    }

    // --- MC mode: Gaussian smearing only ---
    double smearingValue;
    if (pBinned_.count(binnedLookupVariable_)) {
      const auto& bp = pBinned_.at(binnedLookupVariable_);
      double lv = getLookupValue(track);
      smearingValue = isTop ? lookupBinnedValue(bp.edgesTop, bp.valTop, lv)
                            : lookupBinnedValue(bp.edgesBot, bp.valBot, lv);
    } else {
      smearingValue = isTop ? pSmearingValueTop_ : pSmearingValueBot_;
    }

    double rel_smear = (*normal_)(*generator_);
    double sp = rel_smear * smearingValue;

    double psmear = relSmearingP_ ? p + sp * p : p + sp;

    if (debug_)
      std::cout<<"Track isTop: "<<isTop<<" p: "<<p<<" deltaP="<<sp<<" p'="<<psmear<<std::endl;

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

    // --- Data mode: apply mean correction only, no Gaussian smearing ---
    if (isData_) {
      if (!applyMeanCorr_) return z0;
      double mu_data = 0., mu_mc = 0.;
      bool hasMu = false;
      if (z0Binned_.count(binnedLookupVariable_)) {
        const auto& bp = z0Binned_.at(binnedLookupVariable_);
        if (!bp.muDatTop.empty()) {
          double lv = getLookupValue(track);
          mu_data = isTop ? lookupBinnedValue(bp.edgesTop, bp.muDatTop, lv)
                          : lookupBinnedValue(bp.edgesBot, bp.muDatBot, lv);
          mu_mc   = isTop ? lookupBinnedValue(bp.edgesTop, bp.muMcTop,  lv)
                          : lookupBinnedValue(bp.edgesBot, bp.muMcBot,  lv);
          hasMu = true;
        }
      }
      if (!hasMu) {
        if (!hasMeanCorrZ0_) return z0;
        mu_data = isTop ? z0MeanDataTop_ : z0MeanDataBot_;
        mu_mc   = isTop ? z0MeanMcTop_   : z0MeanMcBot_;
      }
      double z0corr = z0 + (mu_mc - mu_data);
      if (debug_) {
        std::cout << "z0 smear (data): isTop=" << isTop
                  << "  source=" << (hasMu ? "binned[" + binnedLookupVariable_ + "]" : "flat")
                  << "  mu_data=" << mu_data << "  mu_mc=" << mu_mc
                  << "  z0=" << z0 << "  shift=" << (mu_mc - mu_data) << "  z0'=" << z0corr << std::endl;
      }
      return z0corr;
    }

    // --- MC mode: Gaussian smearing only ---
    double smearingValue;
    bool usedBinned = false;
    double lookupVal = 0.;
    if (z0Binned_.count(binnedLookupVariable_) &&
        !z0Binned_.at(binnedLookupVariable_).valTop.empty()) {
      const auto& bp = z0Binned_.at(binnedLookupVariable_);
      lookupVal = getLookupValue(track);
      smearingValue = isTop ? lookupBinnedValue(bp.edgesTop, bp.valTop, lookupVal)
                            : lookupBinnedValue(bp.edgesBot, bp.valBot, lookupVal);
      usedBinned = true;
    } else {
      smearingValue = isTop ? z0SmearingValueTop_ : z0SmearingValueBot_;
    }

    double rel_smear = (*normal_)(*generator_);
    double sz0 = rel_smear * smearingValue;

    double z0smear = relSmearingZ0_ ? z0 + sz0 * z0 : z0 + sz0;

    if (debug_) {
      std::cout << "z0 smear (MC): isTop=" << isTop
                << "  source=" << (usedBinned ? "binned[" + binnedLookupVariable_ + "=" + std::to_string(lookupVal) + "]" : "flat")
                << "  sigma=" << smearingValue
                << "  z0=" << z0 << "  deltaZ0=" << sz0 << "  z0'=" << z0smear << std::endl;
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

double TrackSmearingTool::updateWithSmearOmega(Track& trk, double /*bfield*/) {
  // If truth matching is required and track doesn't have a truth match, skip smearing
  if (requireTruthMatch_ && !hasTruthMatch(trk)) {
    if (debug_) {
      std::cout << "TrackSmearingTool: Skipping omega smearing - no truth match" << std::endl;
    }
    return 1.0;  // No smearing applied
  }

  // Get current omega (curvature)
  double omega = trk.getOmega();

  // Determine smearing value based on top/bottom (with optional binned lookup)
  bool isTop = trk.getTanLambda() > 0.;
  double smearingValue;
  if (omegaBinned_.count(binnedLookupVariable_)) {
    const auto& bp = omegaBinned_.at(binnedLookupVariable_);
    double lv = getLookupValue(trk);
    smearingValue = isTop ? lookupBinnedValue(bp.edgesTop, bp.valTop, lv)
                          : lookupBinnedValue(bp.edgesBot, bp.valBot, lv);
  } else {
    smearingValue = isTop ? omegaSmearingValueTop_ : omegaSmearingValueBot_;
  }

  // --- Data mode: apply mean correction only, no Gaussian smearing ---
  if (isData_) {
    if (!applyMeanCorr_) return 1.0;
    double mu_data = 0., mu_mc = 0.;
    bool hasMu = false;
    if (omegaBinned_.count(binnedLookupVariable_)) {
      const auto& bp = omegaBinned_.at(binnedLookupVariable_);
      if (!bp.muDatTop.empty()) {
        double lv = getLookupValue(trk);
        mu_data = isTop ? lookupBinnedValue(bp.edgesTop, bp.muDatTop, lv)
                        : lookupBinnedValue(bp.edgesBot, bp.muDatBot, lv);
        mu_mc   = isTop ? lookupBinnedValue(bp.edgesTop, bp.muMcTop,  lv)
                        : lookupBinnedValue(bp.edgesBot, bp.muMcBot,  lv);
        hasMu = true;
      }
    }
    if (!hasMu) {
      if (!hasMeanCorrOmega_) return 1.0;
      mu_data = isTop ? omegaMeanDataTop_ : omegaMeanDataBot_;
      mu_mc   = isTop ? omegaMeanMcTop_   : omegaMeanMcBot_;
    }
    // Shift data omega toward MC mean, then rescale momentum
    double omega_corr = omega + (mu_mc - mu_data);
    double scale = (omega_corr != 0.) ? omega / omega_corr : 1.0;
    std::vector<double> momentum = trk.getMomentum();
    for (double& coord : momentum) coord *= scale;
    trk.setMomentum(momentum);
    trk.setOmega(omega_corr);
    if (debug_)
      std::cout<<"Data omega corr: isTop="<<isTop<<" omega="<<omega<<" mu_data="<<mu_data<<" mu_mc="<<mu_mc<<" omega'="<<omega_corr<<std::endl;
    return std::fabs(scale);
  }

  // --- MC mode: Gaussian smearing only ---
  // Generate Gaussian random and apply absolute smearing to omega
  double smear = (*normal_)(*generator_);
  double omega_smeared = omega + smear * smearingValue;

  // Scale momentum by omega / omega_smeared (since p ~ 1/|omega|).
  // This avoids reconstructing from helix parameters with a B-field value
  // that may not match the one used when the track was originally created
  // (bLocal from LCIO is not persisted into the ROOT ntuples).
  double scale = omega / omega_smeared;
  std::vector<double> momentum = trk.getMomentum();
  for (double& coord : momentum)
    coord *= scale;
  trk.setMomentum(momentum);
  trk.setOmega(omega_smeared);

  if (debug_) {
    double original_p = trk.getP() / fabs(scale);
    std::cout << "TrackSmearingTool::updateWithSmearOmega:" << std::endl;
    std::cout << "  isTop: " << isTop << " smearingValue: " << smearingValue << std::endl;
    std::cout << "  omega: " << omega << " smear: " << smear << " omega': " << omega_smeared << std::endl;
    std::cout << "  original_p: " << original_p << " smeared_p: " << trk.getP() << std::endl;
  }

  return fabs(scale);
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
