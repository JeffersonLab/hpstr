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

class TFile;
class TH1D;

class TrackSmearingTool {

 public :

  // Constructor that auto-detects file type based on extension:
  // - .root files: histogram-based smearing
  // - .json files: JSON config with top/bot values
  // The seed needs to be set accordingly for each instance / job of the smearing tool
  TrackSmearingTool(const std::string& smearingfile,
                    const bool relSmearing = true,
                    const int seed = 42,
                    const std::string& tracks = "KalmanFullTracks");

  // Constructor with fixed smearing values (no histogram lookup)
  TrackSmearingTool(const double pSmearingValue,
                    const double z0SmearingValue,
                    const bool relSmearing = true,
                    const int seed = 42);

  double smearTrackP(const Track& trk);
  double smearTrackP(const double p);
  double updateWithSmearP(Track& trk);
  void updateVertexWithSmearP(Vertex* vtx, double ele_smear_factor, double pos_smear_factor);

  double smearTrackZ0(const Track& trk);
  double smearTrackZ0(const double z0);
  void updateWithSmearZ0(Track& trk);
  
 private:
  
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
  bool debug_{true};
  bool relSmearing_{false};
  
};
