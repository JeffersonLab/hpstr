#pragma once

//------------------//
//     C++          //
//------------------//
#include <iostream>
#include <random>
#include <memory>

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
  
  // The seed needs to be set accordingly for each instance / job of the smearing tool
  TrackSmearingTool(const std::string& smearingfile,
                    const bool relSmearing = true,
                    const int seed = 42,
                    const std::string& tracks = "KalmanFullTracks");  
  
  double smearTrackP(const Track& trk);
  double updateWithSmearP(Track& trk);
  void updateVertexWithSmearP(Vertex* vtx, double ele_smear_factor, double pos_smear_factor);
  
 private:
  
  //Random engine
  std::shared_ptr<std::default_random_engine> generator_;

  // General Normal distributions

  std::shared_ptr<std::normal_distribution<double>> normal_;
  
  std::shared_ptr<TFile> smearingfile_;
  
  //Smearing terms
  TH1D* smearing_histo_top_;
  TH1D* smearing_histo_bot_;
  
  // debug
  bool debug_{false};
  bool relSmearing_{false};
  
};
