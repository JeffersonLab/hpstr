/*
 * @file ClusterEfficiency2021.h
 * @brief Standalone study of positron-cluster (and v0) reconstruction efficiency
 *        for 2021 displaced A'/SIMP signal MC.
 *
 * Broken out of PreselectAndCategorize2021 so the common preselection code is
 * not bloated with this study. We reuse the small EventBus/Cutflow helper
 * classes defined in PreselectAndCategorize.h but do NOT inherit the
 * preselection processor; this keeps the study lean (no smearing/correction
 * machinery).
 *
 * @author Matthew Gignac
 * @date May 2026
 */

#ifndef __CLUSTER_EFFICIENCY_2021_H__
#define __CLUSTER_EFFICIENCY_2021_H__

#include <memory>
#include <string>

#include "PreselectAndCategorize.h"  // brings in EventBus, Cutflow, Processor, AnaHelpers, event classes
#include "TruthMatchingUtils.h"
#include "TVector3.h"

class ClusterEfficiency2021 : public Processor {
  public:
    ClusterEfficiency2021(const std::string& name, Process& process) : Processor(name, process) {}
    ~ClusterEfficiency2021() = default;
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

    bool isData_{false};
    bool isApSignal_{false};
    bool isSimpSignal_{false};
    int apPDG_{622};            // 622 displaced A', 623 prompt A'
    int signalMomPDG_{622};     // mother PDG of the signal e+/e- (apPDG_ for AP, 625 for SIMP)
    double calTimeOffset_{0.0};
    double clusterEnergyThresh_{0.0};  // a particle is considered to "have a cluster" if E > this
    double eventClusterThresh_{0.2};   // "energetic cluster" threshold for the event-level and
                                       // positron-matched cutflow steps (any/matched RecoEcalCluster E > this)
    double drMatch_{30.0};             // track->cluster dr match radius at ECal [mm]
    bool debug_{false};

    // running counters for the terminal summary printed in finalize()
    long n_triggered_{0};            // singles2/3 triggered events
    long n_event_has_cluster_{0};    // ... with any RecoEcalCluster E > eventClusterThresh_
    long n_has_v0_{0};               // ... with >=1 e+e- v0
    long n_multi_v0_{0};             // ... with >1 e+e- v0
    long n_first_pos_matched_{0};    // FIRST v0: e+ dr-matched to an energetic cluster
    long n_first_ele_matched_{0};    // FIRST v0: e- dr-matched
    long n_any_pos_matched_{0};      // ANY v0 (best case): some v0 has e+ dr-matched
    long n_any_ele_matched_{0};      // ANY v0 (best case): some v0 has e- dr-matched
    // track-level (KalmanFullTracks, no v0 required) -> isolates tracking vs vertexing
    long n_trk_pos_matched_{0};      // >=1 positive track dr-matched to an energetic cluster
    long n_trk_ele_matched_{0};      // >=1 negative track dr-matched
    long n_trkpos_no_v0pos_{0};      // e+ track matched but NO v0 e+ matched  <- vertexing loss
    int n_printed_{0};               // per-event debug lines printed so far
};

#endif  //__CLUSTER_EFFICIENCY_2021_H__
