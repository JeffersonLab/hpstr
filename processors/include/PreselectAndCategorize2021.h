/*
 * @file PreselectAndCategorize.h
 * @author Sarah Gaiser adapting Tom Eichlersmith's PreselectAndCategorize
 * @date Apr 2025
 */

#ifndef __PRESELECT_AND_CATEGORIZE_2021_H__
#define __PRESELECT_AND_CATEGORIZE_2021_H__

#include <sstream>
#include <string>
#include <set>


#include "Math/Vector4D.h"
#include "PreselectAndCategorize.h"
#include "TMath.h"
#include "TSData.h"
#include "TrackBiasingTool.h"
#include "TrackSmearingTool.h"
#include "TruthMatchingUtils.h"
#include "TrackTools.h"

class PreselectAndCategorize2021 : public PreselectAndCategorize {
  public:
    PreselectAndCategorize2021(const std::string& name, Process& process) : PreselectAndCategorize(name, process) {}
    ~PreselectAndCategorize2021() = default;
    virtual void configure(const ParameterSet& parameters) final;
    virtual void initialize(TTree* tree) final;
    virtual void setFile(TFile* out_file) final;
    virtual bool process(IEvent* ievent) final;
    virtual void finalize() final;

  private:
    std::string trkColl_{"KalmanFullTracks"};
    // std::string hitColl_{"SiClustersOnTrack"};
    int smearingSeed_{42};  // configurable seed (default 42 for backward compat)
    bool doSmearing_{true};      // master switch for track smearing
    bool doZ0Corrections_{true}; // if false, skip all z0 corrections (v0proj mean shift, z0 smearing)
    double smearingFactor_{1.0};  // factor to multiply smearing parameters by
    bool requireTruthMatch_{false};  // require truth match for smearing
    bool smearOmega_{false};  // use omega (curvature) smearing instead of p smearing
    std::string smearingVariable_{""};  // "flat", "nHits", "tanLambda", "phi0"; "" = JSON default
    std::string scaleCorrVariable_{""};  // if set, omega data-mode scale uses pBinned_ means
    bool applyMeanCorr_{false};          // enable omega-space mean correction (PATH B)
    bool isSimpSignal_{false};
    bool isApSignal_{false};
    int apPDG_{622};  // PDG code for A': 622 for displaced, 623 for prompt
    bool debug_{false};  // debug output for smearing validation
    bool disablePreselection_{false};  // if true, skip all vertex-level cuts
    bool disableTimingCuts_{false};    // if true, skip all track/cluster timing cuts
    int minHits_{10};  // minimum number of 2D tracker hits required for each track
    double beamE_{3.74};          // beam energy [GeV] used for recoil inference
    double thetaBeamMrad_{0.0};   // beam angle [mrad] used for recoil inference
    std::shared_ptr<TrackBiasingTool> biasingTool_{nullptr};
    std::vector<double> determine_time_cuts(bool isData, int runNumber);
    std::vector<double> time_cuts_{6.9, 5.2, 9.0};

    // Inferred recoil-electron angle relative to the beam direction, in mrad.
    // The recoil 3-momentum is reconstructed from momentum conservation:
    //   p_recoil = p_beam - p_ele - p_pos
    // and theta_R is the angle between p_recoil and the beam direction.
    double calculate_theta_R(const TVector3& ele_mom, const TVector3& pos_mom) const;

    bool saveAllTracks_{false}; // if true, save all tracks in the event
    bool calcMultiTrackVars_{false}; // if true, calculate event-level variables (min_dTanlambda, etc.)

    Track* createInferredTrack(Vertex* vtx, Track* ele_track, Track* pos_track);

    bool isQualityTrack(Track* trk, const Particle& pos) const;
    void calculatePairwiseQuantities(const std::vector<Track*>& all_tracks,  Particle& ele, Particle& pos);
};

#endif  //__PRESELECT_AND_CATEGORIZE_2021_H__
