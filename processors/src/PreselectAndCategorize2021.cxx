/*
* @file PreselectAndCategorize2021.cxx
* @author Sarah Gaiser
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
        for (const auto& [run, entry]: beamspot_corrections_json.items()) {
            beamspot_corrections_[std::stoi(run)] = {
                entry["beamspot_x"],
                entry["beamspot_y"],
                entry["beamspot_z"]
            };
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

        for (const auto& [name, corr]: track_corr.items()) {
            // validate names of corrections, Track::applyCorrection silently ignores
            // names that don't exactly match the names within it, so we check for
            // misspellings once here
            if (name != "track_z0" and name != "track_time") {
                std::cerr << "WARNING: Unrecognized track correction "
                << name << " : " << corr
                << " will be skipped."
                << std::endl;
                continue;
            }
            track_corrections_[name] = corr;
        }
    }

    calTimeOffset_ = parameters.getDouble("calTimeOffset");
    isData_ = parameters.getInteger("isData") != 0;
    isSignal_ = parameters.getInteger("isSignal") != 0;
}


void PreselectAndCategorize2021::initialize(TTree* tree) {
    _ah =  std::make_shared<AnaHelpers>();

    //init Reading Tree
    bus_.board_input<EventHeader>(tree, "EventHeader");
    bus_.board_input<TSData>(tree, "TSBank");
    if (not trkColl_.empty())
        bus_.board_input<std::vector<Track*>>(tree, trkColl_);
    bus_.board_input<std::vector<Vertex*>>(tree, vtxColl_);
    if (not isData_ and not mcColl_.empty())
        bus_.board_input<std::vector<MCParticle*>>(tree, mcColl_);
    
    /* pre-selection on vertices */
    vertex_cf_.add("positron_clusterE_above_0pt2GeV", 100, 0, 4.0);
    vertex_cf_.add("ele_track_cluster_within_6pt9ns", 100, 0.0, 10.0);
    vertex_cf_.add("pos_track_cluster_within_6pt0ns", 100, 0.0, 10.0);
    vertex_cf_.add("ele_track_chi2ndf", 100, 0.0, 30.0);
    vertex_cf_.add("pos_track_chi2ndf", 100, 0.0, 30.0);
    vertex_cf_.add("electron_below_2pt9GeV", 100, 0.0, 4.0);
    vertex_cf_.add("electron_above_0pt4GeV", 100, 0.0, 4.0);
    vertex_cf_.add("positron_above_0pt4GeV", 100, 0.0, 4.0);
    vertex_cf_.add("ele_min_9_hits", 14, 0, 14);
    vertex_cf_.add("pos_min_9_hits", 14, 0, 14);
    vertex_cf_.add("vertex_chi2", 100, 0.0, 30.0);
    vertex_cf_.add("vtx_max_p_4pt0GeV", 100, 0.0, 4.0);
    vertex_cf_.init();
    
    /* event selection after vertex selection */
    event_cf_.add("single_trigger", 2, -0.5, 1.5);
    event_cf_.add("at_least_one_vertex", 10, 0.0, 10.0);
    event_cf_.add("no_extra_vertices", 10, 0.0, 10.0);
    // if (isSignal_) {
    //     event_cf_.add("at_least_one_true_vd", 3, 0.0, 2.0);
    //     event_cf_.add("no_extra_true_vd", 3, 0.0, 2.0);
    // }
    event_cf_.init();

    n_vertices_h_ = std::make_unique<TH2F>(
        "n_vertices_h",
        "N Vertices in Event (readout and preselected)",
        10, -0.5, 9.5,
        10, -0.5, 9.5
    );
}

