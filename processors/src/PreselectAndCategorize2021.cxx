/*
 * @file PreselectAndCategorize2021.cxx
 * @author Sarah Gaiser adapting Tom Eichlersmith's PreselectAndCategorize
 * @date Apr 2025
 */

#include "PreselectAndCategorize2021.h"

void PreselectAndCategorize2021::configure(const ParameterSet& parameters) {
    auto pSmearingFile = parameters.getString("pSmearingFile");

    if (not pSmearingFile.empty()) {
        // just using the same seed=42 for now
        std::cout << "Loading momentum smearing from " << pSmearingFile << std::endl;
        smearingTool_ = std::make_shared<TrackSmearingTool>(pSmearingFile, true);
    }

    auto beamPosCfg = parameters.getString("beamPosCfg");
    if (not beamPosCfg.empty()) {
        std::cout << "Loading beamspot corrections from " << beamPosCfg << std::endl;
        auto beamspot_corrections_json = preselect_json_load(beamPosCfg);
        for (const auto& [run, entry] : beamspot_corrections_json.items()) {
            beamspot_corrections_[std::stoi(run)] = {entry["beamspot_x"], entry["beamspot_y"], entry["beamspot_z"]};
        }
    }

    auto v0ProjectionFitsCfg = parameters.getString("v0ProjectionFitsCfg");
    if (not v0ProjectionFitsCfg.empty()) {
        std::cout << "Loading projection fits from " << v0ProjectionFitsCfg << std::endl;
        v0proj_fits_ = preselect_json_load(v0ProjectionFitsCfg);
    }

    auto trackBiasCfg = parameters.getString("trackBiasCfg");
    if (not trackBiasCfg.empty()) {
        std::cout << "Loading track corrections from " << trackBiasCfg << std::endl;
        auto track_corr = preselect_json_load(trackBiasCfg);
        std::cout << track_corr << std::endl;

        for (const auto& [name, corr] : track_corr.items()) {
            // validate names of corrections, Track::applyCorrection silently ignores
            // names that don't exactly match the names within it, so we check for
            // misspellings once here
            if (name != "track_z0" and name != "track_time") {
                std::cerr << "WARNING: Unrecognized track correction " << name << " : " << corr << " will be skipped."
                          << std::endl;
                continue;
            }
            track_corrections_[name] = corr;
        }
    }

    calTimeOffset_ = parameters.getDouble("calTimeOffset");
    isData_ = parameters.getInteger("isData") != 0;
    isSimpSignal_ = parameters.getInteger("isSimpSignal") != 0;
    isApSignal_ = parameters.getInteger("isApSignal") != 0;
    if (isSimpSignal_ || isApSignal_) isSignal_ = true;
}

std::vector<double> PreselectAndCategorize2021::determine_time_cuts(bool isData, int runNumber) {
    std::vector<double> time_cuts;

    if (isData) {
        // Apply data-specific time cuts
        // time_cuts = {6.9, 5.2, 9.0}; // default values
        time_cuts = {7.11, 5.3, 9.5};     // early runs
        if (runNumber >= 14566) {         // bias voltage increased after this run, better time resolution
            time_cuts = {6.7, 5.0, 8.8};  // later runs
        }
    } else {
        // Apply MC-specific time cuts
        // time_cuts = {9.8, 7.2, 14.1};  // MC with track time smearing
        time_cuts = {3.0, 3.0, 4.2};  // MC without track time smearing
    }

    return time_cuts;
}

