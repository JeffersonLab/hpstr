/*
 * @file ClusterEfficiency2021.cxx
 * @brief Standalone study of positron-cluster (and v0) reconstruction efficiency
 *        for 2021 displaced A'/SIMP signal MC.
 * @author Matthew Gignac
 * @date May 2026
 */

#include "ClusterEfficiency2021.h"

#include <cmath>
#include <cstdio>

void ClusterEfficiency2021::configure(const ParameterSet& parameters) {
    isData_ = parameters.getInteger("isData", 0) != 0;
    isApSignal_ = parameters.getInteger("isApSignal", 0) != 0;
    isSimpSignal_ = parameters.getInteger("isSimpSignal", 0) != 0;
    apPDG_ = parameters.getInteger("apPDG", apPDG_);
    calTimeOffset_ = parameters.getDouble("calTimeOffset", 0.0);
    clusterEnergyThresh_ = parameters.getDouble("clusterEnergyThresh", 0.0);
    eventClusterThresh_ = parameters.getDouble("eventClusterThresh", 0.2);
    drMatch_ = parameters.getDouble("drMatch", 30.0);
    debug_ = parameters.getInteger("debug", 0) != 0;

    auto vtxColl = parameters.getString("vtxCollection", "");
    if (not vtxColl.empty()) vtxColl_ = vtxColl;

    // mother PDG of the signal e+/e-: 625 for SIMP vector decay, apPDG_ for A'
    signalMomPDG_ = isSimpSignal_ ? 625 : apPDG_;

    std::cout << "[ClusterEfficiency2021] isData=" << isData_ << " isApSignal=" << isApSignal_
              << " isSimpSignal=" << isSimpSignal_ << " signalMomPDG=" << signalMomPDG_
              << " vtxColl=" << vtxColl_ << " eventClusterThresh=" << eventClusterThresh_ << std::endl;
}

void ClusterEfficiency2021::initialize(TTree* tree) {
    _ah = std::make_shared<AnaHelpers>();

    bus_.board_input<EventHeader>(tree, "EventHeader");
    bus_.board_input<TSData>(tree, "TSBank");
    if (not trkColl_.empty()) bus_.board_input<std::vector<Track*>>(tree, trkColl_);
    bus_.board_input<std::vector<Vertex*>>(tree, vtxColl_);
    bus_.board_input<std::vector<CalCluster*>>(tree, "RecoEcalClusters");
    if (not isData_ and not mcColl_.empty()) bus_.board_input<std::vector<MCParticle*>>(tree, mcColl_);

    // efficiency chain (quick at-a-glance, in addition to the tree)
    event_cf_.add("single_trigger", 2, -0.5, 1.5);
    event_cf_.add("truth_epem_found", 2, -0.5, 1.5);
    event_cf_.add("has_v0", 2, -0.5, 1.5);
    event_cf_.add("both_tracks_fiducial", 2, -0.5, 1.5);
    event_cf_.add("pos_has_cluster", 2, -0.5, 1.5);
    event_cf_.init();
    event_cf_.set_label_names({"readout", "single2 or 3 trigger", "truth e^{+}e^{-} found",
                               "N_{vtx} #geq 1", "both tracks fiducial", "e^{+} has cluster"});
}