void PreselectAndCategorize2021::setFile(TFile* out_file) {
    Processor::setFile(out_file);
    
    // create output TTree in output file
    output_tree_ = std::make_unique<TTree>("preselection",
                                           "Preselected and Categorized Vertices",
                                           99, out_file);
    bus_.board_output<double>(output_tree_.get(), "weight");
    bus_.board_output<Vertex>(output_tree_.get(), "vertex");
    bus_.board_output<Particle>(output_tree_.get(), "ele");
    bus_.board_output<Particle>(output_tree_.get(), "pos");
    bus_.board_output<double>(output_tree_.get(), "psum");

    for (const auto& name : {"eleL1", "eleL2", "posL1", "posL2"}) {
        bus_.board_output<bool>(output_tree_.get(), name);
    }

    if (not v0proj_fits_.empty()) {
        for (const auto& name : {"vtx_proj_sig", "vtx_proj_x", "vtx_proj_x_sig",
                                 "vtx_proj_y", "vtx_proj_y_sig"}) {
            bus_.board_output<double>(output_tree_.get(), name);
        }
    }

    if (bus_.has(mcColl_) and isSignal_) {
        // bus_.board_output<MCParticle>(output_tree_.get(), "true_vd");
        bus_.board_output<bool>(output_tree_.get(), "isRadEle");
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
    // event_cf_.apply("single_trigger", (tsbank.isSingle2Trigger() or tsbank.isSingle3Trigger()));
    // if (not event_cf_.keep()) {
    //     // we leave BEFORE filling the vertex counting histogram
    //     // so that the vertex count histogram is relative to this trigger
    //     // and does not include other triggers
    //     return true;
    // }

    const auto& vtxs{bus_.get<std::vector<Vertex*>>(vtxColl_)};
    auto trks{bus_.get<std::vector<Track*>>(trkColl_)};
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
    std::vector<std::tuple<Vertex,Particle,Particle>> preselected_vtx;
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
        Track* ele_trk_ptr;
        Track* pos_trk_ptr;
        if (not trkColl_.empty()) {
            bool foundTracks = _ah->MatchToGBLTracks(ele.getTrack().getID(), pos.getTrack().getID(), ele_trk_ptr, pos_trk_ptr, trks);
            if (not foundTracks) {
                std::cout << "PreselectAndCategorize2021::ERROR: couldn't find tracks" << std::endl;
                continue;
            }
            ele_trk = *ele_trk_ptr;
            pos_trk = *pos_trk_ptr;
        } else {
            ele_trk = ele.getTrack();
            pos_trk = pos.getTrack();
        }    

        // apply track_z0 and track_time corrections loaded from JSON
        for (const auto& [name, corr]: track_corrections_) {
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
                throw std::runtime_error("The run number " + std::to_string(eh.getRunNumber())
                    +" is not in the loaded set of beamspot corrections.");
            }
            // assume iterator bsit is pointing to an element of beamspot_corrections_
            // for the current run number.
            // correction is the negative of the beamspot position to remove its shift
            double bs_y = -1*bsit->second.at(1);
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
        if (not ele.getTrack().isKalmanTrack()) ele_nhits*=2;
        int pos_nhits = pos.getTrack().getTrackerHitCount();
        if (not pos.getTrack().isKalmanTrack()) pos_nhits*=2;
        
        double ele_track_cluster_tdiff{
            abs(ele.getTrack().getTrackTime()-pos.getCluster().getTime())
        };
        double pos_track_cluster_tdiff{
            abs(pos.getTrack().getTrackTime()-pos.getCluster().getTime())
        };
        
        TVector3 ele_mom(
            ele.getTrack().getMomentum()[0],
            ele.getTrack().getMomentum()[1],
            ele.getTrack().getMomentum()[2]
        );
        TVector3 pos_mom(
            pos.getTrack().getMomentum()[0],
            pos.getTrack().getMomentum()[1],
            pos.getTrack().getMomentum()[2]
        );

        vertex_cf_.begin_event();
        vertex_cf_.apply("positron_clusterE_above_0pt2GeV", pos.getCluster().getEnergy() >= 0.2);
        vertex_cf_.apply("ele_track_cluster_within_6pt9ns", ele_track_cluster_tdiff <= 6.9);
        vertex_cf_.apply("pos_track_cluster_within_6pt0ns", pos_track_cluster_tdiff <= 6.0);
        vertex_cf_.apply("ele_track_chi2ndf", ele.getTrack().getChi2Ndf() <= 20.0);
        vertex_cf_.apply("pos_track_chi2ndf", pos.getTrack().getChi2Ndf() <= 20.0);
        vertex_cf_.apply("electron_below_2pt9GeV", ele.getTrack().getP() <= 2.9);
        vertex_cf_.apply("electron_above_0pt4GeV", ele.getTrack().getP() >= 0.4);
        vertex_cf_.apply("positron_above_0pt4GeV", pos.getTrack().getP() >= 0.4);
        vertex_cf_.apply("ele_min_9_hits", ele_nhits >= 9);
        vertex_cf_.apply("pos_min_9_hits", pos_nhits >= 9);
        vertex_cf_.apply("vertex_chi2", vtx->getChi2() <= 20.0);
        double vtxmaxp = ele_mom.Mag() + pos_mom.Mag();
        vertex_cf_.apply("vtx_max_p_4pt0GeV", vtxmaxp <= 4.0);
        
        vertex_cf_.fill_nm1("positron_clusterE_above_0pt2GeV", pos.getCluster().getEnergy());
        vertex_cf_.fill_nm1("ele_track_cluster_within_6pt9ns", ele_track_cluster_tdiff);
        vertex_cf_.fill_nm1("pos_track_cluster_within_6pt0ns", pos_track_cluster_tdiff);
        vertex_cf_.fill_nm1("ele_track_chi2ndf", ele.getTrack().getChi2Ndf());
        vertex_cf_.fill_nm1("pos_track_chi2ndf", pos.getTrack().getChi2Ndf());
        vertex_cf_.fill_nm1("electron_below_2pt9GeV", ele.getTrack().getP());
        vertex_cf_.fill_nm1("electron_above_0pt4GeV", ele.getTrack().getP());
        vertex_cf_.fill_nm1("positron_above_0pt4GeV", pos.getTrack().getP());
        vertex_cf_.fill_nm1("ele_min_9_hits", ele_nhits);
        vertex_cf_.fill_nm1("pos_min_9_hits", pos_nhits);
        vertex_cf_.fill_nm1("vertex_chi2", vtx->getChi2());
        vertex_cf_.fill_nm1("vtx_max_p_4pt0GeV", vtxmaxp);
    
        if (vertex_cf_.keep()) {
            preselected_vtx.emplace_back(*vtx, ele, pos);
        }
    }

    n_vertices_h_->Fill(vtxs.size(), preselected_vtx.size());
    event_cf_.apply("at_least_one_vertex", preselected_vtx.size() > 0);
    event_cf_.apply("no_extra_vertices", preselected_vtx.size() < 2);
    if (not event_cf_.keep()) {
        return true;
    }
    
    // correct number of vertices (i.e. only one)
    // unpack the vector of vertices into the single elements
    auto [ vtx, ele, pos ] = preselected_vtx.at(0);

    // earliest layer hit categories
    bool eleL1{false}, eleL2{false}, posL1{false}, posL2{false};
    Track ele_trk{ele.getTrack()}, pos_trk{pos.getTrack()};
    _ah->InnermostLayerCheck(&ele_trk, eleL1, eleL2);
    _ah->InnermostLayerCheck(&pos_trk, posL1, posL2);
    bus_.set("eleL1", eleL1);
    bus_.set("eleL2", eleL2);
    bus_.set("posL1", posL1);
    bus_.set("posL2", posL2);

    TVector3 ele_mom(
        ele_trk.getMomentum()[0],
        ele_trk.getMomentum()[1],
        ele_trk.getMomentum()[2]
    );
    TVector3 pos_mom(
        pos_trk.getMomentum()[0],
        pos_trk.getMomentum()[1],
        pos_trk.getMomentum()[2]
    );
    bus_.set("psum", ele_mom.Mag() + pos_mom.Mag());

    // calculate target projection and its significance
    if (not v0proj_fits_.empty()) {
        double vtx_proj_x{-1.0}, vtx_proj_y{-1.0};
        double vtx_proj_x_sig{-1.0}, vtx_proj_y_sig{-1.0};
        double vtx_proj_sig{-1.0};

        vtx_proj_sig = utils::v0_projection_to_target_significance(
            v0proj_fits_, eh.getRunNumber(),
            vtx_proj_x, vtx_proj_y, vtx_proj_x_sig, vtx_proj_y_sig,
            vtx.getX(), vtx.getY(), vtx.getZ(),
            vtx.getP().X(), vtx.getP().Y(), vtx.getP().Z()
        );

        bus_.set("vtx_proj_sig", vtx_proj_sig);
        bus_.set("vtx_proj_x", vtx_proj_x);
        bus_.set("vtx_proj_x_sig", vtx_proj_x_sig);
        bus_.set("vtx_proj_y", vtx_proj_y);
        bus_.set("vtx_proj_y_sig", vtx_proj_y_sig);
    }

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
    if (bus_.has(mcColl_) and isSignal_) {
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
        int n_vd{0};
        bool ele_is_rad_ele{false};
        for (MCParticle* ptr : mc_ptr) {
            if (ptr->getPDG() == 625) {
                n_vd++;
                vd = ptr;
            } else if (ptr->getID() == truth_ele_id) {
                ele_is_rad_ele = (ptr->getMomPDG() == 625);
            }
        }
        // event_cf_.apply("at_least_one_true_vd", n_vd > 0);
        // event_cf_.apply("no_extra_true_vd", n_vd < 2);
        if (not event_cf_.keep()) {
            return true;
        }
        if (not vd) {
            throw std::runtime_error(
                "ERROR: Logic error: checked for VD earlier but there isn't one."
            );
        }
        bus_.set("true_vd", *vd);
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