void PreselectAndCategorize2021::initialize(TTree* tree) {
    _ah = std::make_shared<AnaHelpers>();

    // init Reading Tree
    bus_.board_input<EventHeader>(tree, "EventHeader");
    bus_.board_input<TSData>(tree, "TSBank");
    if (not trkColl_.empty()) bus_.board_input<std::vector<Track*>>(tree, trkColl_);
    if (not hitColl_.empty()) bus_.board_input<std::vector<TrackerHit*>>(tree, hitColl_);
    bus_.board_input<std::vector<Vertex*>>(tree, vtxColl_);
    if (not isData_ and not mcColl_.empty()) bus_.board_input<std::vector<MCParticle*>>(tree, mcColl_);

    /* pre-selection on vertices */
    // vertex_cf_.add("single_trigger", 2, -0.5, 1.5);
    time_cuts_ = determine_time_cuts(isData_, bus_.get<EventHeader>("EventHeader").getRunNumber());
    vertex_cf_.add("positron_clusterE_above_0pt2GeV", 100, 0, 4.0);
    vertex_cf_.add("ele_track_cluster", 200, 0.0, 20.0);
    vertex_cf_.add("pos_track_cluster", 200, 0.0, 20.0);
    vertex_cf_.add("ele_pos_track", 200, 0.0, 20.0);
    vertex_cf_.add("ele_track_chi2ndf", 100, 0.0, 30.0);
    vertex_cf_.add("pos_track_chi2ndf", 100, 0.0, 30.0);
    vertex_cf_.add("electron_below_2pt9GeV", 100, 0.0, 4.0);
    vertex_cf_.add("electron_above_0pt4GeV", 100, 0.0, 4.0);
    vertex_cf_.add("positron_above_0pt4GeV", 100, 0.0, 4.0);
    vertex_cf_.add("ele_min_10_hits", 14, 0, 14);
    vertex_cf_.add("pos_min_10_hits", 14, 0, 14);
    vertex_cf_.add("vertex_chi2", 100, 0.0, 30.0);
    vertex_cf_.add("vtx_max_p_4pt0GeV", 100, 0.0, 4.0);
    vertex_cf_.init();

    std::stringstream ele_trk_clu_cut, pos_trk_clu_cut, ele_pos_trk_cut;
    ele_trk_clu_cut << "|t_{trk, e^{-}} - t_{clu, e^{+}}| < " << time_cuts_[0] << " ns";
    pos_trk_clu_cut << "|t_{trk, e^{+}} - t_{clu, e^{+}}| < " << time_cuts_[1] << " ns";
    ele_pos_trk_cut << "|t_{trk, e^{-}} - t_{trk, e^{+}}| < " << time_cuts_[2] << " ns";
    std::vector<std::string> labels_vertex_cf = {
        "reconstructed",       "E_{e^{+}} > 0.2 GeV",     ele_trk_clu_cut.str(),        pos_trk_clu_cut.str(),
        ele_pos_trk_cut.str(), "e^{-} #chi^{2}/ndf < 20", "e^{+} #chi^{2}/ndf < 20",    "p_{e^{-}} < 2.9 GeV",
        "p_{e^{-}} > 0.4 GeV", "p_{e^{+}} > 0.4 GeV",     "N_{2D hits, e^{-}} #geq 10", "N_{2D hits, e^{+}} #geq 10",
        "#chi^{2}_{vtx} < 20", "p_{vtx} < 4.0 GeV"};
    vertex_cf_.set_label_names(labels_vertex_cf);

    /* event selection after vertex selection */
    event_cf_.add("single_trigger", 2, -0.5, 1.5);
    event_cf_.add("at_least_one_vertex", 10, 0.0, 10.0);
    event_cf_.add("no_extra_vertices", 10, 0.0, 10.0);
    if (isSimpSignal_) {
        event_cf_.add("at_least_one_true_vd", 3, 0.0, 2.0);
        event_cf_.add("no_extra_true_vd", 3, 0.0, 2.0);
    }
    if (isApSignal_) {
        event_cf_.add("at_least_one_true_ap", 3, 0.0, 2.0);
        event_cf_.add("no_extra_true_ap", 3, 0.0, 2.0);
    }
    event_cf_.init();
    std::vector<std::string> labels_event_cf = {"readout", "single3 trigger", "N_{vtx} >= 1", "N_{vtx} < 2"};
    event_cf_.set_label_names(labels_event_cf);

    n_vertices_h_ = std::make_unique<TH2F>(
        "n_vertices_h", "N Vertices in Event; N_{vtx}^{readout}; N_{vtx}^{preselected}", 10, -0.5, 9.5, 10, -0.5, 9.5);
}