void ClusterEfficiency2021::setFile(TFile* out_file) {
    Processor::setFile(out_file);

    output_tree_ = std::make_unique<TTree>("cluster_eff", "Positron-cluster efficiency study", 99, out_file);

    bus_.board_output<double>(output_tree_.get(), "weight");
    bus_.board_output<int>(output_tree_.get(), "run");
    bus_.board_output<int>(output_tree_.get(), "event");

    // trigger
    bus_.board_output<bool>(output_tree_.get(), "single2");
    bus_.board_output<bool>(output_tree_.get(), "single3");

    // flags
    for (const auto& name : {"truth_epem_found", "has_v0", "ele_fiducial", "pos_fiducial",
                             "both_tracks_fiducial", "ele_has_cluster", "pos_has_cluster"}) {
        bus_.board_output<bool>(output_tree_.get(), name);
    }

    // truth quantities
    bus_.board_output<TVector3>(output_tree_.get(), "ele_truth_p");
    bus_.board_output<TVector3>(output_tree_.get(), "pos_truth_p");
    for (const auto& name : {"ele_truth_E", "pos_truth_E", "true_vertex_invM", "true_vertex_psum",
                             "true_decay_len", "true_betagamma"}) {
        bus_.board_output<double>(output_tree_.get(), name);
    }
    // reco track quantities
    bus_.board_output<TVector3>(output_tree_.get(), "ele_track_p");
    bus_.board_output<TVector3>(output_tree_.get(), "pos_track_p");
    for (const auto& name : {"ele_ecal_x", "ele_ecal_y", "ele_ecal_z",
                             "pos_ecal_x", "pos_ecal_y", "pos_ecal_z"}) {
        bus_.board_output<double>(output_tree_.get(), name);
    }

    // reco cluster quantities (associated to the v0 particles)
    for (const auto& name : {"ele_clu_E", "ele_clu_x", "ele_clu_y", "ele_clu_time",
                             "pos_clu_E", "pos_clu_x", "pos_clu_y", "pos_clu_time"}) {
        bus_.board_output<double>(output_tree_.get(), name);
    }
    bus_.board_output<int>(output_tree_.get(), "ele_clu_nhits");
    bus_.board_output<int>(output_tree_.get(), "pos_clu_nhits");

    // standalone-cluster cross-check near the positron track projection
    for (const auto& name : {"pos_nearest_clu_dr", "pos_nearest_clu_E", "pos_nearest_clu_time"}) {
        bus_.board_output<double>(output_tree_.get(), name);
    }

    // track-level match flags (KalmanFullTracks, no v0 required) + n_v0 multiplicity
    bus_.board_output<bool>(output_tree_.get(), "trk_pos_matched");
    bus_.board_output<bool>(output_tree_.get(), "trk_ele_matched");
    bus_.board_output<int>(output_tree_.get(), "n_v0");

    // tracking/acceptance-loss scenario: energetic cluster(s) present but no positron track on
    // one of them. "orphan" = energetic cluster with NO reconstructed track (any charge) within
    // drMatch_ -> the positron's lost cluster; map its x,y to see WHERE acceptance/tracking fails.
    bus_.board_output<bool>(output_tree_.get(), "pos_acceptance_loss");
    bus_.board_output<bool>(output_tree_.get(), "has_orphan_cluster");
    for (const auto& name : {"orphan_clu_x", "orphan_clu_y", "orphan_clu_E"}) {
        bus_.board_output<double>(output_tree_.get(), name);
    }
}

