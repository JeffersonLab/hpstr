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

class TFile;
class TH1D;

class TrackSmearingTool {
  
 public : 
  
  TrackSmearingTool(const std::string& smearingfile,
                    const int seed = 42,
                    const std::string& tracks = "KalmanFullTracks");  

  double smearTrackP(const Track& trk);
  
  void loadSmearingFile() {};
    
 private:
  
  //Random engine
  std::shared_ptr<std::default_random_engine> generator_;
  
  //Vector of normal distributions, one distribution for each sigma
  std::vector<std::shared_ptr<std::normal_distribution<float>>> normals_top_;
  std::vector<std::shared_ptr<std::normal_distribution<float>>> normals_bot_;
  
  std::shared_ptr<TFile> smearingfile_;
  
  //Smearing terms
  TH1D* smearing_histo_top_;
  TH1D* smearing_histo_bot_;
  

  // debug
  bool debug_{true};
  
};