void PreselectAndCategorize2021::setFile(TFile* out_file) {
    Processor::setFile(out_file);

    // create output TTree in output file
    output_tree_ = std::make_unique<TTree>("preselection", "Preselected and Categorized Vertices", 99, out_file);
    bus_.board_output<double>(output_tree_.get(), "weight");
    bus_.board_output<Vertex>(output_tree_.get(), "vertex");
    bus_.board_output<Particle>(output_tree_.get(), "ele");
    bus_.board_output<Particle>(output_tree_.get(), "pos");
    bus_.board_output<double>(output_tree_.get(), "psum");

    /***************************************
     * adding specific cut variables       *
     ***************************************/

    // hit categories
    for (const auto& name : {"eleL1", "eleL2", "posL1", "posL2"}) {
        bus_.board_output<bool>(output_tree_.get(), name);
    }

    // vertex projection to target
    if (not v0proj_fits_.empty()) {
        for (const auto& name : {"vtx_proj_sig", "vtx_proj_x", "vtx_proj_x_sig", "vtx_proj_y", "vtx_proj_y_sig"}) {
            bus_.board_output<double>(output_tree_.get(), name);
        }
    }

    // isolation cut
    for (const auto& name : {"ele_L1_iso", "pos_L1_iso", "ele_L1_iso_significance", "pos_L1_iso_significance"}) {
        bus_.board_output<double>(output_tree_.get(), name);
    }

    // vertical impact parameter
    for (const auto& name : {"min_y0", "max_y0err"}) {
        bus_.board_output<double>(output_tree_.get(), name);
    }

    if (bus_.has(mcColl_)) {
        if (isSimpSignal_) {
            bus_.board_output<MCParticle>(output_tree_.get(), "true_vd");
        }
        if (isApSignal_) {
            bus_.board_output<MCParticle>(output_tree_.get(), "true_ap");
        }

        bus_.board_output<bool>(output_tree_.get(), "isRadEle");
        bus_.board_output<double>(output_tree_.get(), "true_vertex_invM");
        bus_.board_output<double>(output_tree_.get(), "true_vertex_psum");
    }
}

