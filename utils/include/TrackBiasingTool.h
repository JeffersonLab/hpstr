#pragma once

//------------------//
//     C++          //
//------------------//
#include <iostream>
#include <map>
#include <memory>
#include <random>
#include <string>

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

  // Load run-keyed linear z0 calibration: z0 -= slope*|tanL| + intercept
  // JSON format: { "14194": { "ele_top": {"slope":..,"intercept":..}, ... }, ... }
  // Run lookup: largest key <= current run (same strategy as v0proj).
  void loadZ0Calibration(const std::string& jsonFile);

  // Set the active run; selects the calibration set with largest key <= run.
  // Call once per event (no-op if run hasn't changed).
  void setRun(int run);

  // Apply linear z0 calibration to a track (uses charge and tanLambda sign)
  void updateWithCalibrateZ0(Track& trk);

  void setDebug(bool debug) { debug_ = debug; }

private:

  std::shared_ptr<TFile> biasingfile_;

  //Biasing terms

  //This is per charge -1: electron +1: positron
  TH1D* eop_h_top_;
  TH1D* eop_h_bot_;

  // linear z0 calibration
  struct Z0CalibParams { double slope{0.0}; double intercept{0.0}; };
  struct Z0CalibSet {
    Z0CalibParams ele_top, ele_bot, pos_top, pos_bot;
  };
  std::map<int, Z0CalibSet> z0CalibByRun_;   // all loaded run entries
  Z0CalibSet z0CalibActive_;                  // currently selected run's params
  bool z0CalibLoaded_{false};
  int  z0CalibCurrentRun_{-1};               // last run used in setRun()

  // debug
  bool debug_{false};

};
