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

class TrackBiasingTool {
  
 public : 
  
  TrackBiasingTool(const std::string& biasingfile,
		   const std::string& tracks = "KalmanFullTracks");
  
  double biasTrackP(const Track& track);

  //Update the track P with a specific scale Factor
  void updateWithBiasP(Track& trk, double scaleFactor);

  //Update the track P with scale Factors according to the internal calibration plots
  void updateWithBiasP(Track& trk);
  
private:
  
  std::shared_ptr<TFile> biasingfile_;
  
  //Biasing terms
  
  //This is per charge -1: electron +1: positron
  TH1D* eop_h_top_;
  TH1D* eop_h_bot_;

  // debug
  bool debug_{false};
    
};
