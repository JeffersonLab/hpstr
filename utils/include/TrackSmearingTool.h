#pragma once

//------------------//
//     C++          //
//------------------//
#include <iostream>
#include <random>
#include <memory>
#include <fstream>
#include <algorithm>
#include <cmath>
#include <map>
#include <string>
#include <vector>

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
  // relSmearingP/relSmearingZ0 control whether smearing is relative (multiplicative) or absolute (additive)
  TrackSmearingTool(const std::string& smearingfile,
                    const bool relSmearingP = true,
                    const bool relSmearingZ0 = false,
                    const int seed = 42,
                    const std::string& tracks = "KalmanFullTracks",
                    const double smearingFactor = 1.0);

  // Constructor with fixed smearing values (no histogram lookup)
  // The smearingFactor multiplies all smearing parameters (default 1.0)
  // relSmearingP/relSmearingZ0 control whether smearing is relative (multiplicative) or absolute (additive)
  TrackSmearingTool(const double pSmearingValue,
                    const double z0SmearingValue,
                    const bool relSmearingP = true,
                    const bool relSmearingZ0 = false,
                    const int seed = 42,
                    const double smearingFactor = 1.0);

  double smearTrackP(const Track& trk);
  double smearTrackP(const double p);
  double updateWithSmearP(Track& trk);
  void updateVertexWithSmearP(Vertex* vtx, double p1_smear_factor, double p2_smear_factor);

  // Omega (curvature) smearing - alternative to momentum smearing
  // Smears omega directly, then recalculates momentum from smeared omega
  // Returns the scale factor applied to momentum (smeared_p / original_p)
  double updateWithSmearOmega(Track& trk, double bfield = 0.52);

  double smearTrackZ0(const Track& trk);
  double smearTrackZ0(const double z0);
  void updateWithSmearZ0(Track& trk);

  // Truth matching support
  // Set the MC particles collection for truth matching (call once per event)
  void setMCParticles(const std::vector<MCParticle*>* mc_particles);

  // Enable/disable truth matching requirement (only smear tracks with truth match)
  void setRequireTruthMatch(bool require) { requireTruthMatch_ = require; }
  void setDebug(bool debug) { debug_ = debug; }

  // Declare whether this instance is processing data (true) or MC (false).
  // - MC mode (default): applies Gaussian smearing only.
  // - Data mode: applies mean correction only (shifts data mean toward MC mean);
  //   no Gaussian smearing is applied.  Requires mean values in the JSON and
  //   setApplyMeanCorr(true).
  void setIsData(bool isData) { isData_ = isData; }

  // Enable the mean correction.  Has no effect in MC mode.
  // For relative smearing (p): scale factor mu_mc/mu_data applied to data.
  // For absolute smearing (z0, omega): additive shift mu_mc - mu_data.
  void setApplyMeanCorr(bool apply) { applyMeanCorr_ = apply; }

  // Print a full human-readable summary of the loaded configuration.
  // Call this after all setters (setIsData, setForcedVariable, …) are done.
  void printConfig() const;

  // Explicitly select which smearing parameterization to use.
  // "flat"      -> use scalar top/bot values, disable binned lookup
  // "nHits", "tanLambda", "phi0" -> require binned lookup with that variable;
  //               throws if the JSON does not contain a matching binned section.
  // ""          -> use whatever the JSON provides (default, not recommended)
  void setForcedVariable(const std::string& var);

  // Override the parameterization used for scale (mean) corrections in data mode.
  // When set to a variable name (e.g. "tanLambda"), the data-mode omega smearing
  // derives the scale factor from pSmearing_binned_{var} mu_data/mu_mc values,
  // applying a p-space multiplicative correction rather than an additive omega shift.
  // By default (empty string) the scale correction comes from omegaSmearing_binned_{binnedLookupVariable_}.
  void setScaleCorrVariable(const std::string& var);

  // Check if a track has a truth-matched electron/positron (abs(PDG) == 11)
  // Returns true if matched, false if no match or MC info unavailable
  bool hasTruthMatch(Track& trk);

 private:
  // Get the PDG code of the truth particle best matching this track
  int getTruthPDG(Track& trk);

  // Binned smearing lookup helpers
  // Returns the smearing value for the given x using linear bin lookup
  double lookupBinnedValue(const std::vector<double>& edges,
                           const std::vector<double>& values,
                           double x) const;
  // Returns the lookup variable value from the track (tanLambda or phi0)
  double getLookupValue(const Track& track) const;
  
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

  // Omega (curvature) smearing values
  double omegaSmearingValueTop_{0.};
  double omegaSmearingValueBot_{0.};
  bool smearOmega_{false};  // If true, use omega smearing instead of p smearing

  // Binned smearing lookup tables (from hpsplot tool JSON).
  // Each map key is a variable name (e.g. "tanLambda", "phi0", "nHits").
  // Keys come from JSON entries named {section}_binned_{varname}.
  struct BinnedParam {
    std::vector<double> edgesTop, edgesBot;
    std::vector<double> valTop,   valBot;    // sigma values; empty for z0 (mean-only)
    std::vector<double> muDatTop, muDatBot;  // mean corrections
    std::vector<double> muMcTop,  muMcBot;
  };
  std::string binnedLookupVariable_{"tanLambda"};  // active variable; set by setForcedVariable
  std::string scaleCorrVariable_{""};  // if set, omega data-mode scale uses pBinned_ means
  std::map<std::string, BinnedParam> pBinned_;
  std::map<std::string, BinnedParam> omegaBinned_;
  std::map<std::string, BinnedParam> z0Binned_;

  // Mean corrections (from mu_data / mu_mc in the tool JSON)
  // Scalar — from 1D fits for top/bot regions
  double pMeanDataTop_{0.},     pMeanMcTop_{0.};
  double pMeanDataBot_{0.},     pMeanMcBot_{0.};
  double z0MeanDataTop_{0.},    z0MeanMcTop_{0.};
  double z0MeanDataBot_{0.},    z0MeanMcBot_{0.};
  double omegaMeanDataTop_{0.}, omegaMeanMcTop_{0.};
  double omegaMeanDataBot_{0.}, omegaMeanMcBot_{0.};
  bool hasMeanCorrP_{false};
  bool hasMeanCorrZ0_{false};
  bool hasMeanCorrOmega_{false};
  bool applyMeanCorr_{false};
  bool isData_{false};

  std::string smearingFile_{""};  // path stored for printConfig()

  // debug
  bool debug_{false};
  bool relSmearingP_{false};
  bool relSmearingZ0_{false};

  // factor to multiply all smearing parameters by
  double smearingFactor_{1.0};

  // Truth matching
  const std::vector<MCParticle*>* mcParticles_{nullptr};
  bool requireTruthMatch_{false};

};
