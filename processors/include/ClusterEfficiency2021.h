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
    bool debug_{false};
};

#endif  //__CLUSTER_EFFICIENCY_2021_H__
