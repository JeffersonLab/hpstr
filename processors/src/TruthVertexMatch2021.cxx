/*
 * @file TruthVertexMatch2021.cxx
 * @brief Truth-A' -> reconstructed-vertex matching study for 2021 displaced A'/SIMP signal MC.
 * @author Matthew Gignac
 * @date June 2026
 */

#include "TruthVertexMatch2021.h"

#include <cmath>
#include <set>

void TruthVertexMatch2021::configure(const ParameterSet& parameters) {
    isData_ = parameters.getInteger("isData", 0) != 0;
    isApSignal_ = parameters.getInteger("isApSignal", 0) != 0;
    isSimpSignal_ = parameters.getInteger("isSimpSignal", 0) != 0;
    apPDG_ = parameters.getInteger("apPDG", apPDG_);
    calTimeOffset_ = parameters.getDouble("calTimeOffset", 0.0);
    vertexMatchZ_ = parameters.getDouble("vertexMatchZ", vertexMatchZ_);
    massSelect_ = parameters.getDouble("massSelect", massSelect_);
    massWindow_ = parameters.getDouble("massWindow", massWindow_);
    vtxZMin_ = parameters.getDouble("vtxZMin", vtxZMin_);
    vtxZMax_ = parameters.getDouble("vtxZMax", vtxZMax_);
    eleMomCut_ = parameters.getDouble("eleMomCut", eleMomCut_);
    eleClusEMin_ = parameters.getDouble("eleClusEMin", eleClusEMin_);
    debug_ = parameters.getInteger("debug", 0) != 0;

    auto vtxColl = parameters.getString("vtxCollection", "");
    if (not vtxColl.empty()) vtxColl_ = vtxColl;
    auto hitColl = parameters.getString("hitCollection", "");
    if (not hitColl.empty()) hitColl_ = hitColl;

    // mother PDG of the signal e+/e-: 625 for SIMP vector decay, apPDG_ for A'
    signalMomPDG_ = isSimpSignal_ ? 625 : apPDG_;

    std::cout << "[TruthVertexMatch2021] isData=" << isData_ << " isApSignal=" << isApSignal_
              << " isSimpSignal=" << isSimpSignal_ << " signalMomPDG=" << signalMomPDG_
              << " vtxColl=" << vtxColl_;
    if (massSelect_ > 0.)
        std::cout << " massSelect=" << massSelect_ << " +/- " << massWindow_ << " GeV";
    if (vtxZMin_ > -1.0e8 || vtxZMax_ < 1.0e8)
        std::cout << " vtxZ in [" << vtxZMin_ << ", " << vtxZMax_ << "] mm";
    std::cout << " eleMomCut=" << eleMomCut_ << " GeV eleClusEMin=" << eleClusEMin_ << " GeV" << std::endl;
}

void TruthVertexMatch2021::initialize(TTree* tree) {
    _ah = std::make_shared<AnaHelpers>();

    bus_.board_input<EventHeader>(tree, "EventHeader");
    bus_.board_input<TSData>(tree, "TSBank");
    if (not trkColl_.empty()) bus_.board_input<std::vector<Track*>>(tree, trkColl_);
    bus_.board_input<std::vector<Vertex*>>(tree, vtxColl_);
    bus_.board_input<std::vector<CalCluster*>>(tree, "RecoEcalClusters");
    if (not isData_ and not mcColl_.empty()) bus_.board_input<std::vector<MCParticle*>>(tree, mcColl_);

    // Truth-hit collection is only present when the upstream recoTuple_cfg.py run had
    // -sh/--truthHits 1 enabled. board_input() throws if the branch is missing, so check for the
    // branch on the tree first and simply skip boarding it if absent -- n_truth_hits_ele/pos will
    // report the -1 sentinel in that case rather than the processor crashing or aborting the run.
    hasTruthHits_ = (not isData_) && (tree->GetBranch(hitColl_.c_str()) != nullptr);
    if (hasTruthHits_) {
        bus_.board_input<std::vector<TrackerHit*>>(tree, hitColl_);
    } else if (not isData_) {
        std::cout << "[TruthVertexMatch2021] NOTE: truth-hit collection '" << hitColl_
                  << "' not found in input tree -- n_truth_hits_ele/pos will use the -1 sentinel."
                  << std::endl;
    }

    // quick at-a-glance efficiency chain (in addition to the tree)
    event_cf_.add("truth_signal_found", 2, -0.5, 1.5);
    event_cf_.add("single_truth_signal", 2, -0.5, 1.5);
    event_cf_.add("at_least_one_v0", 10, 0.0, 10.0);
    event_cf_.add("v0_within_z", 50, 0.0, 50.0);
    event_cf_.init();
    event_cf_.set_label_names({"readout", "truth A' (or VD) found", "exactly one truth signal",
                               "N_{v0}^{raw} #geq 1", "v0 within z of truth"});
}