bool ClusterEfficiency2021::process(IEvent*) {
    const auto& tsbank{bus_.get<TSData>("TSBank")};
    const auto& eh{bus_.get<EventHeader>("EventHeader")};

    event_cf_.begin_event();

    // Re-apply trigger decision: data contains other triggers; MC is produced with only
    // this trigger but the event header is not updated, so the bits are still meaningful.
    bool single2 = tsbank.isSingle2Trigger() == 1;
    bool single3 = tsbank.isSingle3Trigger() == 1;
    bool single_trigger = single2 || single3;
    event_cf_.apply("single_trigger", single_trigger);
    event_cf_.fill_nm1("single_trigger", single_trigger ? 1 : 0);
    if (not event_cf_.keep()) return true;

    // ===== Q1: after the singles2/3 trigger, is there an energetic cluster ANYWHERE? =====
    // Loop over ALL RecoEcalClusters, independent of any v0. By definition a singles
    // trigger requires an energetic cluster, so this should be ~100%.
    const auto& allClusters{bus_.get<std::vector<CalCluster*>>("RecoEcalClusters")};
    int n_clusters = static_cast<int>(allClusters.size());
    double event_max_clu_E = -9999.;
    for (const auto* cl : allClusters) {
        if (cl->getEnergy() > event_max_clu_E) event_max_clu_E = cl->getEnergy();
    }
    bool event_has_cluster = event_max_clu_E > eventClusterThresh_;
    n_triggered_++;
    if (event_has_cluster) n_event_has_cluster_++;

    // nearest ENERGETIC cluster (E > eventClusterThresh_) to a track's ECal projection.
    // A track is "dr-matched" to a trigger-quality cluster if the nearest such cluster is
    // within drMatch_. Used to score/pick the best v0 and to fill the per-track dbg below.
    auto nearest_energetic = [&](const std::vector<double>& ecal,
                                 double& out_dr, double& out_E, double& out_t) {
        out_dr = 9999.0; out_E = -9999.0; out_t = -9999.0;
        for (const auto* cl : allClusters) {
            if (cl->getEnergy() <= eventClusterThresh_) continue;
            auto cp = cl->getPosition();
            double dr = std::sqrt((cp[0] - ecal[0]) * (cp[0] - ecal[0]) +
                                  (cp[1] - ecal[1]) * (cp[1] - ecal[1]));
            if (dr < out_dr) { out_dr = dr; out_E = cl->getEnergy(); out_t = cl->getTime() - calTimeOffset_; }
        }
    };

    // ----- defaults (sentinels) -----
    bus_.set("weight", 1.);
    bus_.set("run", eh.getRunNumber());
    bus_.set("event", eh.getEventNumber());
    bus_.set("single2", single2);
    bus_.set("single3", single3);

    bus_.set("ele_truth_p", TVector3(0., 0., 0.));
    bus_.set("pos_truth_p", TVector3(0., 0., 0.));
    bus_.set("ele_truth_E", -9999.);
    bus_.set("pos_truth_E", -9999.);
    bus_.set("true_vertex_invM", -9999.);
    bus_.set("true_vertex_psum", -9999.);
    bus_.set("true_decay_len", -9999.);
    bus_.set("true_betagamma", -9999.);

    bus_.set("ele_track_p", TVector3(0., 0., 0.));
    bus_.set("pos_track_p", TVector3(0., 0., 0.));
    for (const auto& name : {"ele_ecal_x", "ele_ecal_y", "ele_ecal_z",
                             "pos_ecal_x", "pos_ecal_y", "pos_ecal_z",
                             "ele_clu_E", "ele_clu_x", "ele_clu_y", "ele_clu_time",
                             "pos_clu_E", "pos_clu_x", "pos_clu_y", "pos_clu_time",
                             "pos_nearest_clu_dr", "pos_nearest_clu_E", "pos_nearest_clu_time"}) {
        bus_.set<double>(name, -9999.);
    }
    bus_.set("ele_clu_nhits", -1);
    bus_.set("pos_clu_nhits", -1);

    bool truth_epem_found = false, has_v0 = false;
    bool ele_fiducial = false, pos_fiducial = false, both_tracks_fiducial = false;
    bool ele_has_cluster = false, pos_has_cluster = false;
    // Q2: is an energetic cluster matched to the v0 positron track?
    bool pos_matched_cluster = false;          // associated cluster with E > eventClusterThresh_
    double pos_clu_E_dbg = -9999.;             // associated-cluster energy (sentinel if none)
    double pos_nearest_dr_dbg = -9999.;        // dr to nearest standalone cluster at ECal
    // Q3: dr-based track->cluster match (did the e-/e+ make an energetic trigger cluster?)
    bool ele_dr_matched = false, pos_dr_matched = false;
    double ele_nearest_dr_dbg = -9999., ele_dr_E_dbg = -9999.;
    double pos_dr_E_dbg = -9999.;

    // ===================== truth block =====================
    const std::vector<MCParticle*>* mcParticles = nullptr;
    if (bus_.has(mcColl_)) {
        mcParticles = &bus_.get<std::vector<MCParticle*>>(mcColl_);
        MCParticle* mother{nullptr};
        ROOT::Math::PxPyPzEVector trueEleP, truePosP;
        bool foundEle = false, foundPos = false;
        for (MCParticle* ptr : *mcParticles) {
            std::vector<double> lP = ptr->getMomentum();
            if (ptr->getPDG() == apPDG_ || (isSimpSignal_ && ptr->getPDG() == 625)) {
                mother = ptr;
            } else if (ptr->getPDG() == 11 && ptr->getMomPDG() == signalMomPDG_) {
                trueEleP = ROOT::Math::PxPyPzEVector(lP.at(0), lP.at(1), lP.at(2), ptr->getEnergy());
                foundEle = true;
            } else if (ptr->getPDG() == -11 && ptr->getMomPDG() == signalMomPDG_) {
                truePosP = ROOT::Math::PxPyPzEVector(lP.at(0), lP.at(1), lP.at(2), ptr->getEnergy());
                foundPos = true;
            }
        }
        truth_epem_found = foundEle && foundPos;

        if (foundEle) {
            bus_.set("ele_truth_p", TVector3(trueEleP.Px(), trueEleP.Py(), trueEleP.Pz()));
            bus_.set("ele_truth_E", trueEleP.E());
        }
        if (foundPos) {
            bus_.set("pos_truth_p", TVector3(truePosP.Px(), truePosP.Py(), truePosP.Pz()));
            bus_.set("pos_truth_E", truePosP.E());
        }
        if (foundEle && foundPos) {
            bus_.set("true_vertex_invM", (trueEleP + truePosP).M());
            bus_.set("true_vertex_psum", trueEleP.P() + truePosP.P());
        }

        if (mother) {
            std::vector<double> vtxPos = mother->getVertexPosition();
            std::vector<double> endPos = mother->getEndPoint();
            double dx = endPos[0] - vtxPos[0];
            double dy = endPos[1] - vtxPos[1];
            double dz = endPos[2] - vtxPos[2];
            bus_.set("true_decay_len", std::sqrt(dx * dx + dy * dy + dz * dz));

            std::vector<double> mMom = mother->getMomentum();
            double mp = std::sqrt(mMom[0] * mMom[0] + mMom[1] * mMom[1] + mMom[2] * mMom[2]);
            if (mother->getMass() > 0) bus_.set("true_betagamma", mp / mother->getMass());
        }
    }
    event_cf_.apply("truth_epem_found", isData_ ? true : truth_epem_found);
    event_cf_.fill_nm1("truth_epem_found", truth_epem_found ? 1 : 0);

    // ===================== reco block =====================
    const auto& vtxs{bus_.get<std::vector<Vertex*>>(vtxColl_)};

    // Evaluate EVERY e-/e+ v0 and choose the BEST one = the v0 whose positron points closest
    // to an energetic cluster (smallest e+ dr). Also record whether ANY v0 has a matched
    // e+/e- (best-case efficiency) and what the FIRST v0 alone would have given.
    Vertex* chosen{nullptr};
    int chosen_i_ele{-1}, chosen_i_pos{-1};
    int n_v0 = 0;
    double best_pos_dr = 1e9;
    bool any_pos_matched = false, any_ele_matched = false;
    bool first_pos_matched = false, first_ele_matched = false, first_v0_seen = false;
    for (Vertex* vtx : vtxs) {
        int i_ele{-1}, i_pos{-1};
        for (int ipart = 0; ipart < vtx->getParticles().GetEntries(); ++ipart) {
            int pdg_id = ((Particle*)vtx->getParticles().At(ipart))->getPDG();
            if (pdg_id == 11)
                i_ele = ipart;
            else if (pdg_id == -11)
                i_pos = ipart;
        }
        if (i_ele < 0 || i_pos < 0) continue;
        n_v0++;

        Particle* e = dynamic_cast<Particle*>(vtx->getParticles().At(i_ele));
        Particle* p = dynamic_cast<Particle*>(vtx->getParticles().At(i_pos));
        auto eEcal = e->getTrack().getPositionAtEcal();
        auto pEcal = p->getTrack().getPositionAtEcal();
        double pdr, pE, pt, edr, eE, et;
        nearest_energetic(pEcal, pdr, pE, pt);
        nearest_energetic(eEcal, edr, eE, et);
        bool pmatch = (pdr < drMatch_);
        bool ematch = (edr < drMatch_);
        any_pos_matched = any_pos_matched || pmatch;
        any_ele_matched = any_ele_matched || ematch;
        if (not first_v0_seen) {
            first_v0_seen = true;
            first_pos_matched = pmatch;
            first_ele_matched = ematch;
        }
        if (pdr < best_pos_dr) {  // best v0 = positron closest to an energetic cluster
            best_pos_dr = pdr;
            chosen = vtx;
            chosen_i_ele = i_ele;
            chosen_i_pos = i_pos;
        }
    }
    has_v0 = (chosen != nullptr);

    // ---- track-level cluster match (decouple tracking vs vertexing) ----
    // Independent of any v0: does a standalone track (KalmanFullTracks) point at an energetic
    // cluster? If a positron TRACK matches but no v0 positron does, the loss is VERTEXING; if
    // no track matches either, the loss is TRACKING/acceptance.
    bool trk_pos_matched = false, trk_ele_matched = false;
    bool has_orphan_cluster = false;
    double orphan_clu_x = -9999., orphan_clu_y = -9999., orphan_clu_E = -9999.;
    if (bus_.has(trkColl_)) {
        const auto& tracks{bus_.get<std::vector<Track*>>(trkColl_)};
        for (Track* trk : tracks) {
            auto tEcal = trk->getPositionAtEcal();
            if (tEcal.size() < 2) continue;
            double dr, E, t;
            nearest_energetic(tEcal, dr, E, t);
            if (dr < drMatch_) {
                if (trk->getCharge() > 0) trk_pos_matched = true;
                else if (trk->getCharge() < 0) trk_ele_matched = true;
            }
        }

        // orphan energetic clusters: energetic cluster with NO track (any charge) within
        // drMatch_. Keep the highest-E orphan -> the lost (positron) cluster for the x,y map.
        for (const auto* cl : allClusters) {
            if (cl->getEnergy() <= eventClusterThresh_) continue;
            auto cp = cl->getPosition();
            double min_trk_dr = 9999.;
            for (Track* trk : tracks) {
                auto tE = trk->getPositionAtEcal();
                if (tE.size() < 2) continue;
                double dr = std::sqrt((tE[0] - cp[0]) * (tE[0] - cp[0]) +
                                      (tE[1] - cp[1]) * (tE[1] - cp[1]));
                if (dr < min_trk_dr) min_trk_dr = dr;
            }
            if (min_trk_dr > drMatch_) {  // no track points at this energetic cluster
                has_orphan_cluster = true;
                if (cl->getEnergy() > orphan_clu_E) {
                    orphan_clu_E = cl->getEnergy();
                    orphan_clu_x = cp[0];
                    orphan_clu_y = cp[1];
                }
            }
        }
    }
    // "tracking/acceptance" scenario for the positron: an energetic cluster exists but no
    // positron track points at any energetic cluster.
    bool pos_acceptance_loss = event_has_cluster && not trk_pos_matched;

    bus_.set("trk_pos_matched", trk_pos_matched);
    bus_.set("trk_ele_matched", trk_ele_matched);
    bus_.set("n_v0", n_v0);
    bus_.set("pos_acceptance_loss", pos_acceptance_loss);
    bus_.set("has_orphan_cluster", has_orphan_cluster);
    bus_.set<double>("orphan_clu_x", orphan_clu_x);
    bus_.set<double>("orphan_clu_y", orphan_clu_y);
    bus_.set<double>("orphan_clu_E", orphan_clu_E);

    if (has_v0) {
        Particle ele = *dynamic_cast<Particle*>(chosen->getParticles().At(chosen_i_ele));
        Particle pos = *dynamic_cast<Particle*>(chosen->getParticles().At(chosen_i_pos));
        Track ele_trk = ele.getTrack();
        Track pos_trk = pos.getTrack();

        TVector3 ele_p(ele_trk.getMomentum()[0], ele_trk.getMomentum()[1], ele_trk.getMomentum()[2]);
        TVector3 pos_p(pos_trk.getMomentum()[0], pos_trk.getMomentum()[1], pos_trk.getMomentum()[2]);
        bus_.set("ele_track_p", ele_p);
        bus_.set("pos_track_p", pos_p);

        auto eleEcal = ele_trk.getPositionAtEcal();
        auto posEcal = pos_trk.getPositionAtEcal();
        bus_.set<double>("ele_ecal_x", eleEcal[0]);
        bus_.set<double>("ele_ecal_y", eleEcal[1]);
        bus_.set<double>("ele_ecal_z", eleEcal[2]);
        bus_.set<double>("pos_ecal_x", posEcal[0]);
        bus_.set<double>("pos_ecal_y", posEcal[1]);
        bus_.set<double>("pos_ecal_z", posEcal[2]);

        // fiducial: use AnaHelpers::IsECalFiducial on the track projection at ECal
        auto fiducial_at = [&](const std::vector<double>& xyz) -> bool {
            float p[3] = {(float)xyz[0], (float)xyz[1], (float)xyz[2]};
            CalCluster c;
            c.setPosition(p);
            return _ah->IsECalFiducial(&c);
        };
        ele_fiducial = fiducial_at(eleEcal);
        pos_fiducial = fiducial_at(posEcal);
        both_tracks_fiducial = ele_fiducial && pos_fiducial;

        // associated clusters: a missing cluster has the sentinel energy (-9999)
        CalCluster ele_clu = ele.getCluster();
        CalCluster pos_clu = pos.getCluster();
        ele_has_cluster = ele_clu.getEnergy() > clusterEnergyThresh_;
        pos_has_cluster = pos_clu.getEnergy() > clusterEnergyThresh_;
        pos_matched_cluster = pos_clu.getEnergy() > eventClusterThresh_;
        pos_clu_E_dbg = pos_clu.getEnergy();
        if (ele_has_cluster) {
            bus_.set<double>("ele_clu_E", ele_clu.getEnergy());
            bus_.set<double>("ele_clu_x", ele_clu.getPosition()[0]);
            bus_.set<double>("ele_clu_y", ele_clu.getPosition()[1]);
            bus_.set<double>("ele_clu_time", ele_clu.getTime() - calTimeOffset_);
            bus_.set("ele_clu_nhits", ele_clu.getNHits());
        }
        if (pos_has_cluster) {
            bus_.set<double>("pos_clu_E", pos_clu.getEnergy());
            bus_.set<double>("pos_clu_x", pos_clu.getPosition()[0]);
            bus_.set<double>("pos_clu_y", pos_clu.getPosition()[1]);
            bus_.set<double>("pos_clu_time", pos_clu.getTime() - calTimeOffset_);
            bus_.set("pos_clu_nhits", pos_clu.getNHits());
        }

        // dr-match of the BEST v0's tracks to the nearest energetic cluster (for dbg/printout)
        double pos_dr, pos_dr_E, pos_dr_t;
        double ele_dr, ele_dr_E, ele_dr_t;
        nearest_energetic(posEcal, pos_dr, pos_dr_E, pos_dr_t);
        nearest_energetic(eleEcal, ele_dr, ele_dr_E, ele_dr_t);
        bus_.set<double>("pos_nearest_clu_dr", pos_dr);
        bus_.set<double>("pos_nearest_clu_E", pos_dr_E);
        bus_.set<double>("pos_nearest_clu_time", pos_dr_t);
        pos_dr_matched = (pos_dr < drMatch_);
        ele_dr_matched = (ele_dr < drMatch_);
        pos_nearest_dr_dbg = pos_dr;
        pos_dr_E_dbg = pos_dr_E;
        ele_nearest_dr_dbg = ele_dr;
        ele_dr_E_dbg = ele_dr_E;
    }

    bus_.set("truth_epem_found", truth_epem_found);
    bus_.set("has_v0", has_v0);
    bus_.set("ele_fiducial", ele_fiducial);
    bus_.set("pos_fiducial", pos_fiducial);
    bus_.set("both_tracks_fiducial", both_tracks_fiducial);
    bus_.set("ele_has_cluster", ele_has_cluster);
    bus_.set("pos_has_cluster", pos_has_cluster);

    event_cf_.apply("has_v0", has_v0);
    event_cf_.fill_nm1("has_v0", has_v0 ? 1 : 0);
    event_cf_.apply("both_tracks_fiducial", both_tracks_fiducial);
    event_cf_.fill_nm1("both_tracks_fiducial", both_tracks_fiducial ? 1 : 0);
    event_cf_.apply("pos_has_cluster", pos_has_cluster);
    event_cf_.fill_nm1("pos_has_cluster", pos_has_cluster ? 1 : 0);

    if (has_v0) n_has_v0_++;
    if (n_v0 > 1) n_multi_v0_++;
    if (first_pos_matched) n_first_pos_matched_++;
    if (first_ele_matched) n_first_ele_matched_++;
    if (any_pos_matched) n_any_pos_matched_++;
    if (any_ele_matched) n_any_ele_matched_++;
    if (trk_pos_matched) n_trk_pos_matched_++;
    if (trk_ele_matched) n_trk_ele_matched_++;
    if (trk_pos_matched && not any_pos_matched) n_trkpos_no_v0pos_++;

    // per-event terminal dump (first 40 triggered events) so we can inspect by eye.
    // Compares v0-level vs track-level cluster match to separate tracking from vertexing.
    if (n_printed_ < 40) {
        printf("[cluseff] run %d evt %d | nClu=%2d || nV0=%d hasV0=%d "
               "|| v0: pos=%d ele=%d || trk: pos=%d ele=%d%s\n",
               eh.getRunNumber(), eh.getEventNumber(), n_clusters,
               n_v0, has_v0 ? 1 : 0,
               any_pos_matched ? 1 : 0, any_ele_matched ? 1 : 0,
               trk_pos_matched ? 1 : 0, trk_ele_matched ? 1 : 0,
               (trk_pos_matched && not any_pos_matched) ? "  <-- e+ track but no v0!" : "");
        n_printed_++;
    }

    // one row per triggered signal event (sentinels where reco objects are absent)
    output_tree_->Fill();
    return true;
}

