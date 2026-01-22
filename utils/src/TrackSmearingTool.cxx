#include "TrackSmearingTool.h"
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
                                     const bool relSmearing,
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
    relSmearing_ = cfg.value("relSmearing", false);
    useFixedSmearing_ = true;
    useSeparateTopBot_ = true;

    if (debug_) {
      std::cout<<"Using JSON smearing config: "<<smearingfile<<std::endl;
      std::cout<<"  smearingFactor: "<<smearingFactor_<<std::endl;
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
                                     const int seed,
                                     const double smearingFactor) {

  smearingFactor_ = smearingFactor;
  relSmearing_ = relSmearing;
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
    sp = rel_smear * smearing_histo_top_->GetBinContent(binN) * smearingFactor_;
  else
    sp = rel_smear * smearing_histo_bot_->GetBinContent(binN) * smearingFactor_;
  
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

void TrackSmearingTool::updateVertexWithSmearP(Vertex* vtx, double ele_smear_factor, double pos_smear_factor) {
    TVector3 p1_corr, p2_corr;

    if (debug_) {
        double ele_px = vtx->getP1X(), ele_py = vtx->getP1Y(), ele_pz = vtx->getP1Z();
        double pos_px = vtx->getP2X(), pos_py = vtx->getP2Y(), pos_pz = vtx->getP2Z();
        double ele_p = sqrt(ele_px*ele_px + ele_py*ele_py + ele_pz*ele_pz);
        double pos_p = sqrt(pos_px*pos_px + pos_py*pos_py + pos_pz*pos_pz);
        double psum_px = ele_px + pos_px, psum_py = ele_py + pos_py, psum_pz = ele_pz + pos_pz;
        double psum = sqrt(psum_px*psum_px + psum_py*psum_py + psum_pz*psum_pz);
        std::cout << "Before smearing:" << std::endl;
        std::cout << "  Electron: px=" << ele_px << " py=" << ele_py << " pz=" << ele_pz << " |p|=" << ele_p << std::endl;
        std::cout << "  Positron: px=" << pos_px << " py=" << pos_py << " pz=" << pos_pz << " |p|=" << pos_p << std::endl;
        std::cout << "  Psum:     px=" << psum_px << " py=" << psum_py << " pz=" << psum_pz << " |psum|=" << psum << std::endl;
    }


    // We don't know whether p1 or p2 is the electron
    // TODO: Introduce matching between the electron Track (not in this function), and p1/p2 to determine which is which
    // This is most easily done by comparing the py values of p1/2_corr and the track, since they should be in different regions of the detector (top/bottom)
    p1_corr.SetX(vtx->getP1X()*ele_smear_factor);
    p1_corr.SetY(vtx->getP1Y()*ele_smear_factor);
    p1_corr.SetZ(vtx->getP1Z()*ele_smear_factor);

    p2_corr.SetX(vtx->getP2X()*pos_smear_factor);
    p2_corr.SetY(vtx->getP2Y()*pos_smear_factor);
    p2_corr.SetZ(vtx->getP2Z()*pos_smear_factor);

    if (debug_) {
        double ele_p_corr = p1_corr.Mag();
        double pos_p_corr = p2_corr.Mag();
        TVector3 psum_corr = p1_corr + p2_corr;
        std::cout << "After smearing (ele_factor=" << ele_smear_factor << ", pos_factor=" << pos_smear_factor << "):" << std::endl;
        std::cout << "  Electron: px=" << p1_corr.X() << " py=" << p1_corr.Y() << " pz=" << p1_corr.Z() << " |p|=" << ele_p_corr << std::endl;
        std::cout << "  Positron: px=" << p2_corr.X() << " py=" << p2_corr.Y() << " pz=" << p2_corr.Z() << " |p|=" << pos_p_corr << std::endl;
        std::cout << "  Psum:     px=" << psum_corr.X() << " py=" << psum_corr.Y() << " pz=" << psum_corr.Z() << " |psum|=" << psum_corr.Mag() << std::endl;
    }

    // smear invariant mass by sqrt(ele_smear*pos_smear)
    double m_corr = vtx->getInvMass() * sqrt(ele_smear_factor * pos_smear_factor);
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

void TrackSmearingTool::setMCParticles(const std::vector<MCParticle*>* mc_particles) {
  mcParticles_ = mc_particles;
}

bool TrackSmearingTool::hasTruthMatch(Track& trk) {
  int pdg = getTruthPDG(trk);
  return (std::abs(pdg) == 11);
}

int TrackSmearingTool::getTruthPDG(Track& trk) {
  if (!mcParticles_ || mcParticles_->empty()) {
    if (debug_) {
      std::cout << "TrackSmearingTool::getTruthPDG: No MC particles available" << std::endl;
    }
    return 0;
  }

  // Try two methods to find the MC particle ID with the most hits on this track:
  // Method 1: Use getMcpHits() from Track (populated by VertexProcessor)
  // Method 2: Use getSvtHits() and getMCPartIDs() from TrackerHits

  std::map<int, int> count_per_particle_id;

  // Method 1: Try getMcpHits() first
  auto mcp_hits = trk.getMcpHits();
  if (debug_) {
    std::cout << "TrackSmearingTool::getTruthPDG: Method 1 - track has " << mcp_hits.size() << " MCP hits" << std::endl;
  }

  for (const auto& [layer_id, particle_id] : mcp_hits) {
    count_per_particle_id[particle_id]++;
  }

  // Method 2: If no MCP hits, try getting MC info from TrackerHits via getSvtHits()
  if (count_per_particle_id.empty()) {
    TRefArray svt_hits = trk.getSvtHits();
    if (debug_) {
      std::cout << "TrackSmearingTool::getTruthPDG: Method 2 - track has " << svt_hits.GetEntries() << " SVT hits" << std::endl;
    }

    for (int i = 0; i < svt_hits.GetEntries(); i++) {
      TrackerHit* hit = static_cast<TrackerHit*>(svt_hits.At(i));
      if (hit) {
        std::vector<int> mc_part_ids = hit->getMCPartIDs();
        for (int part_id : mc_part_ids) {
          count_per_particle_id[part_id]++;
        }
      }
    }
  }

  if (debug_) {
    std::cout << "TrackSmearingTool::getTruthPDG: found " << count_per_particle_id.size() << " unique particle IDs" << std::endl;
  }

  // Find particle with most hits
  int truth_id{-1}, max_nhits{0};
  for (const auto& [particle_id, count] : count_per_particle_id) {
    if (count > max_nhits) {
      truth_id = particle_id;
      max_nhits = count;
    }
  }

  if (truth_id < 0) {
    if (debug_) {
      std::cout << "TrackSmearingTool::getTruthPDG: No hits found, returning 0" << std::endl;
    }
    return 0;
  }

  // Find the MC particle with this ID and return its PDG
  for (MCParticle* ptr : *mcParticles_) {
    if (ptr->getID() == truth_id) {
      if (debug_) {
        std::cout << "TrackSmearingTool::getTruthPDG: Match found, PDG=" << ptr->getPDG() << std::endl;
      }
      return ptr->getPDG();
    }
  }

  if (debug_) {
    std::cout << "TrackSmearingTool::getTruthPDG: No MC particle found with ID " << truth_id << std::endl;
  }
  return 0;
}