void TruthVertexMatch2021::setFile(TFile* out_file) {
    Processor::setFile(out_file);

    output_tree_ = std::make_unique<TTree>("truth_vertex_match", "Truth A' -> reco vertex matching", 99, out_file);

    bus_.board_output<double>(output_tree_.get(), "weight");
    bus_.board_output<int>(output_tree_.get(), "run");
    bus_.board_output<int>(output_tree_.get(), "event");

    // truth signal-mother (A' / SIMP vector) quantities
    bus_.board_output<int>(output_tree_.get(), "n_signal");        // number of truth signal mothers found
    bus_.board_output<bool>(output_tree_.get(), "truth_signal_found");
    bus_.board_output<TVector3>(output_tree_.get(), "true_ap_p");
    for (const auto& name : {"true_ap_E", "true_ap_mass", "true_betagamma", "true_decay_len"}) {
        bus_.board_output<double>(output_tree_.get(), name);
    }
    // truth decay vertex (A' endpoint = where e+e- are produced) and production vertex
    for (const auto& name : {"true_vtx_x", "true_vtx_y", "true_vtx_z",
                             "true_prod_x", "true_prod_y", "true_prod_z"}) {
        bus_.board_output<double>(output_tree_.get(), name);
    }

    // truth e+/e- decay products
    bus_.board_output<TVector3>(output_tree_.get(), "true_ele_p");
    bus_.board_output<TVector3>(output_tree_.get(), "true_pos_p");
    for (const auto& name : {"true_ele_E", "true_pos_E", "true_vertex_invM", "true_vertex_psum"}) {
        bus_.board_output<double>(output_tree_.get(), name);
    }
    // truth e+/e- geometry/kinematics (production momentum): pt, pz, polar angle, and the
    // bend-plane (px/pz) and vertical (py/pz ~ tanLambda) slopes -> for studying acceptance.
    for (const auto& name : {"true_ele_pt", "true_ele_pz", "true_ele_theta", "true_ele_slopeX", "true_ele_slopeY",
                             "true_pos_pt", "true_pos_pz", "true_pos_theta", "true_pos_slopeX", "true_pos_slopeY"}) {
        bus_.board_output<double>(output_tree_.get(), name);
    }

    // reconstructed track inventory (KalmanFullTracks, no truth match)
    bus_.board_output<int>(output_tree_.get(), "n_trk_pos");  // # positive (e+) tracks in event
    bus_.board_output<int>(output_tree_.get(), "n_trk_ele");  // # negative (e-) tracks in event

    // reconstructed-vertex matching (step 2)
    bus_.board_output<int>(output_tree_.get(), "n_v0");       // number of reco v0s (no selection)
    bus_.board_output<bool>(output_tree_.get(), "has_v0");    // n_v0 >= 1
    bus_.board_output<bool>(output_tree_.get(), "matched_z"); // >=1 v0 within vertexMatchZ_ in z of truth
    // properties of the closest-in-z v0 (sentinels when no v0):
    for (const auto& name : {"best_dz", "best_vtx_x", "best_vtx_y", "best_vtx_z", "best_vtx_chi2"}) {
        bus_.board_output<double>(output_tree_.get(), name);
    }

    // track-based vertex matching (step 3): matches the v0's two constituent tracks to the truth
    // e-/e+ by majority-hit MCParticle ID, independent of the reconstructed vertex z.
    bus_.board_output<int>(output_tree_.get(), "n_v0_trk_matched");  // # v0s that are track-matched
    bus_.board_output<bool>(output_tree_.get(), "matched_trk");      // n_v0_trk_matched >= 1
    bus_.board_output<bool>(output_tree_.get(), "matched_z_or_trk");
    bus_.board_output<bool>(output_tree_.get(), "matched_z_and_trk");
    // z offset (reco - truth) of the closest-in-z track-matched v0 (sentinel when none):
    bus_.board_output<double>(output_tree_.get(), "best_trk_dz");
    // single-leg track matches (no requirement on the other leg): how often does a v0 capture
    // just the truth e- or just the truth e+, vs. both (matched_trk above)?
    bus_.board_output<bool>(output_tree_.get(), "matched_trk_ele");
    bus_.board_output<bool>(output_tree_.get(), "matched_trk_pos");

    // Truth-hit counts: number of 2D SVT hits (RotatedHelicalTrackHits, tagged via
    // SVTTrueHitRelations) whose majority-contributing MCParticle is the truth e-/e+, i.e. real
    // detector hits the true signal track left behind, independent of whether tracking/vertexing
    // succeeded. Lets us bin v0-finding efficiency vs. the number of truth hits available.
    // has_truth_hits distinguishes "0 real hits" from "collection unavailable in this file"
    // (n_truth_hits_ele/pos report the -1 sentinel in the latter case).
    bus_.board_output<bool>(output_tree_.get(), "has_truth_hits");
    bus_.board_output<int>(output_tree_.get(), "n_truth_hits_ele");
    bus_.board_output<int>(output_tree_.get(), "n_truth_hits_pos");
    // Unique-layer versions of the above: TrackerHit::getLayer() distinguishes axial vs. stereo
    // sub-layers (0-13 for the 7-layer SVT), and the same physical hit can occasionally show up
    // twice (split-cluster artifacts) -- both inflate a raw hit count without adding real tracking
    // information. n_truth_layers_ele/pos count DISTINCT layer_ values instead, so they saturate
    // at the number of sub-layers the true track actually crossed.
    bus_.board_output<int>(output_tree_.get(), "n_truth_layers_ele");
    bus_.board_output<int>(output_tree_.get(), "n_truth_layers_pos");

    // ECal cluster info for the truth-matched e-/e+ legs of the "best" track-matched v0, where
    // "best" is picked by cluster-completeness tier first and z-closeness as a tiebreaker (NOT
    // simply the closest-in-z v0 like best_trk_dz -- see the tiered selection in process(), which
    // guarantees these branches describe a v0 actually consistent with matched_trk_clus_both /
    // matched_trk_clus_pos_only when those are true). utils::buildParticle only calls setCluster()
    // when the LCIO ReconstructedParticle had an associated cluster, so has_*_clus is a real
    // "was this leg cluster-matched" flag. NOTE: presence is tested via getEnergy() > sentinel,
    // NOT getNHits() -- utils::buildCalCluster never calls addHit(), so getNHits() is always 0
    // even for a real cluster; energy/position/time ARE populated.
    bus_.board_output<bool>(output_tree_.get(), "has_ele_clus");
    bus_.board_output<bool>(output_tree_.get(), "has_pos_clus");
    for (const auto& name : {"ele_clus_E", "ele_clus_x", "ele_clus_y", "ele_clus_z", "ele_clus_t",
                             "pos_clus_E", "pos_clus_x", "pos_clus_y", "pos_clus_z", "pos_clus_t"}) {
        bus_.board_output<double>(output_tree_.get(), name);
    }
    // Reco track momentum (Kalman fit, |p|) of the same two legs, for track-p vs cluster-E
    // (E/p-style) diagnostics.
    bus_.board_output<double>(output_tree_.get(), "ele_trk_p");
    bus_.board_output<double>(output_tree_.get(), "pos_trk_p");
    // Tracker hit count (Track::getTrackerHitCount(), max 14 for a full 7-layer Kalman track) of
    // the same two legs, for track-quality-vs-momentum studies.
    bus_.board_output<int>(output_tree_.get(), "ele_trk_nhits");
    bus_.board_output<int>(output_tree_.get(), "pos_trk_nhits");

    // Cluster-aware track matching: quantifies what relaxing the electron ECal-cluster
    // requirement gains over requiring clusters on BOTH legs (see finalize() for the headline
    // acceptance-gain number). All are event-level ORs over v0s, same convention as matched_trk*.
    bus_.board_output<bool>(output_tree_.get(), "matched_trk_ele_clus");     // >=1 v0: e- track has a cluster (any e+)
    bus_.board_output<bool>(output_tree_.get(), "matched_trk_pos_clus");     // >=1 v0: e+ track has a cluster (any e-)
    bus_.board_output<bool>(output_tree_.get(), "matched_trk_clus_both");      // >=1 v0: both tracks + both clusters
    bus_.board_output<bool>(output_tree_.get(), "matched_trk_clus_pos_only"); // >=1 v0: both tracks + e+ cluster only
    bus_.board_output<bool>(output_tree_.get(), "matched_trk_clus_ele_only"); // >=1 v0: both tracks + e- cluster only
}

