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

class TrackSmearingTool {
  
 public : 
  
  TrackSmearingTool(const std::string& smearingfile);  
  double smearTrackMomentum() {};
  
  void loadSmearingFile() {};
    
 private:
  
  //Random engine
  std::default_random_engine generator_;
  std::shared_ptr<std::normal_distribution<float>> normal_;
  
  std::shared_ptr<TFile> smearingfile_;
  

};