bool PreselectAndCategorize2021::process(IEvent*) {
    const auto& tsbank{bus_.get<TSData>("TSBank")};
    const auto& eh{bus_.get<EventHeader>("EventHeader")};
    int run_number = eh.getRunNumber();

    event_cf_.begin_event();

    // re-apply trigger desicion to data sample since it contains
    // other triggers within it
    // MC is created with only this trigger AND the event header
    // is not updated so we need to skip this check for MC
    event_cf_.apply("single_trigger", (tsbank.isSingle3Trigger()));
    event_cf_.fill_nm1("single_trigger", (tsbank.isSingle3Trigger()) ? 1 : 0);
    if (not event_cf_.keep()) {
        // we leave BEFORE filling the vertex counting histogram
        // so that the vertex count histogram is relative to this trigger
        // and does not include other triggers
        return true;
    }

    const auto& vtxs{bus_.get<std::vector<Vertex*>>(vtxColl_)};
    // auto trks{bus_.get<std::vector<Track*>>(trkColl_)};
    auto hits{bus_.get<std::vector<TrackerHit*>>(hitColl_)};
    /**
     * pre-selection on vertices defining "quality" vertices
     *
     * In this process of pre-selecting vertices, we also apply corrections
     * and momentum smearing to the underlying particle tracks and clusters.
     * These are then stored in our new copy of these objects in the resulting
     * preselected_vtx collection (vtx, ele, pos).
     *
     * We need to keep the particles separately since the particles "stored"
     * within Vertex are references to the collections. We make a copy here
     * to apply corrections and do not modify the collections referenced
     * elsewhere in memory.
     */
    std::vector<std::tuple<Vertex, Particle, Particle>> preselected_vtx;
    for (Vertex* vtx : vtxs) {
        // access the indiviual Vertex, electron, and positron
        // and add corrections to them before applying pre-selection
        int i_ele{-1}, i_pos{-1};
        for (int ipart = 0; ipart < vtx->getParticles().GetEntries(); ++ipart) {
            int pdg_id = ((Particle*)vtx->getParticles().At(ipart))->getPDG();
            if (pdg_id == 11) {
                i_ele = ipart;
            } else if (pdg_id == -11) {
                i_pos = ipart;
            }
        }
        if (i_ele < 0 or i_pos < 0) {
            throw std::runtime_error("Vertex formed without either an electron or positron!");
        }

        Particle ele = *dynamic_cast<Particle*>(vtx->getParticles().At(i_ele));
        Particle pos = *dynamic_cast<Particle*>(vtx->getParticles().At(i_pos));

        // for both particles, get cluster, update, re-set cluster
        CalCluster ele_clu = ele.getCluster();
        CalCluster pos_clu = pos.getCluster();

        ele_clu.setTime(ele_clu.getTime() - calTimeOffset_);
        pos_clu.setTime(pos_clu.getTime() - calTimeOffset_);

        ele.setCluster(&ele_clu);
        pos.setCluster(&pos_clu);

        Track ele_trk;
        Track pos_trk;
        // Track* ele_trk_ptr;
        // Track* pos_trk_ptr;
        // if (not trkColl_.empty()) {
        //     bool foundTracks = _ah->MatchToGBLTracks(ele.getTrack().getID(), pos.getTrack().getID(), ele_trk_ptr,
        //     pos_trk_ptr, trks); if (not foundTracks) {
        //         std::cout << "PreselectAndCategorize2021::ERROR: couldn't find tracks" << std::endl;
        //         continue;
        //     }
        //     ele_trk = *ele_trk_ptr;
        //     pos_trk = *pos_trk_ptr;
        // } else {
        ele_trk = ele.getTrack();
        pos_trk = pos.getTrack();
        // }

        // apply track_z0 and track_time corrections loaded from JSON
        for (const auto& [name, corr] : track_corrections_) {
            ele_trk.applyCorrection(name, corr);
            pos_trk.applyCorrection(name, corr);
        }

        if (not beamspot_corrections_.empty()) {
            /**
             * When the user provides a set of beamspot corrections,
             * we apply them to the tracks in a vertex shifting the
             * track_z0 by the beamspot y.
             * (Same direction just different coordinate names.)
             *
             * This is separate from the track_corrections_ because,
             * for real data, the beamspot_corrections_ change run-by-run
             * while for simulation samples, they do not.
             *
             * In the samples inherited from Alic and the L1L1 analysis,
             * these beamspot corrections were applied earlier in the
             * chain for data and so do not need to be re-applied at this level;
             * however, I am leaving this code here since it was helpful to
             * confirm this.
             */
            auto bsit = beamspot_corrections_.find(eh.getRunNumber());
            if (bsit == beamspot_corrections_.end()) {
                // the current run number is not in the provided set of beamspots
                // I don't think this should be the case, so I'm throwing an exception.
                // We could also just pick the closest run number which is what
                // the V0 projection fit code does.
                throw std::runtime_error("The run number " + std::to_string(eh.getRunNumber()) +
                                         " is not in the loaded set of beamspot corrections.");
            }
            // assume iterator bsit is pointing to an element of beamspot_corrections_
            // for the current run number.
            // correction is the negative of the beamspot position to remove its shift
            double bs_y = -1 * bsit->second.at(1);
            ele_trk.applyCorrection("track_z0", bs_y);
            pos_trk.applyCorrection("track_z0", bs_y);
        }

        // smear track momentum
        if (smearingTool_) {
            double ele_smear = smearingTool_->updateWithSmearP(ele_trk);
            double pos_smear = smearingTool_->updateWithSmearP(pos_trk);
            smearingTool_->updateVertexWithSmearP(vtx, ele_smear, pos_smear);
        }

        // put tracks back into their particles
        // with their new data
        ele.setTrack(&ele_trk);
        pos.setTrack(&pos_trk);

        // corrections have been applied, now onto pre-selecting
        int ele_nhits = ele.getTrack().getTrackerHitCount();
        if (not ele.getTrack().isKalmanTrack()) ele_nhits *= 2;
        int pos_nhits = pos.getTrack().getTrackerHitCount();
        if (not pos.getTrack().isKalmanTrack()) pos_nhits *= 2;

        double ele_track_cluster_tdiff{abs(ele.getTrack().getTrackTime() - pos.getCluster().getTime())};
        double pos_track_cluster_tdiff{abs(pos.getTrack().getTrackTime() - pos.getCluster().getTime())};

        TVector3 ele_mom(ele.getTrack().getMomentum()[0], ele.getTrack().getMomentum()[1],
                         ele.getTrack().getMomentum()[2]);
        TVector3 pos_mom(pos.getTrack().getMomentum()[0], pos.getTrack().getMomentum()[1],
                         pos.getTrack().getMomentum()[2]);

        vertex_cf_.begin_event();
        vertex_cf_.apply("positron_clusterE_above_0pt2GeV", pos.getCluster().getEnergy() >= 0.2);
        vertex_cf_.apply("ele_track_cluster", ele_track_cluster_tdiff <= time_cuts_[0]);
        vertex_cf_.apply("pos_track_cluster", pos_track_cluster_tdiff <= time_cuts_[1]);
        vertex_cf_.apply("ele_pos_track",
                         abs(ele.getTrack().getTrackTime() - pos.getTrack().getTrackTime()) <= time_cuts_[2]);
        vertex_cf_.apply("ele_track_chi2ndf", ele.getTrack().getChi2Ndf() <= 20.0);
        vertex_cf_.apply("pos_track_chi2ndf", pos.getTrack().getChi2Ndf() <= 20.0);
        vertex_cf_.apply("electron_below_2pt9GeV", ele.getTrack().getP() <= 2.9);
        vertex_cf_.apply("electron_above_0pt4GeV", ele.getTrack().getP() >= 0.4);
        vertex_cf_.apply("positron_above_0pt4GeV", pos.getTrack().getP() >= 0.4);
        vertex_cf_.apply("ele_min_10_hits", ele_nhits >= 10);
        vertex_cf_.apply("pos_min_10_hits", pos_nhits >= 10);
        vertex_cf_.apply("vertex_chi2", vtx->getChi2() <= 20.0);
        double vtxmaxp = ele_mom.Mag() + pos_mom.Mag();
        vertex_cf_.apply("vtx_max_p_4pt0GeV", vtxmaxp <= 4.0);

        vertex_cf_.fill_nm1("positron_clusterE_above_0pt2GeV", pos.getCluster().getEnergy());
        vertex_cf_.fill_nm1("ele_track_cluster", ele_track_cluster_tdiff);
        vertex_cf_.fill_nm1("pos_track_cluster", pos_track_cluster_tdiff);
        vertex_cf_.fill_nm1("ele_pos_track", abs(ele.getTrack().getTrackTime() - pos.getTrack().getTrackTime()));
        vertex_cf_.fill_nm1("ele_track_chi2ndf", ele.getTrack().getChi2Ndf());
        vertex_cf_.fill_nm1("pos_track_chi2ndf", pos.getTrack().getChi2Ndf());
        vertex_cf_.fill_nm1("electron_below_2pt9GeV", ele.getTrack().getP());
        vertex_cf_.fill_nm1("electron_above_0pt4GeV", ele.getTrack().getP());
        vertex_cf_.fill_nm1("positron_above_0pt4GeV", pos.getTrack().getP());
        vertex_cf_.fill_nm1("ele_min_10_hits", ele_nhits);
        vertex_cf_.fill_nm1("pos_min_10_hits", pos_nhits);
        vertex_cf_.fill_nm1("vertex_chi2", vtx->getChi2());
        vertex_cf_.fill_nm1("vtx_max_p_4pt0GeV", vtxmaxp);

        if (vertex_cf_.keep()) {
            preselected_vtx.emplace_back(*vtx, ele, pos);
        }
    }

    n_vertices_h_->Fill(vtxs.size(), preselected_vtx.size());
    event_cf_.apply("at_least_one_vertex", preselected_vtx.size() > 0);
    event_cf_.apply("no_extra_vertices", preselected_vtx.size() < 2);

    event_cf_.fill_nm1("at_least_one_vertex", preselected_vtx.size());
    event_cf_.fill_nm1("no_extra_vertices", preselected_vtx.size());
    if (not event_cf_.keep()) {
        return true;
    }

    // correct number of vertices (i.e. only one)
    // unpack the vector of vertices into the single elements
    auto [vtx, ele, pos] = preselected_vtx.at(0);

    // earliest layer hit categories
    bool eleL1{false}, eleL2{false}, posL1{false}, posL2{false};
    Track ele_trk{ele.getTrack()}, pos_trk{pos.getTrack()};
    auto ele_layers = _ah->GetTrackHitLayers(&ele_trk);
    auto pos_layers = _ah->GetTrackHitLayers(&pos_trk);

    if (ele_layers.at(0) == 1 && ele_layers.at(1) == 1) eleL1 = true;
    if (ele_layers.at(2) == 1 && ele_layers.at(3) == 1) eleL2 = true;
    if (pos_layers.at(0) == 1 && pos_layers.at(1) == 1) posL1 = true;
    if (pos_layers.at(2) == 1 && pos_layers.at(3) == 1) posL2 = true;

    bus_.set("eleL1", eleL1);
    bus_.set("eleL2", eleL2);
    bus_.set("posL1", posL1);
    bus_.set("posL2", posL2);

    double ele_L1_iso{9999.0}, pos_L1_iso{9999.0};
    if (eleL1 && eleL2 && posL1 && posL2) {
        if (ele_trk.isKalmanTrack()) {
            ele_L1_iso = utils::getKalmanTrackL1Isolations(&ele_trk, &hits);
            pos_L1_iso = utils::getKalmanTrackL1Isolations(&pos_trk, &hits);
        }
    }
    bus_.set("ele_L1_iso", ele_L1_iso);
    bus_.set("pos_L1_iso", pos_L1_iso);

    // isolation significance
    double ele_L1_iso_significance{9999.0}, pos_L1_iso_significance{9999.0};

    if (eleL1 && eleL2 && posL1 && posL2) {
        int sign_ele_py = (ele_trk.getMomentum()[1] > 0) - (ele_trk.getMomentum()[0] < 0);
        ele_L1_iso_significance = (2 * ele_L1_iso + sign_ele_py * ele_trk.getZ0()) / ele_trk.getZ0Err();
        int sign_pos_py = (pos_trk.getMomentum()[1] > 0) - (pos_trk.getMomentum()[0] < 0);
        pos_L1_iso_significance = (2 * pos_L1_iso + sign_pos_py * pos_trk.getZ0()) / pos_trk.getZ0Err();
    }
    bus_.set("ele_L1_iso_significance", ele_L1_iso_significance);
    bus_.set("pos_L1_iso_significance", pos_L1_iso_significance);

    TVector3 ele_mom(ele_trk.getMomentum()[0], ele_trk.getMomentum()[1], ele_trk.getMomentum()[2]);
    TVector3 pos_mom(pos_trk.getMomentum()[0], pos_trk.getMomentum()[1], pos_trk.getMomentum()[2]);
    bus_.set("psum", ele_mom.Mag() + pos_mom.Mag());

    // calculate target projection and its significance
    if (not v0proj_fits_.empty()) {
        double vtx_proj_x{-1.0}, vtx_proj_y{-1.0};
        double vtx_proj_x_sig{-1.0}, vtx_proj_y_sig{-1.0};
        double vtx_proj_sig{-1.0};

        vtx_proj_sig = utils::v0_projection_to_target_significance(v0proj_fits_, eh.getRunNumber(), vtx_proj_x,
                                                                   vtx_proj_y, vtx_proj_x_sig, vtx_proj_y_sig, &vtx);

        bus_.set("vtx_proj_sig", vtx_proj_sig);
        bus_.set("vtx_proj_x", vtx_proj_x);
        bus_.set("vtx_proj_x_sig", vtx_proj_x_sig);
        bus_.set("vtx_proj_y", vtx_proj_y);
        bus_.set("vtx_proj_y_sig", vtx_proj_y_sig);
    }

    // vertical impact parameters
    double min_y0{9999.0}, max_y0err{-1.0};

    abs(ele_trk.getZ0()) < abs(pos_trk.getZ0()) ? min_y0 = ele_trk.getZ0() : min_y0 = pos_trk.getZ0();
    ele_trk.getZ0Err() > pos_trk.getZ0Err() ? max_y0err = ele_trk.getZ0Err() : max_y0err = pos_trk.getZ0Err();

    bus_.set("min_y0", min_y0);
    bus_.set("max_y0err", max_y0err);

    // set vertex object and tracks
    bus_.set("weight", 1.);
    bus_.set("vertex", vtx);
    bus_.set("ele", ele);
    bus_.set("pos", pos);

    /**
     * This is where the output TTree is filled,
     * if we leave before this point, then the event will not
     * be kept as part of pre-selection.
     *
     * We wait until here to copy over the MCParticles to avoid
     * unnecessary copying if the event is not going to be kept
     * anyways.
     */
    if (bus_.has(mcColl_)) {
        /**
         * Before we loop through the MCParticles we go through the
         * the hits on the electron track in this vertex and find out
         * which MCParticle has the most hits on the track.
         */
        std::map<int, int> count_per_particle_id;
        for (const auto& [layer_id, particle_id] : ele_trk.getMcpHits()) {
            if (count_per_particle_id.find(particle_id) == count_per_particle_id.end()) {
                count_per_particle_id[particle_id] = 0;
            }
            count_per_particle_id[particle_id]++;
        }
        int truth_ele_id{-1}, max_nhits{0};
        for (const auto& [particle_id, count] : count_per_particle_id) {
            if (count > max_nhits) {
                truth_ele_id = particle_id;
                max_nhits = count;
            }
        }
        /**
         * The implementation of the beam-overlay mechanism very rarely causes two
         * signal events to occur within the same software event (or no signal event
         * to happen at all). The complexity of handling these cases is too high and
         * the rarity (both in simulation and expected due to small cross section)
         * motivates just removing these events from the sample.
         *
         * The warnings are still printed so an estimate on the number of events being
         * dropped can be retrieved from the logs.
         */
        const auto& mc_ptr{bus_.get<std::vector<MCParticle*>>(mcColl_)};
        MCParticle* vd{nullptr};
        MCParticle* ap{nullptr};
        ROOT::Math::PxPyPzEVector trueEleP;
        ROOT::Math::PxPyPzEVector truePosP;

        int n_vd{0};
        int n_ap{0};
        bool ele_is_rad_ele{false};
        for (MCParticle* ptr : mc_ptr) {
            std::vector<double> lP = ptr->getMomentum();
            if (isSimpSignal_) {
                if (ptr->getPDG() == 625) {
                    n_vd++;
                    vd = ptr;
                } else if (ptr->getID() == truth_ele_id) {
                    ele_is_rad_ele = (ptr->getMomPDG() == 625);
                    trueEleP = ROOT::Math::PxPyPzEVector(lP.at(0), lP.at(1), lP.at(2), ptr->getEnergy());
                } else if (ptr->getPDG() == -11 && ptr->getMomPDG() == 625) {
                    truePosP = ROOT::Math::PxPyPzEVector(lP.at(0), lP.at(1), lP.at(2), ptr->getEnergy());
                }
            } else if (isApSignal_) {
                if (ptr->getPDG() == 622) {
                    n_ap++;
                    ap = ptr;
                } else if (ptr->getID() == truth_ele_id) {
                    ele_is_rad_ele = (ptr->getMomPDG() == 622);
                    trueEleP = ROOT::Math::PxPyPzEVector(lP.at(0), lP.at(1), lP.at(2), ptr->getEnergy());
                } else if (ptr->getPDG() == -11 && ptr->getMomPDG() == 622) {
                    truePosP = ROOT::Math::PxPyPzEVector(lP.at(0), lP.at(1), lP.at(2), ptr->getEnergy());
                }
            } else {
                if (ptr->getID() == truth_ele_id) {
                    ele_is_rad_ele = (ptr->getMomPDG() == 623);
                }
                if (ptr->getPDG() == 11 && ptr->getMomPDG() == 623) {
                    trueEleP = ROOT::Math::PxPyPzEVector(lP.at(0), lP.at(1), lP.at(2), ptr->getEnergy());
                } else if (ptr->getPDG() == -11 && ptr->getMomPDG() == 623) {
                    truePosP = ROOT::Math::PxPyPzEVector(lP.at(0), lP.at(1), lP.at(2), ptr->getEnergy());
                }
            }
        }

        if (isSimpSignal_) {
            event_cf_.apply("at_least_one_true_vd", n_vd > 0);
            event_cf_.apply("no_extra_true_vd", n_vd < 2);
            if (not event_cf_.keep()) {
                return true;
            }
            if (not vd) {
                throw std::runtime_error("ERROR: Logic error: checked for VD earlier but there isn't one.");
            }
            bus_.set("true_vd", *vd);
        } else if (isApSignal_) {
            event_cf_.apply("at_least_one_true_ap", n_ap > 0);
            event_cf_.apply("no_extra_true_ap", n_ap < 2);
            if (not event_cf_.keep()) {
                return true;
            }
            if (not ap) {
                throw std::runtime_error("ERROR: Logic error: checked for AP earlier but there isn't one.");
            }
            bus_.set("true_ap", *ap);
        }
        if (trueEleP.P() > 0 and truePosP.P() > 0) {
            bus_.set("true_vertex_invM", (trueEleP + truePosP).M());
            bus_.set("true_vertex_psum", trueEleP.P() + truePosP.P());
        }

        bus_.set("isRadEle", ele_is_rad_ele);
    }
    output_tree_->Fill();
    return true;
}

void PreselectAndCategorize2021::finalize() {
    outF_->cd();
    output_tree_->Write();
    n_vertices_h_->Write();
    vertex_cf_.save();
    event_cf_.save();
}

DECLARE_PROCESSOR(PreselectAndCategorize2021);