void ClusterEfficiency2021::finalize() {
    auto pct = [](long num, long den) { return den > 0 ? 100.0 * num / den : 0.0; };
    std::cout << "\n================ ClusterEfficiency2021 summary ================\n"
              << "  energetic-cluster threshold     : " << eventClusterThresh_ << " GeV\n"
              << "  singles2/3 triggered events     : " << n_triggered_ << "\n"
              << "  Q1: cluster > thr ANYWHERE      : " << n_event_has_cluster_
              << "  (" << pct(n_event_has_cluster_, n_triggered_) << "% of triggered)\n"
              << "      events with >=1 e+e- v0     : " << n_has_v0_
              << "  (" << pct(n_has_v0_, n_triggered_) << "% of triggered)\n"
              << "      ... of which have >1 v0     : " << n_multi_v0_
              << "  (" << pct(n_multi_v0_, n_has_v0_) << "% of v0 events)\n"
              << "  dr match radius                 : " << drMatch_ << " mm\n"
              << "  --- FIRST v0 only (old behavior) ---\n"
              << "  e+ dr-matched cluster > thr     : " << n_first_pos_matched_
              << "  (" << pct(n_first_pos_matched_, n_has_v0_) << "% of v0)\n"
              << "  e- dr-matched cluster > thr     : " << n_first_ele_matched_
              << "  (" << pct(n_first_ele_matched_, n_has_v0_) << "% of v0)\n"
              << "  --- ANY v0 (best case), as % of TRIGGERED ---\n"
              << "  e+ dr-matched in some v0        : " << n_any_pos_matched_
              << "  (" << pct(n_any_pos_matched_, n_triggered_) << "% of triggered, "
              << pct(n_any_pos_matched_, n_has_v0_) << "% of v0)\n"
              << "  e- dr-matched in some v0        : " << n_any_ele_matched_
              << "  (" << pct(n_any_ele_matched_, n_triggered_) << "% of triggered, "
              << pct(n_any_ele_matched_, n_has_v0_) << "% of v0)\n"
              << "  --- TRACK level (KalmanFullTracks, NO v0 required), % of TRIGGERED ---\n"
              << "  e+ track dr-matched cluster     : " << n_trk_pos_matched_
              << "  (" << pct(n_trk_pos_matched_, n_triggered_) << "% of triggered)\n"
              << "  e- track dr-matched cluster     : " << n_trk_ele_matched_
              << "  (" << pct(n_trk_ele_matched_, n_triggered_) << "% of triggered)\n"
              << "  e+ track matched but NO v0 e+   : " << n_trkpos_no_v0pos_
              << "  (" << pct(n_trkpos_no_v0pos_, n_triggered_) << "% of triggered)"
              << "  <- VERTEXING loss\n"
              << "===============================================================\n" << std::endl;

    outF_->cd();
    output_tree_->Write();
    event_cf_.save();
    outF_->Close();
}

DECLARE_PROCESSOR(ClusterEfficiency2021);
