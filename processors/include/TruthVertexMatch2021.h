/*
 * @file TruthVertexMatch2021.h
 * @brief Truth-A' -> reconstructed-vertex matching study for 2021 displaced A'/SIMP signal MC.
 *
 * Step 1 (this version): replicate the PreselectAndCategorize2021 truth block that finds the
 * true A' (or SIMP vector) and its e+e- decay, and write the truth decay vertex + kinematics
 * to a flat tree. Step 2 (later): search the reconstructed UnconstrainedV0Vertices_KF for a
 * vertex matching this truth vertex (matching strategy TBD).
 *
 * Reuses the lean EventBus/Cutflow helpers from PreselectAndCategorize.h (no preselection
 * machinery), mirroring ClusterEfficiency2021.
 *
 * @author Matthew Gignac
 * @date June 2026
 */

#ifndef __TRUTH_VERTEX_MATCH_2021_H__
#define __TRUTH_VERTEX_MATCH_2021_H__

#include <memory>
#include <string>

#include "PreselectAndCategorize.h"  // EventBus, Cutflow, Processor, AnaHelpers, event classes
#include "TruthMatchingUtils.h"
#include "TVector3.h"

class TruthVertexMatch2021 : public Processor {
  public:
    TruthVertexMatch2021(const std::string& name, Process& process) : Processor(name, process) {}
    ~TruthVertexMatch2021() = default;
    virtual void configure(const ParameterSet& parameters) final;
    virtual void initialize(TTree* tree) final;
    virtual void setFile(TFile* out_file) final;
    virtual bool process(IEvent* ievent) final;
    virtual void finalize() final;

  private:
    EventBus bus_;
    std::unique_ptr<TTree> output_tree_;
    Cutflow event_cf_{"event", "readout"};
    std::shared_ptr<AnaHelpers> _ah;

    std::string vtxColl_{"UnconstrainedV0Vertices_KF"};
    std::string mcColl_{"MCParticle"};
    std::string trkColl_{"KalmanFullTracks"};
    // Truth-tagged 2D SVT hits (written by Tracker2DHitProcessor when recoTuple_cfg.py is run with
    // -sh/--truthHits 1). Older/other productions won't have this branch at all, so availability is
    // checked at runtime in initialize() -- see hasTruthHits_.
    std::string hitColl_{"RotatedHelicalTrackHits"};
    bool hasTruthHits_{false};

    bool isData_{false};
    bool isApSignal_{false};
    bool isSimpSignal_{false};
    int apPDG_{622};         // 622 displaced A', 623 prompt A'
    int signalMomPDG_{622};  // mother PDG of the signal e+/e- (apPDG_ for A', 625 for SIMP)
    double calTimeOffset_{0.0};
    double vertexMatchZ_{5.0};  // |z_reco - z_truth| window [mm] for a v0 to "match" the truth vertex
    // Optional truth e+e- invariant-mass window. For the rad (continuum) sample this slices the
    // spectrum to a fixed mass so it is a proper mass-matched control for a given A' mass. When
    // massSelect_ < 0 the window is disabled (all truth-signal events kept).
    double massSelect_{-1.0};    // target true e+e- invariant mass [GeV]; <0 disables
    double massWindow_{0.002};   // half-width of the mass window [GeV]
    // Optional truth decay-vertex z fiducial window [mm]. Restricts the studied events to those
    // whose A' (or VD) decays in [vtxZMin_, vtxZMax_] BEFORE evaluating any acceptance. Setting
    // vtxZMax_ < ~50 mm keeps decays upstream of the first tracking layer.
    double vtxZMin_{-1.0e9};
    double vtxZMax_{1.0e9};
    // Electron-momentum threshold [GeV] for the secondary (energetic-e-) category reported
    // alongside the inclusive rates.
    double eleMomCut_{0.5};
    // Sanity cut on the electron leg's ECal cluster energy [GeV]: below this, a "matched" cluster
    // is treated as absent. Guards against noise-level cluster matches inflating has_ele_clus.
    double eleClusEMin_{0.2};
    bool debug_{false};

    // running counters for the terminal rate summary (printed in finalize)
    long n_truth_{0};      // events with a truth signal (A'/VD) found
    long n_has_v0_{0};     // ... with >=1 reconstructed v0 (no selection)
    long n_matched_z_{0};  // ... with >=1 v0 within vertexMatchZ_ in z of the truth vertex
    // track-based truth matching: a v0 "track-matches" when its two constituent tracks are each
    // majority-hit truth-matched (see utils::getTruthPDG) to the true signal e-/e+, independent
    // of where the vertex fit places z. Lets us see how much the z-window under/over-counts.
    long n_matched_trk_{0};        // ... with >=1 v0 that is track-matched
    long n_matched_z_and_trk_{0};  // ... with >=1 v0 that is BOTH within the z-window AND track-matched
    long n_matched_trk_ele_{0};    // ... with >=1 v0 carrying the truth e- (no requirement on e+)
    long n_matched_trk_pos_{0};    // ... with >=1 v0 carrying the truth e+ (no requirement on e-)
    // ECal-cluster-aware track matching: quantifies the acceptance gained by relaxing the
    // electron ECal-cluster requirement (the point of the singles trigger, which only needs the
    // positron's cluster). Both counters below are subsets of n_matched_trk_ (both tracks correct).
    long n_matched_trk_ele_clus_{0};        // ... with >=1 v0: truth e- track has a cluster (any e+)
    long n_matched_trk_pos_clus_{0};        // ... with >=1 v0: truth e+ track has a cluster (any e-)
    long n_matched_trk_clus_both_{0};       // ... with >=1 v0: both tracks correct AND both have clusters (strict)
    long n_matched_trk_clus_pos_only_{0};   // ... with >=1 v0: both tracks correct AND e+ has a cluster (relaxed)
    long n_matched_trk_clus_ele_only_{0};   // ... with >=1 v0: both tracks correct AND e- has a cluster (e+ optional)
    // no-v0 track inventory (KalmanFullTracks, NO truth match) -> tracking vs vertexing
    long n_novtx_both_trk_{0};  // no-v0 events with >=1 e+ AND >=1 e- track  (vertexing issue)
    long n_novtx_pos_only_{0};  // no-v0 with e+ track but no e- track
    long n_novtx_ele_only_{0};  // no-v0 with e- track but no e+ track
    long n_novtx_neither_{0};   // no-v0 with neither track
    // secondary category: events whose truth electron momentum > eleMomCut_
    long n_truth_ehi_{0};
    long n_has_v0_ehi_{0};
    long n_matched_z_ehi_{0};
};

#endif  //__TRUTH_VERTEX_MATCH_2021_H__
