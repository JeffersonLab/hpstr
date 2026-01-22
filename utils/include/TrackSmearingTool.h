#pragma once

//------------------//
//     C++          //
//------------------//
#include <iostream>
#include <random>
#include <memory>
#include <fstream>

//------------------//
//     JSON         //
//------------------//
#include "json.hpp"
using json = nlohmann::json;

//------------------//
//    hpstr         //
//------------------//

#include "Track.h"
#include "Vertex.h"
#include "Particle.h"
#include "MCParticle.h"
#include "TrackerHit.h"

class TFile;
class TH1D;

class TrackSmearingTool {

 public :

  // Constructor that auto-detects file type based on extension:
  // - .root files: histogram-based smearing
  // - .json files: JSON config with top/bot values
  // The seed needs to be set accordingly for each instance / job of the smearing tool
  // The smearingFactor multiplies all smearing parameters (default 1.0)
  TrackSmearingTool(const std::string& smearingfile,
                    const bool relSmearing = true,
                    const int seed = 42,
                    const std::string& tracks = "KalmanFullTracks",
                    const double smearingFactor = 1.0);

  // Constructor with fixed smearing values (no histogram lookup)
  // The smearingFactor multiplies all smearing parameters (default 1.0)
  TrackSmearingTool(const double pSmearingValue,
                    const double z0SmearingValue,
                    const bool relSmearing = true,
                    const int seed = 42,
                    const double smearingFactor = 1.0);

  double smearTrackP(const Track& trk);
  double smearTrackP(const double p);
  double updateWithSmearP(Track& trk);
  void updateVertexWithSmearP(Vertex* vtx, double ele_smear_factor, double pos_smear_factor);

  double smearTrackZ0(const Track& trk);
  double smearTrackZ0(const double z0);
  void updateWithSmearZ0(Track& trk);

  // Truth matching support
  // Set the MC particles collection for truth matching (call once per event)
  void setMCParticles(const std::vector<MCParticle*>* mc_particles);

  // Enable/disable truth matching requirement (only smear tracks with truth match)
  void setRequireTruthMatch(bool require) { requireTruthMatch_ = require; }

  // Check if a track has a truth-matched electron/positron (abs(PDG) == 11)
  // Returns true if matched, false if no match or MC info unavailable
  bool hasTruthMatch(Track& trk);

 private:
  // Get the PDG code of the truth particle best matching this track
  int getTruthPDG(Track& trk);
  
  //Random engine
  std::shared_ptr<std::default_random_engine> generator_;

  // General Normal distributions

  std::shared_ptr<std::normal_distribution<double>> normal_;
  
  std::shared_ptr<TFile> smearingfile_;
  
  //Smearing terms
  TH1D* smearing_histo_top_{nullptr};
  TH1D* smearing_histo_bot_{nullptr};

  // Fixed smearing values (used when constructed with fixed values)
  double pSmearingValue_{0.};
  double z0SmearingValue_{0.};
  bool useFixedSmearing_{false};

  // Separate top/bottom smearing values (used when loaded from JSON config)
  double pSmearingValueTop_{0.};
  double pSmearingValueBot_{0.};
  double z0SmearingValueTop_{0.};
  double z0SmearingValueBot_{0.};
  bool useSeparateTopBot_{false};

  // debug
  bool debug_{false};
  bool relSmearing_{false};

  // factor to multiply all smearing parameters by
  double smearingFactor_{1.0};

  // Truth matching
  const std::vector<MCParticle*>* mcParticles_{nullptr};
  bool requireTruthMatch_{false};

};