bool TruthVertexMatch2021::process(IEvent*) {
    // Reset output branches so conditionally-filled ones do not carry over the
    // previous event's value into this event's entry.
    bus_.clear_outputs();

    const auto& eh{bus_.get<EventHeader>("EventHeader")};

    event_cf_.begin_event();

    // ----- defaults (sentinels) -----
    bus_.set("weight", 1.);
    bus_.set("run", eh.getRunNumber());
    bus_.set("event", eh.getEventNumber());
    bus_.set("n_signal", 0);
    bus_.set("truth_signal_found", false);
    bus_.set("true_ap_p", TVector3(0., 0., 0.));
    bus_.set("true_ele_p", TVector3(0., 0., 0.));
    bus_.set("true_pos_p", TVector3(0., 0., 0.));
    for (const auto& name : {"true_ap_E", "true_ap_mass", "true_betagamma", "true_decay_len",
                             "true_vtx_x", "true_vtx_y", "true_vtx_z",
                             "true_prod_x", "true_prod_y", "true_prod_z",
                             "true_ele_E", "true_pos_E", "true_vertex_invM", "true_vertex_psum",
                             "true_ele_pt", "true_ele_pz", "true_ele_theta", "true_ele_slopeX", "true_ele_slopeY",
                             "true_pos_pt", "true_pos_pz", "true_pos_theta", "true_pos_slopeX", "true_pos_slopeY"}) {
        bus_.set<double>(name, -9999.);
    }
    bus_.set("n_trk_pos", 0);
    bus_.set("n_trk_ele", 0);
    bus_.set("n_v0", 0);
    bus_.set("has_v0", false);
    bus_.set("matched_z", false);
    for (const auto& name : {"best_dz", "best_vtx_x", "best_vtx_y", "best_vtx_z", "best_vtx_chi2"}) {
        bus_.set<double>(name, -9999.);
    }
    bus_.set("n_v0_trk_matched", 0);
    bus_.set("matched_trk", false);
    bus_.set("matched_z_or_trk", false);
    bus_.set("matched_z_and_trk", false);
    bus_.set<double>("best_trk_dz", -9999.);
    bus_.set("matched_trk_ele", false);
    bus_.set("matched_trk_pos", false);
    bus_.set("has_truth_hits", hasTruthHits_);
    bus_.set<int>("n_truth_hits_ele", -1);
    bus_.set<int>("n_truth_hits_pos", -1);
    bus_.set<int>("n_truth_layers_ele", -1);
    bus_.set<int>("n_truth_layers_pos", -1);
    bus_.set("has_ele_clus", false);
    bus_.set("has_pos_clus", false);
    for (const auto& name : {"ele_clus_E", "ele_clus_x", "ele_clus_y", "ele_clus_z", "ele_clus_t",
                             "pos_clus_E", "pos_clus_x", "pos_clus_y", "pos_clus_z", "pos_clus_t",
                             "ele_trk_p", "pos_trk_p"}) {
        bus_.set<double>(name, -9999.);
    }
    bus_.set<int>("ele_trk_nhits", 0);
    bus_.set<int>("pos_trk_nhits", 0);
    bus_.set("matched_trk_ele_clus", false);
    bus_.set("matched_trk_pos_clus", false);
    bus_.set("matched_trk_clus_both", false);
    bus_.set("matched_trk_clus_pos_only", false);
    bus_.set("matched_trk_clus_ele_only", false);

    if (not bus_.has(mcColl_)) {
        // no truth available (e.g. data): nothing to do for this study
        output_tree_->Fill();
        return true;
    }

    // ===================== truth block (mirrors PreselectAndCategorize2021) =====================
    // Find the signal mother (A' = apPDG_, or SIMP vector = 625) and its e+/e- decay products,
    // identified by PDG and mother-PDG. n_signal counts the mothers so we can drop the very rare
    // double-signal / no-signal beam-overlay events downstream.
    const auto& mcParticles{bus_.get<std::vector<MCParticle*>>(mcColl_)};
    MCParticle* mother{nullptr};
    int n_signal{0};
    double true_vtx_z = -9999.;  // truth decay-vertex z (A' endpoint), used for reco matching
    ROOT::Math::PxPyPzEVector trueEleP, truePosP;
    bool foundEle{false}, foundPos{false};
    // LCIO IDs of the truth e-/e+, used for track-based vertex matching (independent of z)
    int trueEleID{-1}, truePosID{-1};
    for (MCParticle* ptr : mcParticles) {
        std::vector<double> lP = ptr->getMomentum();
        if (ptr->getPDG() == apPDG_ || (isSimpSignal_ && ptr->getPDG() == 625)) {
            n_signal++;
            mother = ptr;
        } else if (ptr->getPDG() == 11 && ptr->getMomPDG() == signalMomPDG_) {
            trueEleP = ROOT::Math::PxPyPzEVector(lP.at(0), lP.at(1), lP.at(2), ptr->getEnergy());
            trueEleID = ptr->getID();
            foundEle = true;
        } else if (ptr->getPDG() == -11 && ptr->getMomPDG() == signalMomPDG_) {
            truePosP = ROOT::Math::PxPyPzEVector(lP.at(0), lP.at(1), lP.at(2), ptr->getEnergy());
            truePosID = ptr->getID();
            foundPos = true;
        }
    }

    // Truth-hit counts: tally every 2D SVT hit by its majority... actually by ALL contributing
    // MCParticle IDs (a hit can list more than one via addMCPartID in Tracker2DHitProcessor when
    // multiple raw strip hits/particles feed the same 2D hit), then look up the counts for the
    // specific truth e-/e+ found above. Building the full per-MCParticle map (rather than only
    // counting hits matching trueEleID/truePosID as we go) keeps this generic across all truth
    // particles in the event, not just the two signal legs.
    if (hasTruthHits_) {
        const auto& truthHits{bus_.get<std::vector<TrackerHit*>>(hitColl_)};
        std::map<int, int> nHitsByMcpID;
        std::map<int, std::set<int>> layersByMcpID;
        for (TrackerHit* hit : truthHits) {
            for (int mcpID : hit->getMCPartIDs()) {
                nHitsByMcpID[mcpID]++;
                layersByMcpID[mcpID].insert(hit->getLayer());
            }
        }
        if (trueEleID >= 0) {
            bus_.set<int>("n_truth_hits_ele", nHitsByMcpID[trueEleID]);
            bus_.set<int>("n_truth_layers_ele", static_cast<int>(layersByMcpID[trueEleID].size()));
        }
        if (truePosID >= 0) {
            bus_.set<int>("n_truth_hits_pos", nHitsByMcpID[truePosID]);
            bus_.set<int>("n_truth_layers_pos", static_cast<int>(layersByMcpID[truePosID].size()));
        }
    }

    // Optional truth e+e- invariant-mass window (slices the rad continuum to a fixed mass so it
    // is a mass-matched control). Disabled when massSelect_ < 0.
    bool mass_ok = true;
    if (massSelect_ > 0.) {
        double invM = (foundEle && foundPos) ? (trueEleP + truePosP).M() : -1.;
        mass_ok = (invM > 0.) && (std::fabs(invM - massSelect_) < massWindow_);
    }

    // Optional truth decay-vertex z fiducial window (apply BEFORE any acceptance evaluation).
    bool zfid_ok = true;
    if (mother) {
        double decay_z = mother->getEndPoint().at(2);
        zfid_ok = (decay_z >= vtxZMin_ && decay_z <= vtxZMax_);
    }

    bool truth_signal_found = (n_signal > 0) && mass_ok && zfid_ok;
    bus_.set("n_signal", n_signal);
    bus_.set("truth_signal_found", truth_signal_found);

    event_cf_.apply("truth_signal_found", truth_signal_found);
    event_cf_.fill_nm1("truth_signal_found", truth_signal_found ? 1 : 0);
    event_cf_.apply("single_truth_signal", n_signal == 1);
    event_cf_.fill_nm1("single_truth_signal", n_signal);

    if (foundEle) {
        bus_.set("true_ele_p", TVector3(trueEleP.Px(), trueEleP.Py(), trueEleP.Pz()));
        bus_.set<double>("true_ele_E", trueEleP.E());
        bus_.set<double>("true_ele_pt", trueEleP.Pt());
        bus_.set<double>("true_ele_pz", trueEleP.Pz());
        bus_.set<double>("true_ele_theta", trueEleP.Theta());
        bus_.set<double>("true_ele_slopeX", trueEleP.Pz() != 0 ? trueEleP.Px() / trueEleP.Pz() : -9999.);
        bus_.set<double>("true_ele_slopeY", trueEleP.Pz() != 0 ? trueEleP.Py() / trueEleP.Pz() : -9999.);
    }
    if (foundPos) {
        bus_.set("true_pos_p", TVector3(truePosP.Px(), truePosP.Py(), truePosP.Pz()));
        bus_.set<double>("true_pos_E", truePosP.E());
        bus_.set<double>("true_pos_pt", truePosP.Pt());
        bus_.set<double>("true_pos_pz", truePosP.Pz());
        bus_.set<double>("true_pos_theta", truePosP.Theta());
        bus_.set<double>("true_pos_slopeX", truePosP.Pz() != 0 ? truePosP.Px() / truePosP.Pz() : -9999.);
        bus_.set<double>("true_pos_slopeY", truePosP.Pz() != 0 ? truePosP.Py() / truePosP.Pz() : -9999.);
    }
    if (foundEle && foundPos) {
        bus_.set<double>("true_vertex_invM", (trueEleP + truePosP).M());
        bus_.set<double>("true_vertex_psum", trueEleP.P() + truePosP.P());
    }

    if (mother) {
        std::vector<double> prodPos = mother->getVertexPosition();  // where the A' is produced
        std::vector<double> endPos = mother->getEndPoint();         // where the A' decays -> e+e- vertex
        bus_.set<double>("true_prod_x", prodPos[0]);
        bus_.set<double>("true_prod_y", prodPos[1]);
        bus_.set<double>("true_prod_z", prodPos[2]);
        bus_.set<double>("true_vtx_x", endPos[0]);
        bus_.set<double>("true_vtx_y", endPos[1]);
        bus_.set<double>("true_vtx_z", endPos[2]);
        true_vtx_z = endPos[2];

        // NOTE: for the A' MCParticle getVertexPosition()==getEndPoint() (both store the DECAY
        // vertex), so end-prod is identically 0. The decay length is the decay-vertex distance
        // from the target (~origin). (The same end-prod formula in PreselectAndCategorize2021
        // likewise yields true_decay_len==0.)
        bus_.set<double>("true_decay_len",
                         std::sqrt(endPos[0] * endPos[0] + endPos[1] * endPos[1] + endPos[2] * endPos[2]));

        std::vector<double> mMom = mother->getMomentum();
        double mp = std::sqrt(mMom[0] * mMom[0] + mMom[1] * mMom[1] + mMom[2] * mMom[2]);
        bus_.set("true_ap_p", TVector3(mMom[0], mMom[1], mMom[2]));
        bus_.set<double>("true_ap_E", mother->getEnergy());
        bus_.set<double>("true_ap_mass", mother->getMass());
        if (mother->getMass() > 0) bus_.set<double>("true_betagamma", mp / mother->getMass());
    }

    // ===================== reco-vertex matching (step 2) =====================
    // Count reconstructed v0s (NO selection) and find the one closest in z to the truth vertex.
    const auto& vtxs{bus_.get<std::vector<Vertex*>>(vtxColl_)};
    int n_v0 = static_cast<int>(vtxs.size());
    bool has_v0 = (n_v0 >= 1);

    // Majority-hit truth match of a track (see utils::getTruthPDG): returns the LCIO ID of the
    // MCParticle contributing the most hits to this track, or -1 if it has no truth-linked hits.
    auto majorityMcpID = [](Track& trk) -> int {
        std::map<int, int> count_per_id;
        for (const auto& hit : trk.getMcpHits()) count_per_id[hit.second]++;
        int best_id{-1}, best_n{0};
        for (const auto& [id, n] : count_per_id) {
            if (n > best_n) {
                best_n = n;
                best_id = id;
            }
        }
        return best_id;
    };

    double best_abs_dz = 1e9, best_dz = -9999.;
    Vertex* best_vtx = nullptr;
    int n_v0_trk_matched = 0;
    int n_v0_ele_trk_matched = 0, n_v0_pos_trk_matched = 0;
    int n_v0_ele_trk_matched_clus = 0, n_v0_pos_trk_matched_clus = 0;
    int n_v0_trk_matched_clus_both = 0, n_v0_trk_matched_clus_pos_only = 0;
    int n_v0_trk_matched_clus_ele_only = 0;
    double best_trk_abs_dz = 1e9, best_trk_dz = -9999.;
    // Separate "best" bookkeeping for the cluster/track-p branches: picked by cluster-completeness
    // tier first (2 = both clusters, 1 = e+ cluster only, 0 = neither), z-closeness as tiebreaker,
    // so a v0 satisfying matched_trk_clus_both is guaranteed to be the one these branches describe
    // (a pure closest-in-z pick, like best_trk_dz, could otherwise report values from a DIFFERENT,
    // cluster-incomplete v0 when more than one track-matched v0 exists in the event).
    int best_clus_tier = -1;
    double best_clus_abs_dz = 1e9;
    CalCluster best_ele_clus, best_pos_clus;
    double best_ele_trk_p = -9999., best_pos_trk_p = -9999.;
    int best_ele_trk_nhits = 0, best_pos_trk_nhits = 0;
    bool have_best_trk_clus = false;
    bool matched_z_and_trk = false;
    for (Vertex* vtx : vtxs) {
        double dz = vtx->getZ() - true_vtx_z;
        if (std::fabs(dz) < best_abs_dz) {
            best_abs_dz = std::fabs(dz);
            best_dz = dz;
            best_vtx = vtx;
        }

        // Track-based match: this v0's two constituent tracks (independent of its z position)
        // majority-hit truth-match the true signal e- and e+ specifically (by MCParticle ID).
        // ele/pos matches are also checked individually (no requirement on the other leg) to see
        // how often a v0 captures just one of the two truth tracks vs. both.
        bool this_ele_matched = false, this_pos_matched = false;
        bool this_ele_has_clus = false, this_pos_has_clus = false;
        Particle *eleP{nullptr}, *posP{nullptr};  // tree-owned; stay valid past this if-block
        TRefArray parts = vtx->getParticles();
        if (parts.GetEntries() >= 2) {
            Particle* p0 = static_cast<Particle*>(parts.At(0));
            Particle* p1 = static_cast<Particle*>(parts.At(1));
            if (p0 && p1) {
                Track t0 = p0->getTrack();
                Track t1 = p1->getTrack();
                Track *eleT{nullptr}, *posT{nullptr};
                if (t0.getCharge() < 0 && t1.getCharge() > 0) {
                    eleT = &t0;
                    posT = &t1;
                    eleP = p0;
                    posP = p1;
                } else if (t1.getCharge() < 0 && t0.getCharge() > 0) {
                    eleT = &t1;
                    posT = &t0;
                    eleP = p1;
                    posP = p0;
                }
                if (eleT && trueEleID >= 0) this_ele_matched = (majorityMcpID(*eleT) == trueEleID);
                if (posT && truePosID >= 0) this_pos_matched = (majorityMcpID(*posT) == truePosID);
                // utils::buildParticle only calls setCluster() when the LCIO particle actually had
                // a track-cluster-matched cluster. Test via energy, not getNHits() (see note in
                // setFile() -- utils::buildCalCluster never populates the hit count). The electron
                // leg additionally requires eleClusEMin_ as a sanity floor against noise-level matches.
                if (eleP) this_ele_has_clus = (eleP->getCluster().getEnergy() > eleClusEMin_);
                if (posP) this_pos_has_clus = (posP->getCluster().getEnergy() > -9000.);

                if (this_ele_matched && this_ele_has_clus) n_v0_ele_trk_matched_clus++;
                if (this_pos_matched && this_pos_has_clus) n_v0_pos_trk_matched_clus++;
                if (this_ele_matched && this_pos_matched) {
                    if (this_ele_has_clus && this_pos_has_clus) n_v0_trk_matched_clus_both++;
                    if (this_pos_has_clus) n_v0_trk_matched_clus_pos_only++;  // e- cluster not required
                    if (this_ele_has_clus) n_v0_trk_matched_clus_ele_only++;  // e+ cluster not required
                }
            }
        }
        bool this_trk_matched = this_ele_matched && this_pos_matched;
        if (this_ele_matched) n_v0_ele_trk_matched++;
        if (this_pos_matched) n_v0_pos_trk_matched++;
        if (this_trk_matched) {
            n_v0_trk_matched++;
            if (std::fabs(dz) < best_trk_abs_dz) {
                best_trk_abs_dz = std::fabs(dz);
                best_trk_dz = dz;
            }
            if (eleP && posP) {
                int this_tier = this_pos_has_clus ? (this_ele_has_clus ? 2 : 1) : 0;
                bool better = (this_tier > best_clus_tier) ||
                              (this_tier == best_clus_tier && std::fabs(dz) < best_clus_abs_dz);
                if (better) {
                    best_clus_tier = this_tier;
                    best_clus_abs_dz = std::fabs(dz);
                    best_ele_clus = eleP->getCluster();
                    best_pos_clus = posP->getCluster();
                    best_ele_trk_p = eleP->getTrack().getP();
                    best_pos_trk_p = posP->getTrack().getP();
                    best_ele_trk_nhits = eleP->getTrack().getTrackerHitCount();
                    best_pos_trk_nhits = posP->getTrack().getTrackerHitCount();
                    have_best_trk_clus = true;
                }
            }
            if (std::fabs(dz) < vertexMatchZ_) matched_z_and_trk = true;
        }
    }
    bool matched_z = has_v0 && (true_vtx_z > -9000.) && (best_abs_dz < vertexMatchZ_);
    bool matched_trk = (n_v0_trk_matched >= 1);
    bool matched_trk_ele = (n_v0_ele_trk_matched >= 1);  // >=1 v0 has the truth e- (no requirement on e+)
    bool matched_trk_pos = (n_v0_pos_trk_matched >= 1);  // >=1 v0 has the truth e+ (no requirement on e-)
    bool matched_z_or_trk = matched_z || matched_trk;
    bool matched_trk_ele_clus = (n_v0_ele_trk_matched_clus >= 1);
    bool matched_trk_pos_clus = (n_v0_pos_trk_matched_clus >= 1);
    bool matched_trk_clus_both = (n_v0_trk_matched_clus_both >= 1);
    bool matched_trk_clus_pos_only = (n_v0_trk_matched_clus_pos_only >= 1);
    bool matched_trk_clus_ele_only = (n_v0_trk_matched_clus_ele_only >= 1);

    bus_.set("n_v0", n_v0);
    bus_.set("has_v0", has_v0);
    bus_.set("matched_z", matched_z);
    if (best_vtx) {
        bus_.set<double>("best_dz", best_dz);
        bus_.set<double>("best_vtx_x", best_vtx->getX());
        bus_.set<double>("best_vtx_y", best_vtx->getY());
        bus_.set<double>("best_vtx_z", best_vtx->getZ());
        bus_.set<double>("best_vtx_chi2", best_vtx->getChi2());
    }
    bus_.set("n_v0_trk_matched", n_v0_trk_matched);
    bus_.set("matched_trk", matched_trk);
    bus_.set("matched_z_or_trk", matched_z_or_trk);
    bus_.set("matched_z_and_trk", matched_z_and_trk);
    if (n_v0_trk_matched > 0) bus_.set<double>("best_trk_dz", best_trk_dz);
    bus_.set("matched_trk_ele", matched_trk_ele);
    bus_.set("matched_trk_pos", matched_trk_pos);
    bus_.set("matched_trk_ele_clus", matched_trk_ele_clus);
    bus_.set("matched_trk_pos_clus", matched_trk_pos_clus);
    bus_.set("matched_trk_clus_both", matched_trk_clus_both);
    bus_.set("matched_trk_clus_pos_only", matched_trk_clus_pos_only);
    bus_.set("matched_trk_clus_ele_only", matched_trk_clus_ele_only);
    if (have_best_trk_clus) {
        bool ele_has_clus = best_ele_clus.getEnergy() > eleClusEMin_;
        bool pos_has_clus = best_pos_clus.getEnergy() > -9000.;
        bus_.set("has_ele_clus", ele_has_clus);
        bus_.set("has_pos_clus", pos_has_clus);
        bus_.set<double>("ele_trk_p", best_ele_trk_p);
        bus_.set<double>("pos_trk_p", best_pos_trk_p);
        bus_.set<int>("ele_trk_nhits", best_ele_trk_nhits);
        bus_.set<int>("pos_trk_nhits", best_pos_trk_nhits);
        if (ele_has_clus) {
            std::vector<double> xyz = best_ele_clus.getPosition();
            bus_.set<double>("ele_clus_E", best_ele_clus.getEnergy());
            bus_.set<double>("ele_clus_x", xyz[0]);
            bus_.set<double>("ele_clus_y", xyz[1]);
            bus_.set<double>("ele_clus_z", xyz[2]);
            bus_.set<double>("ele_clus_t", best_ele_clus.getTime());
        }
        if (pos_has_clus) {
            std::vector<double> xyz = best_pos_clus.getPosition();
            bus_.set<double>("pos_clus_E", best_pos_clus.getEnergy());
            bus_.set<double>("pos_clus_x", xyz[0]);
            bus_.set<double>("pos_clus_y", xyz[1]);
            bus_.set<double>("pos_clus_z", xyz[2]);
            bus_.set<double>("pos_clus_t", best_pos_clus.getTime());
        }
    }

    event_cf_.apply("at_least_one_v0", has_v0);
    event_cf_.fill_nm1("at_least_one_v0", n_v0);
    event_cf_.apply("v0_within_z", matched_z);
    event_cf_.fill_nm1("v0_within_z", best_abs_dz < 1e8 ? best_abs_dz : 49.999);

    // ---- reconstructed track inventory (NO truth match): are the e+/e- tracks there? ----
    // For the no-v0 events this separates tracking (no tracks reconstructed) from vertexing
    // (both tracks reconstructed but no v0 formed). charge>0 = e+, charge<0 = e-.
    int n_trk_pos = 0, n_trk_ele = 0;
    if (bus_.has(trkColl_)) {
        const auto& tracks{bus_.get<std::vector<Track*>>(trkColl_)};
        for (Track* trk : tracks) {
            if (trk->getCharge() > 0) n_trk_pos++;
            else if (trk->getCharge() < 0) n_trk_ele++;
        }
    }
    bus_.set("n_trk_pos", n_trk_pos);
    bus_.set("n_trk_ele", n_trk_ele);

    if (truth_signal_found) {
        n_truth_++;
        if (has_v0) n_has_v0_++;
        if (matched_z) n_matched_z_++;
        if (matched_trk) n_matched_trk_++;
        if (matched_z_and_trk) n_matched_z_and_trk_++;
        if (matched_trk_ele) n_matched_trk_ele_++;
        if (matched_trk_pos) n_matched_trk_pos_++;
        if (matched_trk_ele_clus) n_matched_trk_ele_clus_++;
        if (matched_trk_pos_clus) n_matched_trk_pos_clus_++;
        if (matched_trk_clus_both) n_matched_trk_clus_both_++;
        if (matched_trk_clus_pos_only) n_matched_trk_clus_pos_only_++;
        if (matched_trk_clus_ele_only) n_matched_trk_clus_ele_only_++;
        if (not has_v0) {  // tracking-vs-vertexing breakdown for the no-v0 events
            bool hp = (n_trk_pos > 0), he = (n_trk_ele > 0);
            if (hp && he) n_novtx_both_trk_++;
            else if (hp && not he) n_novtx_pos_only_++;
            else if (he && not hp) n_novtx_ele_only_++;
            else n_novtx_neither_++;
        }
        // secondary category: events with an energetic truth electron (p > eleMomCut_)
        if (foundEle && trueEleP.P() > eleMomCut_) {
            n_truth_ehi_++;
            if (has_v0) n_has_v0_ehi_++;
            if (matched_z) n_matched_z_ehi_++;
        }
    }

    output_tree_->Fill();
    return true;
}

