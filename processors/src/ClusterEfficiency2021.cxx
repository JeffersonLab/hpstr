/*
 * @file ClusterEfficiency2021.cxx
 * @brief Standalone study of positron-cluster (and v0) reconstruction efficiency
 *        for 2021 displaced A'/SIMP signal MC.
 * @author Matthew Gignac
 * @date May 2026
 */

#include "ClusterEfficiency2021.h"

#include <cmath>

void ClusterEfficiency2021::configure(const ParameterSet& parameters) {
    isData_ = parameters.getInteger("isData", 0) != 0;
    isApSignal_ = parameters.getInteger("isApSignal", 0) != 0;
    isSimpSignal_ = parameters.getInteger("isSimpSignal", 0) != 0;
    apPDG_ = parameters.getInteger("apPDG", apPDG_);
    calTimeOffset_ = parameters.getDouble("calTimeOffset", 0.0);
    clusterEnergyThresh_ = parameters.getDouble("clusterEnergyThresh", 0.0);
    debug_ = parameters.getInteger("debug", 0) != 0;

    auto vtxColl = parameters.getString("vtxCollection", "");
    if (not vtxColl.empty()) vtxColl_ = vtxColl;

    // mother PDG of the signal e+/e-: 625 for SIMP vector decay, apPDG_ for A'
    signalMomPDG_ = isSimpSignal_ ? 625 : apPDG_;

    std::cout << "[ClusterEfficiency2021] isData=" << isData_ << " isApSignal=" << isApSignal_
              << " isSimpSignal=" << isSimpSignal_ << " signalMomPDG=" << signalMomPDG_
              << " vtxColl=" << vtxColl_ << std::endl;
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
    const auto& allClusters{bus_.get<std::vector<CalCluster*>>("RecoEcalClusters")};

    // pick a v0: prefer one whose electron/positron tracks truth-match e-/e+, else the first one
    Vertex* chosen{nullptr};
    int chosen_i_ele{-1}, chosen_i_pos{-1};
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
        if (chosen == nullptr) {
            chosen = vtx;
            chosen_i_ele = i_ele;
            chosen_i_pos = i_pos;
        }
        if (mcParticles) {
            Particle* e = dynamic_cast<Particle*>(vtx->getParticles().At(i_ele));
            Particle* p = dynamic_cast<Particle*>(vtx->getParticles().At(i_pos));
            Track et = e->getTrack();
            Track pt = p->getTrack();
            if (utils::getTruthPDG(et, mcParticles) == 11 && utils::getTruthPDG(pt, mcParticles) == -11) {
                chosen = vtx;
                chosen_i_ele = i_ele;
                chosen_i_pos = i_pos;
                break;
            }
        }
    }
    has_v0 = (chosen != nullptr);

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

        // standalone-cluster cross-check: nearest RecoEcalCluster to the positron track
        // projection at ECal. Reveals cases where a cluster exists but was not associated.
        double min_dr = 9999.0, min_dr_E = -9999.0, min_dr_t = -9999.0;
        for (const auto* cl : allClusters) {
            auto clPos = cl->getPosition();
            double dx = clPos[0] - posEcal[0];
            double dy = clPos[1] - posEcal[1];
            double dr = std::sqrt(dx * dx + dy * dy);
            if (dr < min_dr) {
                min_dr = dr;
                min_dr_E = cl->getEnergy();
                min_dr_t = cl->getTime() - calTimeOffset_;
            }
        }
        if (not allClusters.empty()) {
            bus_.set<double>("pos_nearest_clu_dr", min_dr);
            bus_.set<double>("pos_nearest_clu_E", min_dr_E);
            bus_.set<double>("pos_nearest_clu_time", min_dr_t);
        }
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

    // one row per triggered signal event (sentinels where reco objects are absent)
    output_tree_->Fill();
    return true;
}

void ClusterEfficiency2021::finalize() {
    outF_->cd();
    output_tree_->Write();
    event_cf_.save();
    outF_->Close();
}

DECLARE_PROCESSOR(ClusterEfficiency2021);
