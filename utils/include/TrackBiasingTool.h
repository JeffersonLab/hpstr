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

class TFile;
class TH1D;
class TVector3;

class TrackBiasingTool {
  
 public : 
  
  TrackBiasingTool(const std::string& biasingfile,
		   const std::string& tracks = "KalmanFullTracks");
  
  double biasTrackP(const Track& track);

  double getCorrection(const double& p,
		       const double tanL,
		       const int q);

  void updateVertexWithBiasP(Vertex* vtx);
   
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