void TruthVertexMatch2021::finalize() {
    auto pct = [](long num, long den) { return den > 0 ? 100.0 * num / den : 0.0; };
    std::cout << "\n============= TruthVertexMatch2021 summary =============\n"
              << "  truth-signal events             : " << n_truth_ << "\n"
              << "  (1) NO reco v0 in event         : " << (n_truth_ - n_has_v0_)
              << "  (" << pct(n_truth_ - n_has_v0_, n_truth_) << "%)\n"
              << "  (2) >=1 reco v0 (no selection)  : " << n_has_v0_
              << "  (" << pct(n_has_v0_, n_truth_) << "%)\n"
              << "  (3) >=1 v0 within " << vertexMatchZ_ << " mm in z of truth : " << n_matched_z_
              << "  (" << pct(n_matched_z_, n_truth_) << "% of truth, "
              << pct(n_matched_z_, n_has_v0_) << "% of has-v0)\n";
    std::cout << "  --- track-based match: v0's 2 tracks truth-match signal e-/e+ (any z) ---\n"
              << "  (4) >=1 v0 track-matched         : " << n_matched_trk_
              << "  (" << pct(n_matched_trk_, n_truth_) << "% of truth, "
              << pct(n_matched_trk_, n_has_v0_) << "% of has-v0)\n"
              << "  (5) >=1 v0 BOTH z- and trk-matched : " << n_matched_z_and_trk_
              << "  (" << pct(n_matched_z_and_trk_, n_truth_) << "% of truth)\n"
              << "  z-matched but NOT track-matched  : " << (n_matched_z_ - n_matched_z_and_trk_)
              << "  <- z-window catching wrong/combinatoric tracks\n"
              << "  track-matched but NOT z-matched  : " << (n_matched_trk_ - n_matched_z_and_trk_)
              << "  <- correct tracks vertexed outside the +/-" << vertexMatchZ_ << " mm window\n"
              << "  --- single-leg track match: is it harder to get BOTH tracks than just one? ---\n"
              << "  >=1 v0 has truth e- (any e+)     : " << n_matched_trk_ele_
              << "  (" << pct(n_matched_trk_ele_, n_truth_) << "% of truth)\n"
              << "  >=1 v0 has truth e+ (any e-)     : " << n_matched_trk_pos_
              << "  (" << pct(n_matched_trk_pos_, n_truth_) << "% of truth)\n"
              << "  >=1 v0 has BOTH (matched_trk)    : " << n_matched_trk_
              << "  (" << pct(n_matched_trk_, n_truth_) << "% of truth)\n";
    std::cout << "  --- ECal cluster requirement (within matched_trk = both tracks correct) ---\n"
              << "  the singles trigger only needs the e+ cluster; is requiring an e- cluster too costing us?\n"
              << "  >=1 v0: e- track has cluster (any e+) : " << n_matched_trk_ele_clus_
              << "  (" << pct(n_matched_trk_ele_clus_, n_truth_) << "% of truth)\n"
              << "  >=1 v0: e+ track has cluster (any e-) : " << n_matched_trk_pos_clus_
              << "  (" << pct(n_matched_trk_pos_clus_, n_truth_) << "% of truth)\n"
              << "  STRICT  both tracks + BOTH clusters   : " << n_matched_trk_clus_both_
              << "  (" << pct(n_matched_trk_clus_both_, n_truth_) << "% of truth, "
              << pct(n_matched_trk_clus_both_, n_matched_trk_) << "% of matched_trk)\n"
              << "  RELAXED both tracks + e+ cluster ONLY : " << n_matched_trk_clus_pos_only_
              << "  (" << pct(n_matched_trk_clus_pos_only_, n_truth_) << "% of truth, "
              << pct(n_matched_trk_clus_pos_only_, n_matched_trk_) << "% of matched_trk)\n"
              << "  both tracks + e- cluster ONLY (e+ optional) : " << n_matched_trk_clus_ele_only_
              << "  (" << pct(n_matched_trk_clus_ele_only_, n_truth_) << "% of truth, "
              << pct(n_matched_trk_clus_ele_only_, n_matched_trk_) << "% of matched_trk)\n"
              << "  ACCEPTANCE GAIN from dropping the e- cluster requirement: +"
              << (n_matched_trk_clus_pos_only_ - n_matched_trk_clus_both_) << " events  (+"
              << (pct(n_matched_trk_clus_pos_only_, n_truth_) - pct(n_matched_trk_clus_both_, n_truth_))
              << " pp of truth)\n";
    long n_novtx = n_truth_ - n_has_v0_;
    std::cout << "  --- NO-v0 track inventory (KalmanFullTracks, no truth match) ---\n"
              << "  no-v0 events                    : " << n_novtx << "\n"
              << "  both e+ and e- track present    : " << n_novtx_both_trk_
              << "  (" << pct(n_novtx_both_trk_, n_novtx) << "%)  <- VERTEXING (tracks exist, no v0)\n"
              << "  e+ track only (no e-)           : " << n_novtx_pos_only_
              << "  (" << pct(n_novtx_pos_only_, n_novtx) << "%)\n"
              << "  e- track only (no e+)           : " << n_novtx_ele_only_
              << "  (" << pct(n_novtx_ele_only_, n_novtx) << "%)\n"
              << "  neither track                   : " << n_novtx_neither_
              << "  (" << pct(n_novtx_neither_, n_novtx) << "%)  <- TRACKING (no tracks)\n";
    std::cout << "  --- secondary category: truth e- momentum > " << eleMomCut_ << " GeV ---\n"
              << "  truth-signal (e- p>cut) events  : " << n_truth_ehi_ << "\n"
              << "  (1) NO reco v0                  : " << (n_truth_ehi_ - n_has_v0_ehi_)
              << "  (" << pct(n_truth_ehi_ - n_has_v0_ehi_, n_truth_ehi_) << "%)\n"
              << "  (2) >=1 reco v0 (no selection)  : " << n_has_v0_ehi_
              << "  (" << pct(n_has_v0_ehi_, n_truth_ehi_) << "%)\n"
              << "  (3) >=1 v0 within " << vertexMatchZ_ << " mm in z       : " << n_matched_z_ehi_
              << "  (" << pct(n_matched_z_ehi_, n_truth_ehi_) << "% of these, "
              << pct(n_matched_z_ehi_, n_has_v0_ehi_) << "% of has-v0)\n"
              << "=======================================================\n" << std::endl;

    outF_->cd();
    output_tree_->Write();
    event_cf_.save();
    outF_->Close();
}

DECLARE_PROCESSOR(TruthVertexMatch2021);
