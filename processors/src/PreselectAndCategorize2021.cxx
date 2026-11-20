/*
 * @file PreselectAndCategorize2021.cxx
 * @author Sarah Gaiser adapting Tom Eichlersmith's PreselectAndCategorize
 * @date Apr 2025
 */

#include "PreselectAndCategorize2021.h"

void PreselectAndCategorize2021::configure(const ParameterSet& parameters) {
    // Track corrections (z0 offsets / calibration, data scale corrections, MC smearing) are
    // configured by one JSON for TrackCorrector; the per-job settings stay parameters here.
    trackCorrectionsCfg_ = parameters.getString("trackCorrectionsCfg", "");
    smearingSeed_ = parameters.getInteger("smearingSeed", smearingSeed_);
    doSmearing_ = parameters.getInteger("doSmearing", 0) != 0;
    // These settings moved into the trackCorrectionsCfg JSON; refuse them rather than ignore them.
    for (const char* moved : {"smearingCfg", "pSmearingFile", "trackBiasCfg", "z0CalibCfg", "z0CalibMcCfg"}) {
        if (!parameters.getString(moved, "").empty())
            throw std::runtime_error(std::string("PreselectAndCategorize2021: parameter '") + moved +
                                     "' moved to the trackCorrectionsCfg JSON");
    }

    // Debug output flag
    debug_ = parameters.getInteger("debug", 0) != 0;

    // Disable all vertex-level preselection cuts (default: cuts enabled)
    disablePreselection_ = parameters.getInteger("disablePreselection", 0) != 0;
    disableTimingCuts_   = parameters.getInteger("disableTimingCuts", 0) != 0;
    disableVertexChi2Cut_ = parameters.getInteger("disableVertexChi2Cut", 0) != 0;

    // If true (default), veto events with >=2 preselected vertices. If false, keep the event
    // and select the preselected vertex with the lowest vertex chi2 instead of vetoing.
    requireSingleVertex_ = parameters.getInteger("requireSingleVertex", 1) != 0;

    // Momentum source for the e+/e- used in psum and the per-track momentum cuts: vertex-fitted
    // (default) or standalone track-fit. Defaults to true to preserve prior behavior.
    useVertexMomentum_ = parameters.getInteger("useVertexMomentum", 1) != 0;

    // Minimum number of 2D tracker hits required per track (default 10). The electron and
    // positron can be controlled independently via eleMinHits/posMinHits; each defaults to
    // minHits when not set explicitly.
    minHits_ = parameters.getInteger("minHits", minHits_);
    eleMinHits_ = parameters.getInteger("eleMinHits", minHits_);
    posMinHits_ = parameters.getInteger("posMinHits", minHits_);

    // Which trigger to require: "singles2" (default: singles2||singles3) or "pairs"
    triggerSelection_ = parameters.getString("triggerSelection", triggerSelection_);

    // Optional minimum psum cut; disabled (default) when < 0
    psumCut_ = parameters.getDouble("psumCut", psumCut_);

    // Beam parameters used to infer the recoil-electron direction
    beamE_ = parameters.getDouble("beamE", beamE_);
    thetaBeamMrad_ = parameters.getDouble("thetaBeamMrad", thetaBeamMrad_);

    // Require truth match for smearing: -1 (default) uses the trackCorrectionsCfg value
    requireTruthMatch_ = parameters.getInteger("requireTruthMatch", -1);

    // Save all tracks in the event
    saveAllTracks_ = parameters.getInteger("saveAllTracks", 0) != 0;
    // Calculate variables that depend on multiple tracks
    calcMultiTrackVars_ = parameters.getInteger("calcMultiTrackVars", 0) != 0;
    // Hit requirement for the extra tracks; < 0 = eleMinHits/posMinHits
    extraTrackMinHits_ = parameters.getInteger("extraTrackMinHits", extraTrackMinHits_);



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


    auto vtxColl = parameters.getString("vtxCollection");
    if (not vtxColl.empty()) {
        std::cout << "Setting vertex collection to : " << vtxColl << std::endl;
        setVtxColl(vtxColl);
    }

    calTimeOffset_ = parameters.getDouble("calTimeOffset");
    isData_ = parameters.getInteger("isData") != 0;
    isSimpSignal_ = parameters.getInteger("isSimpSignal") != 0;
    isApSignal_ = parameters.getInteger("isApSignal") != 0;
    if (isSimpSignal_ || isApSignal_) isSignal_ = true;
    apPDG_ = parameters.getInteger("apPDG", 622);

    if (not trackCorrectionsCfg_.empty()) {
        TrackCorrector::Options opts;
        opts.isData = isData_;
        opts.seed = smearingSeed_;
        opts.doSmearing = doSmearing_;
        opts.requireTruthMatch = requireTruthMatch_;
        opts.debug = debug_;
        corrector_ = std::make_shared<TrackCorrector>(trackCorrectionsCfg_, opts);
        corrector_->printConfig();
    }
}

std::vector<double> PreselectAndCategorize2021::determine_time_cuts(bool isData, int runNumber) {
    std::vector<double> time_cuts;

    if (isData) {
        // Apply data-specific time cuts
        time_cuts = {6.0, 5.1, 7.8};  // v9 values
    } else {
        // Apply MC-specific time cuts
        time_cuts = {6.0, 5.7, 9.2};  // MC with track time smearing
        //time_cuts = {18.0, 17.1, 27.6};  // MC with track time smearing
        // time_cuts = {3.0, 3.0, 4.2};  // MC without track time smearing
    }

    return time_cuts;
}

double PreselectAndCategorize2021::calculate_theta_R(const TVector3& ele_mom, const TVector3& pos_mom) const {
    // Beam 4-momentum (assume m_e ~ 0 for the beam)
    double theta_beam = thetaBeamMrad_ / 1000.0;  // convert to radians
    double beam_px = beamE_ * std::sin(theta_beam);
    double beam_py = 0.0;
    double beam_pz = beamE_ * std::cos(theta_beam);

    // Inferred recoil 3-momentum from momentum conservation
    double recoil_px = beam_px - ele_mom.X() - pos_mom.X();
    double recoil_py = beam_py - ele_mom.Y() - pos_mom.Y();
    double recoil_pz = beam_pz - ele_mom.Z() - pos_mom.Z();

    TVector3 recoil_mom(recoil_px, recoil_py, recoil_pz);
    double p_recoil_mag = recoil_mom.Mag();
    if (p_recoil_mag <= 0.0) return -9999.0;

    // Beam direction (unit vector)
    TVector3 beam_dir(std::sin(theta_beam), 0.0, std::cos(theta_beam));

    // Angle between recoil and beam directions, in mrad
    double cos_theta = recoil_mom.Dot(beam_dir) / p_recoil_mag;
    cos_theta = std::max(-1.0, std::min(1.0, cos_theta));
    return std::acos(cos_theta) * 1000.0;
}

Track PreselectAndCategorize2021::createInferredTrack(Vertex* vtx, Track* ele_track, Track* pos_track) {

    Track inferredTrack;

    TVector3 ele_mom(ele_track->getMomentum()[0], ele_track->getMomentum()[1], ele_track->getMomentum()[2]);
    TVector3 pos_mom(pos_track->getMomentum()[0], pos_track->getMomentum()[1], pos_track->getMomentum()[2]);

    // Beam 4-momentum (assume m_e ~ 0 for the beam)
    double theta_beam = thetaBeamMrad_ / 1000.0;  // convert to radians
    double beam_px = beamE_ * std::sin(theta_beam);
    double beam_py = 0.0;
    double beam_pz = beamE_ * std::cos(theta_beam);

    // Inferred recoil 3-momentum from momentum conservation
    double recoil_px =  beam_px - ele_mom.X() - pos_mom.X();
    double recoil_py = beam_py - ele_mom.Y() - pos_mom.Y();
    double recoil_pz = beam_pz - ele_mom.Z() - pos_mom.Z();

    float inferredCharge = -1; // Third track is an electron
    float pT = std::sqrt(ele_mom.X()*ele_mom.X() + ele_mom.Z()*ele_mom.Z());
    float cB = pT * fabs(ele_track->getOmega());



    float target_pos = -1.1;
    // Calculate track parameters
    std::vector<float> params = TrackTools::calculateTrackParameters(
        vtx->getX(), vtx->getY(), vtx->getZ(),
        recoil_px, recoil_py, recoil_pz, inferredCharge, cB, target_pos);

    inferredTrack.setTrackParameters(
        params[0],  // d0
        params[1],  // phi0
        params[2],  // omega
        params[4],  // tanLambda
        params[3]   // z0
    );

    TMatrixD covVertex = TrackTools::getVertexCovariance(vtx);

    TMatrixD eleCovMomentum = TrackTools::momentumCovarianceFromTrack(ele_track);
    TMatrixD posCovMomentum = TrackTools::momentumCovarianceFromTrack(pos_track);

    TMatrixD covMomentum = eleCovMomentum + posCovMomentum;

    // Calculate track parameter covariance
    TMatrixD covTrackParams = TrackTools::calculateTrackCovariance(
        vtx->getX(), vtx->getY(), vtx->getZ(),
        recoil_px, recoil_py, recoil_pz,
        covVertex, covMomentum,
        inferredCharge, cB, target_pos);


    // Order: [d0, phi0, omega, z0, tanLambda]
    std::vector<float> covArray;
    covArray.reserve(15);
    
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j <= i; j++) {  
            covArray.push_back(covTrackParams(j, i));
        }
    }
    
    inferredTrack.setCov(covArray);
    
    // Set momentum
    inferredTrack.setMomentum(recoil_px, recoil_py, recoil_pz);

    return inferredTrack;

}


bool PreselectAndCategorize2021::isQualityTrack(Track* trk, const Particle& pos) const {
    // chi2/ndf <= 20
    if (trk->getChi2Ndf() > 20.0) return false;
    
    // n_hits: the per-charge preselection cuts by default, or an explicit override
    int required = (extraTrackMinHits_ >= 0)
        ? extraTrackMinHits_
        : (trk->getCharge() < 0 ? eleMinHits_ : posMinHits_);
    int nhits = trk->getTrackerHitCount();
    if (!trk->isKalmanTrack()) nhits *= 2;
    if (nhits < required) return false;
    
    // p >= 0.4
    double p = trk->getP();
    if (p < 0.4) return false;
    
    // for electrons (charge = -1), p <= 2.9
    if (trk->getCharge() < 0 && p > 2.9) return false;
    
    // Timing cuts relative to preselection positron
    double trk_time = trk->getTrackTime();
    double pos_trk_time = pos.getTrack().getTrackTime();
    double pos_clu_time = pos.getCluster().getTime();
    
    if (!disableTimingCuts_) {
        if (trk->getCharge() < 0) {       
            if (std::fabs(trk_time - pos_clu_time) > time_cuts_[0]) return false;
            if (std::fabs(trk_time - pos_trk_time) > time_cuts_[2]) return false;  
        } else if (trk->getCharge() > 0) {
            if (std::fabs(trk_time - pos_clu_time) > time_cuts_[1]) return false;
            if (std::fabs(trk_time - pos_trk_time) > time_cuts_[2]) return false;
        }
    }
    
    return true;
}

void PreselectAndCategorize2021::calculatePairwiseQuantities(
    const std::vector<Track*>& all_tracks,
    Particle& ele,
    Particle& pos)
{
    // Select quality tracks with good timing relative to preselection positron
    std::vector<Track> electrons, positrons;
    std::vector<Track> all_good_tracks;
    
    // include the preselected electron and positron first
    Track ele_trk = ele.getTrack();
    Track pos_trk = pos.getTrack();
    
    if (ele_trk.getCharge() < 0) {
        electrons.push_back(ele_trk);
        if (saveAllTracks_) all_good_tracks.push_back(ele_trk);
    }
    
    if (pos_trk.getCharge() > 0) {
        positrons.push_back(pos_trk);
        if (saveAllTracks_) all_good_tracks.push_back(pos_trk);
    }
    
    // Track which tracks from the collection we've already included
    std::set<int> included_track_ids;
    if (ele_trk.getID() >= 0) included_track_ids.insert(ele_trk.getID());
    if (pos_trk.getID() >= 0) included_track_ids.insert(pos_trk.getID());
    
    // Then check additional tracks from the collection
    for (auto* trk : all_tracks) {
        // Skip if this is the same track as preselected electron or positron
        if (trk->getID() >= 0 && included_track_ids.count(trk->getID()) > 0) {
            continue;
        }
        
        if (!isQualityTrack(trk, pos)) continue;
        
        // Store a copy of the good track
        if (saveAllTracks_) {
            all_good_tracks.push_back(*trk);
        }
        
        if (trk->getCharge() < 0) {
            electrons.push_back(*trk);
        } else if (trk->getCharge() > 0) {
            positrons.push_back(*trk);
        }
    }
    
    // Save all good tracks if enabled
    if (saveAllTracks_) {
        bus_.set("all_preselected_tracks", all_good_tracks);
    }
    
    // Initialize extrema values
    double min_dTanLambda = 9999.0, max_dTanLambda = -9999.0;
    double min_opening = 9999.0, max_opening = -9999.0;
    double min_mass = 9999.0, max_mass = -9999.0;
    double min_asym = 9999.0, max_asym = -9999.0;
    double min_dPhi = 9999.0, max_dPhi = -9999.0;
    
    const double me = 0.000511; 
    
    // Loop over all e+/e- pairs
    for (auto& ele_track : electrons) {
        const std::vector<double>& ele_mom = ele_track.getMomentum();
        TVector3 pe(ele_mom[0], ele_mom[1], ele_mom[2]);
        double pe_mag = pe.Mag();
        double ele_tanLambda = ele_track.getTanLambda();
        double ele_phi0 = ele_track.getPhi();
        
        for (auto& pos_track : positrons) {
            const std::vector<double>& pos_mom = pos_track.getMomentum();
            TVector3 pp(pos_mom[0], pos_mom[1], pos_mom[2]);
            double pp_mag = pp.Mag();
            double pos_tanLambda = pos_track.getTanLambda();
            double pos_phi0 = pos_track.getPhi();
            
            // TanLambda difference
            double dTanLambda = std::fabs(ele_tanLambda - pos_tanLambda);
            min_dTanLambda = std::min(min_dTanLambda, dTanLambda);
            max_dTanLambda = std::max(max_dTanLambda, dTanLambda);
            
            // Phi difference
            double dPhi = std::fabs(ele_phi0 - pos_phi0);
            min_dPhi = std::min(min_dPhi, dPhi);
            max_dPhi = std::max(max_dPhi, dPhi);
            
            // Opening angle
            double cos_theta = pe.Dot(pp) / (pe_mag * pp_mag);
            cos_theta = std::max(-1.0, std::min(1.0, cos_theta));
            double opening_angle = std::acos(cos_theta);
            min_opening = std::min(min_opening, opening_angle);
            max_opening = std::max(max_opening, opening_angle);
            
            // Invariant mass
            double Ee = std::sqrt(pe_mag * pe_mag + me * me);
            double Ep = std::sqrt(pp_mag * pp_mag + me * me);
            TVector3 p_tot = pe + pp;
            double E_tot = Ee + Ep;
            double m2 = E_tot * E_tot - p_tot.Mag2();
            double mass = (m2 > 0) ? std::sqrt(m2) : 0.0;
            min_mass = std::min(min_mass, mass);
            max_mass = std::max(max_mass, mass);
            
            // Asymmetry
            double asymmetry = std::fabs(pe_mag - pp_mag) / (pe_mag + pp_mag);
            min_asym = std::min(min_asym, asymmetry);
            max_asym = std::max(max_asym, asymmetry);
        }
    }
    
    // Store results (use -9999 if no pairs found)
    bool has_pairs = (!electrons.empty() && !positrons.empty());
    
    bus_.set("min_dTanLambda", has_pairs ? min_dTanLambda : -9999.0);
    bus_.set("max_dTanLambda", has_pairs ? max_dTanLambda : -9999.0);
    bus_.set("min_opening", has_pairs ? min_opening : -9999.0);
    bus_.set("max_opening", has_pairs ? max_opening : -9999.0);
    bus_.set("min_mass", has_pairs ? min_mass : -9999.0);
    bus_.set("max_mass", has_pairs ? max_mass : -9999.0);
    bus_.set("min_asym", has_pairs ? min_asym : -9999.0);
    bus_.set("max_asym", has_pairs ? max_asym : -9999.0);
    bus_.set("min_dPhi", has_pairs ? min_dPhi : -9999.0);
    bus_.set("max_dPhi", has_pairs ? max_dPhi : -9999.0);
}

void PreselectAndCategorize2021::initialize(TTree* tree) {
    _ah = std::make_shared<AnaHelpers>();

    // init Reading Tree
    bus_.board_input<EventHeader>(tree, "EventHeader");
    bus_.board_input<TSData>(tree, "TSBank");
    if (not trkColl_.empty()) bus_.board_input<std::vector<Track*>>(tree, trkColl_);
    // if (not hitColl_.empty()) bus_.board_input<std::vector<TrackerHit*>>(tree, hitColl_);
    bus_.board_input<std::vector<Vertex*>>(tree, vtxColl_);
    bus_.board_input<std::vector<CalCluster*>>(tree, "RecoEcalClusters");
    if (not isData_ and not mcColl_.empty()) bus_.board_input<std::vector<MCParticle*>>(tree, mcColl_);

    /* pre-selection on vertices */
    // vertex_cf_.add("single_trigger", 2, -0.5, 1.5);
    time_cuts_ = determine_time_cuts(isData_, bus_.get<EventHeader>("EventHeader").getRunNumber());
    // NOTE: the vertex cutflow's denominator (nocut bin) is now SEEDED: only v0 candidates with a
    // positron that has BOTH a track and a cluster, and an electron with a track (cluster optional),
    // enter the cutflow. Cluster-EXISTENCE is folded into the seed; the positron cluster-ENERGY
    // sanity cut (E_clus > 0.2 GeV) remains as the first cut after the skim.
    vertex_cf_.add("positron_clusterE_above_0pt2GeV", 100, 0, 4.0);
    vertex_cf_.add("electron_above_0pt2GeV", 100, 0.0, 4.0);
    vertex_cf_.add("electron_below_2pt9GeV", 100, 0.0, 4.0);
    vertex_cf_.add("ele_track_chi2ndf", 100, 0.0, 30.0);
    vertex_cf_.add("pos_track_chi2ndf", 100, 0.0, 30.0);
    vertex_cf_.add("positron_above_0pt4GeV", 100, 0.0, 4.0);
    vertex_cf_.add("ele_min_hits", 14, 0, 14);
    vertex_cf_.add("pos_min_hits", 14, 0, 14);
    vertex_cf_.add("ele_track_cluster", 200, 0.0, 60.0);
    vertex_cf_.add("pos_track_cluster", 200, 0.0, 20.0);
    vertex_cf_.add("ele_pos_track", 200, 0.0, 20.0);
    vertex_cf_.add("vertex_chi2", 100, 0.0, 30.0);
    if (psumCut_ >= 0.0) vertex_cf_.add("psum_min", 100, 0.0, 4.0);
    vertex_cf_.add("vtx_max_p_4pt0GeV", 100, 0.0, 4.0);
    vertex_cf_.init();

    std::stringstream ele_trk_clu_cut, pos_trk_clu_cut, ele_pos_trk_cut;
    ele_trk_clu_cut << "|t_{trk, e^{-}} - t_{clu, e^{+}}| < " << time_cuts_[0] << " ns";
    pos_trk_clu_cut << "|t_{trk, e^{+}} - t_{clu, e^{+}}| < " << time_cuts_[1] << " ns";
    ele_pos_trk_cut << "|t_{trk, e^{-}} - t_{trk, e^{+}}| < " << time_cuts_[2] << " ns";
    std::vector<std::string> labels_vertex_cf = {
        "e^{+} trk+clu, e^{-} trk", "E_{e^{+}} > 0.2 GeV",
        "p_{e^{-}} > 0.2 GeV", "p_{e^{-}} < 2.9 GeV",
        "e^{-} #chi^{2}/ndf < 20", "e^{+} #chi^{2}/ndf < 20", "p_{e^{+}} > 0.4 GeV",
        "N_{2D hits, e^{-}} #geq " + std::to_string(eleMinHits_), "N_{2D hits, e^{+}} #geq " + std::to_string(posMinHits_),
        ele_trk_clu_cut.str(), pos_trk_clu_cut.str(), ele_pos_trk_cut.str(),
        "#chi^{2}_{vtx} < 30"};
    if (psumCut_ >= 0.0) {
        std::stringstream psum_cut;
        psum_cut << "p_{sum} > " << psumCut_ << " GeV";
        labels_vertex_cf.push_back(psum_cut.str());
    }
    labels_vertex_cf.push_back("p_{vtx} < 4.0 GeV");
    vertex_cf_.set_label_names(labels_vertex_cf);

    /* event selection after vertex selection */
    event_cf_.add("single_trigger", 2, -0.5, 1.5);
    event_cf_.add("at_least_one_v0", 10, 0.0, 10.0);
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
    std::string trig_label = (triggerSelection_ == "pairs") ? "pairs trigger" : "single2 or 3 trigger";
    std::string no_extra_vtx_label = requireSingleVertex_ ? "N_{vtx}^{presel} < 2" : "best #chi^{2} selected";
    std::vector<std::string> labels_event_cf = {"readout", trig_label, "N_{v0}^{raw} >= 1", "N_{vtx}^{presel} >= 1", no_extra_vtx_label};
    event_cf_.set_label_names(labels_event_cf);

    n_vertices_h_ = std::make_unique<TH2F>(
        "n_vertices_h", "N Vertices in Event; N_{vtx}^{readout}; N_{vtx}^{preselected}", 10, -0.5, 9.5, 10, -0.5, 9.5);

    // Best-chi2 multi-vertex diagnostics: filled for every event with >=2 preselected vertices,
    // independent of requireSingleVertex_, so the effect of relaxing the veto can be checked
    // against real data/MC before turning it on.
    multivtx_chosen_chi2_h_ = std::make_unique<TH1F>(
        "multivtx_chosen_chi2_h", "Chosen vertex #chi^{2} (events with #geq2 preselected vertices);#chi^{2};N", 60, 0.0, 30.0);
    multivtx_discarded_chi2_h_ = std::make_unique<TH1F>(
        "multivtx_discarded_chi2_h", "Discarded vertex #chi^{2} (events with #geq2 preselected vertices);#chi^{2};N", 60, 0.0, 30.0);
}

void PreselectAndCategorize2021::setFile(TFile* out_file) {
    Processor::setFile(out_file);

    // create output TTree in output file
    output_tree_ = std::make_unique<TTree>("preselection", "Preselected and Categorized Vertices", 99, out_file);
    bus_.board_output<double>(output_tree_.get(), "weight");
    /**
     * Event identity. Without these an entry in this tree cannot be traced back to the
     * LCIO event it came from, which makes it impossible to inspect an interesting
     * candidate at any earlier stage of the chain (recon, readout, generation) or to
     * put two processings of the same events side by side.
     */
    bus_.board_output<int>(output_tree_.get(), "run_number");
    bus_.board_output<int>(output_tree_.get(), "event_number");
    // Position of the event in the input tuple: unique where MC event numbers are not.
    bus_.board_output<int>(output_tree_.get(), "input_entry");
    // Per raw v0 candidate (from vertex collection): the signed track-momentum scale the
    // corrections applied, omega_before / omega_after
    bus_.board_output<std::vector<double>>(output_tree_.get(), "vtx_ele_smear_scale");
    bus_.board_output<std::vector<double>>(output_tree_.get(), "vtx_pos_smear_scale");
    bus_.board_output<Vertex>(output_tree_.get(), "vertex");
    bus_.board_output<Particle>(output_tree_.get(), "ele");
    bus_.board_output<Particle>(output_tree_.get(), "pos");
    bus_.board_output<Track>(output_tree_.get(), "third_track");
    bus_.board_output<double>(output_tree_.get(), "psum");
    bus_.board_output<double>(output_tree_.get(), "psum_scalar");
    bus_.board_output<double>(output_tree_.get(), "epem_opening_angle");
    bus_.board_output<double>(output_tree_.get(), "recoil_theta");
    bus_.board_output<double>(output_tree_.get(), "ele_p_smear_ratio");
    bus_.board_output<double>(output_tree_.get(), "pos_p_smear_ratio");
    bus_.board_output<double>(output_tree_.get(), "pos_cl_min_dr");
    bus_.board_output<double>(output_tree_.get(), "pos_cl_min_dr_t");
    bus_.board_output<bool>(output_tree_.get(), "ele_has_truth_link");
    bus_.board_output<bool>(output_tree_.get(), "pos_has_truth_link");
    bus_.board_output<TVector3>(output_tree_.get(), "ele_track_p");
    bus_.board_output<TVector3>(output_tree_.get(), "pos_track_p");
    bus_.board_output<TVector3>(output_tree_.get(), "ele_track_p_prefit");
    bus_.board_output<TVector3>(output_tree_.get(), "pos_track_p_prefit");
    bus_.board_output<TVector3>(output_tree_.get(), "ele_truth_p");
    bus_.board_output<TVector3>(output_tree_.get(), "pos_truth_p");

    // Standalone-track vs vertex-fit comparison, independent of useVertexMomentum. Both are
    // captured before smearing/scale corrections, so the difference is due to the fit alone.
    for (const auto& name : {"trk_invM",
            "ele_p_prefit", "ele_px_prefit", "ele_py_prefit", "ele_pz_prefit",
            "pos_p_prefit", "pos_px_prefit", "pos_py_prefit", "pos_pz_prefit",
            "ele_p_postfit", "ele_px_postfit", "ele_py_postfit", "ele_pz_postfit",
            "pos_p_postfit", "pos_px_postfit", "pos_py_postfit", "pos_pz_postfit"}) {
        bus_.board_output<double>(output_tree_.get(), name);
    }

    /***************************************
     * adding specific cut variables       *
     ***************************************/

    // hit categories (layer has both axial+stereo)
    for (const auto& name : {"eleL1", "eleL2", "eleL3", "eleL4", "posL1", "posL2", "posL3", "posL4", "single2", "single3"}) {
        bus_.board_output<bool>(output_tree_.get(), name);
    }

    // vertex layer category flags (outward hit requirement enforced)
    for (const auto& name : {"isL1L1", "isL2L2", "isL3L3", "isL1L2", "isL2L3"}) {
        bus_.board_output<bool>(output_tree_.get(), name);
    }

    // top/bottom track flags
    for (const auto& name : {"ele_isTop", "pos_isTop"}) {
        bus_.board_output<bool>(output_tree_.get(), name);
    }

    // per-layer axial/stereo hit flags
    for (const auto& name : {
            "eleL1_axial", "eleL1_stereo", "eleL2_axial", "eleL2_stereo",
            "eleL3_axial", "eleL3_stereo", "eleL4_axial", "eleL4_stereo",
            "posL1_axial", "posL1_stereo", "posL2_axial", "posL2_stereo",
            "posL3_axial", "posL3_stereo", "posL4_axial", "posL4_stereo"}) {
        bus_.board_output<bool>(output_tree_.get(), name);
    }

    // vertex projection to target
    if (not v0proj_fits_.empty()) {
        for (const auto& name : {"vtx_proj_sig", "vtx_proj_x", "vtx_proj_x_sig", "vtx_proj_y", "vtx_proj_y_sig",
                                  "vtx_proj_x_centered", "vtx_proj_y_centered",
                                  "vtx_proj_sig_lcio", "vtx_proj_x_lcio", "vtx_proj_x_sig_lcio",
                                  "vtx_proj_y_lcio", "vtx_proj_y_sig_lcio",
                                  "vtx_proj_x_err_lcio", "vtx_proj_y_err_lcio",
                                  "vtx_proj_x_centered_lcio", "vtx_proj_y_centered_lcio"}) {
            bus_.board_output<double>(output_tree_.get(), name);
        }
    }

    // isolation cut
    for (const auto& name : {"ele_L1_iso", "pos_L1_iso", "ele_L1_iso_significance", "pos_L1_iso_significance",
                              "ele_L1_iso_significance_raw", "pos_L1_iso_significance_raw"}) {
        bus_.board_output<double>(output_tree_.get(), name);
    }

    // vertical impact parameter
    for (const auto& name : {"min_y0", "max_y0err", "min_y0err", "ele_y0err", "pos_y0err", "min_y0_vtx", "min_y0_vtx_proj", "vtx_y_at_zero", "delta_y0_vtx_proj"}) {
        bus_.board_output<double>(output_tree_.get(), name);
    }

    if (calcMultiTrackVars_) {
        for (const auto& extrema : {"min", "max"}) {
            bus_.board_output<double>(output_tree_.get(), std::string(extrema) + "_dTanLambda");
            bus_.board_output<double>(output_tree_.get(), std::string(extrema) + "_opening");
            bus_.board_output<double>(output_tree_.get(), std::string(extrema) + "_mass");
            bus_.board_output<double>(output_tree_.get(), std::string(extrema) + "_asym");
            bus_.board_output<double>(output_tree_.get(), std::string(extrema) + "_dPhi");
        }
    }

    if (saveAllTracks_) {
        bus_.board_output<std::vector<Track>>(output_tree_.get(), "all_preselected_tracks");
    }

    if (bus_.has(mcColl_)) {
        if (isSimpSignal_) {
            bus_.board_output<MCParticle>(output_tree_.get(), "true_vd");
            bus_.board_output<double>(output_tree_.get(), "true_vd_betagamma");
        }
        if (isApSignal_) {
            bus_.board_output<MCParticle>(output_tree_.get(), "true_ap");
            bus_.board_output<double>(output_tree_.get(), "true_decay_len");
            bus_.board_output<double>(output_tree_.get(), "true_ap_betagamma");
        }
        bus_.board_output<bool>(output_tree_.get(), "isRadEle");
        bus_.board_output<double>(output_tree_.get(), "true_vertex_invM");
        bus_.board_output<double>(output_tree_.get(), "true_vertex_psum");
    }
}

bool PreselectAndCategorize2021::process(IEvent*) {
    // Reset the output branches so that anything we do not explicitly set this
    // event is written as its default rather than carrying over the previous
    // event's value. Matters for the conditionally-filled truth branches
    // (true_vertex_invM, true_ap, true_decay_len, ...), which are only set when
    // the truth match succeeds.
    bus_.clear_outputs();

    const auto& tsbank{bus_.get<TSData>("TSBank")};
    const auto& eh{bus_.get<EventHeader>("EventHeader")};
    int run_number = eh.getRunNumber();
    if (corrector_) {
        const std::vector<MCParticle*>* mcParticles =
            bus_.has(mcColl_) ? &bus_.get<std::vector<MCParticle*>>(mcColl_) : nullptr;
        corrector_->beginEvent(run_number, mcParticles);
    }
    // Set immediately after clear_outputs so the identity is recorded no matter which
    // of the many early returns below ends up rejecting the event.
    bus_.set("run_number", run_number);
    bus_.set("event_number", eh.getEventNumber());
    bus_.set("input_entry", ++input_entry_);

    event_cf_.begin_event();

    // re-apply trigger desicion to data sample since it contains
    // other triggers within it
    // MC is created with only this trigger AND the event header
    // is not updated so we need to skip this check for MC
    // trigger decision selected by configuration: default "singles2" (singles2||singles3),
    // or "pairs" (OR of all pair triggers). Anything else falls back to the singles logic.
    bool trigger_decision = (triggerSelection_ == "pairs")
        ? tsbank.isPairTrigger()
        : (tsbank.isSingle3Trigger() || tsbank.isSingle2Trigger());
    event_cf_.apply("single_trigger", trigger_decision);
    event_cf_.fill_nm1("single_trigger", trigger_decision ? 1 : 0);
    if (not event_cf_.keep()) {
        // we leave BEFORE filling the vertex counting histogram
        // so that the vertex count histogram is relative to this trigger
        // and does not include other triggers
        return true;
    }

    const auto& allClusters{bus_.get<std::vector<CalCluster*>>("RecoEcalClusters")};

    const auto& vtxs{bus_.get<std::vector<Vertex*>>(vtxColl_)};

    // event-level: does the event have at least one raw v0 (before any preselection cuts)?
    // sits between the trigger and the preselected-vertex requirement so the cutflow shows
    // trigger -> >=1 v0 -> >=1 good (preselected) v0.
    event_cf_.apply("at_least_one_v0", vtxs.size() >= 1);
    event_cf_.fill_nm1("at_least_one_v0", vtxs.size());

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
     *
     * Per-vertex quantities destined for the output tree are carried along here as
     * well, rather than published from inside the loop below. The loop runs over
     * every raw v0 candidate, so a bus_.set() inside it survives only until the next
     * iteration overwrites it -- what reaches TTree::Fill() is the last raw candidate
     * iterated, which need not be the selected vertex and need not even have passed
     * preselection. Stashing them here and publishing after the best-chi2 vertex is
     * chosen keeps every branch on an entry describing the same vertex.
     */
    struct PreselectedVertex {
        Vertex vtx;
        Particle ele;
        Particle pos;
        double ele_z0_raw;
        double pos_z0_raw;
        TVector3 ele_track_p;
        TVector3 pos_track_p;
        TVector3 ele_track_p_prefit;
        TVector3 pos_track_p_prefit;
        TVector3 ele_p_postfit;
        TVector3 pos_p_postfit;
        double ele_p_smear_ratio;
        double pos_p_smear_ratio;
    };
    std::vector<PreselectedVertex> preselected_vtx;
    std::vector<double> vtx_ele_smear_scale, vtx_pos_smear_scale;  // one entry per raw v0
    int ivtx = 0;

    // Orphan-positron-cluster diagnostic, aggregated over the whole event. It only takes a
    // value for candidates whose positron has NO cluster, and those are rejected below -- so
    // it can never describe the vertex that gets written out, and is necessarily an
    // event-level quantity. We keep the closest orphan over all raw candidates; the sentinel
    // survives when every candidate had a positron cluster.
    double evt_pos_cl_min_dr = -9999.0;
    double evt_pos_cl_min_dr_t = -9999.0;

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

        Track ele_trk = ele.getTrack();
        Track pos_trk = pos.getTrack();

        // cache z0 before any corrections (used to recompute isolation significance)
        double ele_z0_raw = ele_trk.getZ0();
        double pos_z0_raw = pos_trk.getZ0();

        // snapshot pre-vertex-fit track momenta before they are overwritten
        TVector3 ele_p_prefit(ele_trk.getMomentum()[0], ele_trk.getMomentum()[1], ele_trk.getMomentum()[2]);
        TVector3 pos_p_prefit(pos_trk.getMomentum()[0], pos_trk.getMomentum()[1], pos_trk.getMomentum()[2]);
        // snapshot vertex-fitted momenta now: smearing below rescales them in place on vtx
        TVector3 ele_p_postfit(vtx->getP1X(), vtx->getP1Y(), vtx->getP1Z());
        TVector3 pos_p_postfit(vtx->getP2X(), vtx->getP2Y(), vtx->getP2Z());

        // Replace particle track momenta with the vertex-fitted momenta (only when enabled). The
        // vertex stores its two fitted momenta in a fixed electron-then-positron slot order
        // (verified against hps-java: BilliorVertexer.fitVertex preserves the input track order
        // [electron, positron] from HpsReconParticleDriver.makeV0Candidates, and
        // BilliorVertex::getParameters writes p1 <- electron, p2 <- positron), independent of
        // which vertical half (top/bottom) either track curves into. When useVertexMomentum_ is
        // false, the standalone track-fit momenta are kept as-is.
        if (useVertexMomentum_) {
            ele_trk.setMomentum(vtx->getP1X(), vtx->getP1Y(), vtx->getP1Z());
            pos_trk.setMomentum(vtx->getP2X(), vtx->getP2Y(), vtx->getP2Z());
        }

        // Track corrections, in TrackCorrector's fixed order: offsets and z0 calibration, data
        // scale corrections, MC smearing. The vertex-fitted momenta follow the track momenta.
        double ele_p_smear_ratio = 1.0;
        double pos_p_smear_ratio = 1.0;
        const double ele_omega_before = ele_trk.getOmega();
        const double pos_omega_before = pos_trk.getOmega();
        if (corrector_) {
            auto corr = corrector_->correct({&ele_trk, &pos_trk}, {-1, +1});
            ele_p_smear_ratio = corr[0].pRatio();
            pos_p_smear_ratio = corr[1].pRatio();
            if (corrector_->updatesVertex()) TrackCorrector::correctVertex(vtx, ele_p_smear_ratio, pos_p_smear_ratio);
            if (debug_)
                std::cout << "[TrackCorrector] ele ratio " << ele_p_smear_ratio << "  pos ratio " << pos_p_smear_ratio
                          << "  ele |p| " << ele_trk.getP() << "  pos |p| " << pos_trk.getP() << std::endl;
        }
        // Recorded for every raw candidate, before the seeding requirement below can skip it.
        vtx_ele_smear_scale.push_back(ele_trk.getOmega() != 0. ? ele_omega_before / ele_trk.getOmega() : 1.);
        vtx_pos_smear_scale.push_back(pos_trk.getOmega() != 0. ? pos_omega_before / pos_trk.getOmega() : 1.);
        // Captured, not published -- see the PreselectedVertex comment above.
        TVector3 ele_track_p(ele_trk.getMomentum()[0], ele_trk.getMomentum()[1], ele_trk.getMomentum()[2]);
        TVector3 pos_track_p(pos_trk.getMomentum()[0], pos_trk.getMomentum()[1], pos_trk.getMomentum()[2]);

        // put tracks back into their particles
        // with their new data
        ele.setTrack(&ele_trk);
        pos.setTrack(&pos_trk);

        // corrections have been applied, now onto pre-selecting
        int ele_nhits = ele.getTrack().getTrackerHitCount();
        if (not ele.getTrack().isKalmanTrack()) ele_nhits *= 2;
        int pos_nhits = pos.getTrack().getTrackerHitCount();
        if (not pos.getTrack().isKalmanTrack()) pos_nhits *= 2;

        if (debug_) {
            std::cout << "Tracker hit counts (vertex " << ivtx << "):" << std::endl;
            std::cout << "  Electron: TrackerHitCount=" << ele.getTrack().getTrackerHitCount()
                      << ", nhits (after scaling)=" << ele_nhits << std::endl;
            std::cout << "  Positron: TrackerHitCount=" << pos.getTrack().getTrackerHitCount()
                      << ", nhits (after scaling)=" << pos_nhits << std::endl;
        }

        TVector3 ele_mom(ele.getTrack().getMomentum()[0], ele.getTrack().getMomentum()[1],
                         ele.getTrack().getMomentum()[2]);
        TVector3 pos_mom(pos.getTrack().getMomentum()[0], pos.getTrack().getMomentum()[1],
                         pos.getTrack().getMomentum()[2]);

        TVector3 psum = ele_mom + pos_mom;

        // ----- seed the vertex cutflow -----
        // Only v0 candidates with a positron that has BOTH a track and a cluster, and an electron
        // with a track (its cluster is irrelevant here), are counted. These define the cutflow
        // denominator (nocut bin); the remaining cuts then measure preselection efficiency relative
        // to this clean candidate, instead of charging cluster-acceptance against the first cut.
        const auto& posCl = pos.getCluster();
        double posClE = posCl.getEnergy();
        bool pos_has_cluster = (posClE > -9000.0);
        bool pos_has_track   = (pos.getTrack().getTrackerHitCount() > 0);
        bool ele_has_track   = (ele.getTrack().getTrackerHitCount() > 0);

        // orphan-cluster diagnostic (only filled when the positron has NO cluster, which the seed
        // below rejects -> stays at the sentinel for seeded candidates).
        double pos_cl_min_dr = -9999.0;
        double pos_cl_min_dr_t = -9999.0;
        if (not pos_has_cluster) {
            auto posAtEcal = pos_trk.getPositionAtEcal();
            double min_dr = 9999.0;
            for (const auto* cl : allClusters) {
                auto clPos = cl->getPosition();
                double dx = clPos[0] - posAtEcal[0];
                double dy = clPos[1] - posAtEcal[1];
                double dr = std::sqrt(dx*dx + dy*dy);
                if (dr < min_dr) {
                    min_dr = dr;
                    pos_cl_min_dr_t = cl->getTime() - calTimeOffset_;
                }
            }
            pos_cl_min_dr = min_dr;
        }
        if (not pos_has_cluster &&
            (evt_pos_cl_min_dr < 0.0 || pos_cl_min_dr < evt_pos_cl_min_dr)) {
            evt_pos_cl_min_dr = pos_cl_min_dr;
            evt_pos_cl_min_dr_t = pos_cl_min_dr_t;
        }

        if (not (pos_has_track && pos_has_cluster && ele_has_track)) {
            ivtx++;
            continue;  // does not seed the cutflow and cannot be preselected
        }

        vertex_cf_.begin_event();
        // sanity cut on the (now-guaranteed-present) positron cluster energy
        vertex_cf_.apply("positron_clusterE_above_0pt2GeV", posClE >= 0.2);
        vertex_cf_.apply("electron_above_0pt2GeV", ele.getTrack().getP() >= 0.2);
        vertex_cf_.apply("electron_below_2pt9GeV", ele.getTrack().getP() <= 2.9);
        vertex_cf_.apply("ele_track_chi2ndf", ele.getTrack().getChi2Ndf() <= 20.0);
        vertex_cf_.apply("pos_track_chi2ndf", pos.getTrack().getChi2Ndf() <= 20.0);
        vertex_cf_.apply("positron_above_0pt4GeV", pos.getTrack().getP() >= 0.4);
        vertex_cf_.apply("ele_min_hits", ele_nhits >= eleMinHits_);
        vertex_cf_.apply("pos_min_hits", pos_nhits >= posMinHits_);
        vertex_cf_.apply("ele_track_cluster",
                         disableTimingCuts_ || fabs(ele.getTrack().getTrackTime() - pos.getCluster().getTime()) <= time_cuts_[0]);
        vertex_cf_.apply("pos_track_cluster",
                         disableTimingCuts_ || fabs(pos.getTrack().getTrackTime() - pos.getCluster().getTime()) <= time_cuts_[1]);
        vertex_cf_.apply("ele_pos_track",
                         disableTimingCuts_ || fabs(ele.getTrack().getTrackTime() - pos.getTrack().getTrackTime()) <= time_cuts_[2]);
        vertex_cf_.apply("vertex_chi2", disableVertexChi2Cut_ || vtx->getChi2() <= 30.0);
        if (psumCut_ >= 0.0) vertex_cf_.apply("psum_min", psum.Mag() > psumCut_);
        double vtxmaxp = ele_mom.Mag() + pos_mom.Mag();
        vertex_cf_.apply("vtx_max_p_4pt0GeV", psum.Mag() <= 4.0);

        vertex_cf_.fill_nm1("positron_clusterE_above_0pt2GeV", posClE);
        vertex_cf_.fill_nm1("electron_above_0pt2GeV", ele.getTrack().getP());
        vertex_cf_.fill_nm1("electron_below_2pt9GeV", ele.getTrack().getP());
        vertex_cf_.fill_nm1("ele_track_chi2ndf", ele.getTrack().getChi2Ndf());
        vertex_cf_.fill_nm1("pos_track_chi2ndf", pos.getTrack().getChi2Ndf());
        vertex_cf_.fill_nm1("positron_above_0pt4GeV", pos.getTrack().getP());
        vertex_cf_.fill_nm1("ele_min_hits", ele_nhits);
        vertex_cf_.fill_nm1("pos_min_hits", pos_nhits);
        vertex_cf_.fill_nm1("ele_track_cluster", fabs(ele.getTrack().getTrackTime() - pos.getCluster().getTime()));
        vertex_cf_.fill_nm1("pos_track_cluster", fabs(pos.getTrack().getTrackTime() - pos.getCluster().getTime()));
        vertex_cf_.fill_nm1("ele_pos_track", fabs(ele.getTrack().getTrackTime() - pos.getTrack().getTrackTime()));
        vertex_cf_.fill_nm1("vertex_chi2", vtx->getChi2());
        if (psumCut_ >= 0.0) vertex_cf_.fill_nm1("psum_min", psum.Mag());
        vertex_cf_.fill_nm1("vtx_max_p_4pt0GeV", vtxmaxp);

        if (disablePreselection_ || vertex_cf_.keep()) {
            preselected_vtx.push_back(PreselectedVertex{*vtx, ele, pos, ele_z0_raw, pos_z0_raw,
                                                        ele_track_p, pos_track_p,
                                                        ele_p_prefit, pos_p_prefit,
                                                        ele_p_postfit, pos_p_postfit,
                                                        ele_p_smear_ratio, pos_p_smear_ratio});
        }
        ivtx++;
    }

    n_vertices_h_->Fill(vtxs.size(), preselected_vtx.size());

    bus_.set("vtx_ele_smear_scale", vtx_ele_smear_scale);
    bus_.set("vtx_pos_smear_scale", vtx_pos_smear_scale);
    bus_.set("pos_cl_min_dr", evt_pos_cl_min_dr);
    bus_.set("pos_cl_min_dr_t", evt_pos_cl_min_dr_t);

    // Index of the preselected vertex with the lowest chi2 -- the vertex that will be carried
    // forward if the event survives. Trivially valid when there are 0 or 1 preselected vertices.
    std::size_t i_best_vtx = 0;
    for (std::size_t i = 1; i < preselected_vtx.size(); ++i) {
        if (preselected_vtx[i].vtx.getChi2() < preselected_vtx[i_best_vtx].vtx.getChi2()) {
            i_best_vtx = i;
        }
    }
    // Multi-vertex chi2 diagnostics: always filled (independent of requireSingleVertex_) so the
    // effect of relaxing the veto can be checked against data/MC before turning it on.
    if (preselected_vtx.size() >= 2) {
        for (std::size_t i = 0; i < preselected_vtx.size(); ++i) {
            double chi2 = preselected_vtx[i].vtx.getChi2();
            (i == i_best_vtx ? multivtx_chosen_chi2_h_ : multivtx_discarded_chi2_h_)->Fill(chi2);
        }
    }

    event_cf_.apply("at_least_one_vertex", disablePreselection_ || preselected_vtx.size() >= 1);
    event_cf_.apply("no_extra_vertices",
                     disablePreselection_ || !requireSingleVertex_ || preselected_vtx.size() < 2);

    event_cf_.fill_nm1("at_least_one_vertex", preselected_vtx.size());
    event_cf_.fill_nm1("no_extra_vertices", preselected_vtx.size());
    if (not event_cf_.keep()) {
        return true;
    }

    bool single2{false}, single3{false};
    if (tsbank.isSingle3Trigger() == 1) single3 = true;
    if (tsbank.isSingle2Trigger() == 1) single2 = true;
    bus_.set("single2", single2);
    bus_.set("single3", single3);

    // unpack the selected vertex into its elements. Normally there is exactly one preselected
    // vertex; if requireSingleVertex_ is false and multiple survived, i_best_vtx (computed above)
    // picks the one with the lowest vertex chi2.
    // Guard against empty collection (can occur when disablePreselection=true bypasses at_least_one_vertex)
    if (preselected_vtx.empty()) return true;
    auto selected = preselected_vtx.at(i_best_vtx);
    Vertex& vtx = selected.vtx;
    Particle& ele = selected.ele;
    Particle& pos = selected.pos;
    const double ele_z0_raw = selected.ele_z0_raw;
    const double pos_z0_raw = selected.pos_z0_raw;

    // Per-vertex quantities stashed during the loop, now published for the selected vertex.
    bus_.set("ele_track_p", selected.ele_track_p);
    bus_.set("pos_track_p", selected.pos_track_p);
    bus_.set("ele_track_p_prefit", selected.ele_track_p_prefit);
    bus_.set("pos_track_p_prefit", selected.pos_track_p_prefit);
    for (const auto& [prefix, prefit, postfit] : {
            std::make_tuple("ele_", selected.ele_track_p_prefit, selected.ele_p_postfit),
            std::make_tuple("pos_", selected.pos_track_p_prefit, selected.pos_p_postfit)}) {
        const std::string pre{prefix};
        bus_.set(pre + "p_prefit", prefit.Mag());
        bus_.set(pre + "px_prefit", prefit.X());
        bus_.set(pre + "py_prefit", prefit.Y());
        bus_.set(pre + "pz_prefit", prefit.Z());
        bus_.set(pre + "p_postfit", postfit.Mag());
        bus_.set(pre + "px_postfit", postfit.X());
        bus_.set(pre + "py_postfit", postfit.Y());
        bus_.set(pre + "pz_postfit", postfit.Z());
    }
    {
        // e+e- invariant mass from the standalone track momenta (never the vertex-fitted ones)
        constexpr double electron_mass{0.000510999};  // GeV
        const TVector3& pe{selected.ele_track_p_prefit};
        const TVector3& pp{selected.pos_track_p_prefit};
        ROOT::Math::PxPyPzMVector ele_p4(pe.X(), pe.Y(), pe.Z(), electron_mass);
        ROOT::Math::PxPyPzMVector pos_p4(pp.X(), pp.Y(), pp.Z(), electron_mass);
        bus_.set("trk_invM", (ele_p4 + pos_p4).M());
    }
    bus_.set("ele_p_smear_ratio", selected.ele_p_smear_ratio);
    bus_.set("pos_p_smear_ratio", selected.pos_p_smear_ratio);

    // earliest layer hit categories
    bool eleL1{false}, eleL2{false}, eleL3{false}, eleL4{false};
    bool posL1{false}, posL2{false}, posL3{false}, posL4{false};
    Track ele_trk{ele.getTrack()}, pos_trk{pos.getTrack()};
    auto ele_layers = _ah->GetTrackHitLayers(&ele_trk);
    auto pos_layers = _ah->GetTrackHitLayers(&pos_trk);

    if (ele_layers.at(0) == 1 && ele_layers.at(1) == 1) eleL1 = true;
    if (ele_layers.at(2) == 1 && ele_layers.at(3) == 1) eleL2 = true;
    if (ele_layers.at(4) == 1 && ele_layers.at(5) == 1) eleL3 = true;
    if (ele_layers.at(6) == 1 && ele_layers.at(7) == 1) eleL4 = true;
    if (pos_layers.at(0) == 1 && pos_layers.at(1) == 1) posL1 = true;
    if (pos_layers.at(2) == 1 && pos_layers.at(3) == 1) posL2 = true;
    if (pos_layers.at(4) == 1 && pos_layers.at(5) == 1) posL3 = true;
    if (pos_layers.at(6) == 1 && pos_layers.at(7) == 1) posL4 = true;

    bus_.set("eleL1", eleL1);
    bus_.set("eleL2", eleL2);
    bus_.set("eleL3", eleL3);
    bus_.set("eleL4", eleL4);
    bus_.set("posL1", posL1);
    bus_.set("posL2", posL2);
    bus_.set("posL3", posL3);
    bus_.set("posL4", posL4);

    // vertex layer category flags (outward hit requirement: LN track requires L[N..3] hits and no L[1..N-1])
    bool ele_L1 = eleL1 && eleL2 && eleL3;
    bool ele_L2 = !eleL1 && eleL2 && eleL3;
    bool ele_L3 = !eleL1 && !eleL2 && eleL3;
    bool pos_L1 = posL1 && posL2 && posL3;
    bool pos_L2 = !posL1 && posL2 && posL3;
    bool pos_L3 = !posL1 && !posL2 && posL3;

    bus_.set("isL1L1", ele_L1 && pos_L1);
    bus_.set("isL2L2", ele_L2 && pos_L2);
    bus_.set("isL3L3", ele_L3 && pos_L3);
    bus_.set("isL1L2", (ele_L1 && pos_L2) || (pos_L1 && ele_L2));
    bus_.set("isL2L3", (ele_L2 && pos_L3) || (pos_L2 && ele_L3));

    // top/bottom identification
    bool ele_isTop = ele_trk.getTanLambda() > 0;
    bool pos_isTop = pos_trk.getTanLambda() > 0;
    bus_.set("ele_isTop", ele_isTop);
    bus_.set("pos_isTop", pos_isTop);

    // Per-layer axial/stereo hit flags
    // From sensor_locations.txt z-positions:
    //   Top:    even index (0,2,4,6) = axial,  odd index (1,3,5,7) = stereo
    //   Bottom: even index (0,2,4,6) = stereo, odd index (1,3,5,7) = axial
    auto setLayerHits = [&](const std::vector<int>& layers, bool isTop, const std::string& prefix) {
        for (int iL = 0; iL < 4; iL++) {
            int idx_even = 2 * iL;
            int idx_odd  = 2 * iL + 1;
            bool hit_axial  = isTop ? (layers.at(idx_even) == 1) : (layers.at(idx_odd) == 1);
            bool hit_stereo = isTop ? (layers.at(idx_odd) == 1)  : (layers.at(idx_even) == 1);
            bus_.set(prefix + "L" + std::to_string(iL + 1) + "_axial",  hit_axial);
            bus_.set(prefix + "L" + std::to_string(iL + 1) + "_stereo", hit_stereo);
        }
    };
    setLayerHits(ele_layers, ele_isTop, "ele");
    setLayerHits(pos_layers, pos_isTop, "pos");

    double ele_L1_iso{9999.0}, pos_L1_iso{9999.0};
    if (eleL1 && eleL2 && posL1 && posL2) {
        ele_L1_iso = ele_trk.getIsolation(0);
        pos_L1_iso = pos_trk.getIsolation(0);
    }
    bus_.set("ele_L1_iso", ele_L1_iso);
    bus_.set("pos_L1_iso", pos_L1_iso);

    // isolation significance
    double ele_L1_iso_significance{9999.0}, pos_L1_iso_significance{9999.0};

    if (debug_) {
        std::cout << "[iso_sig debug] eleL1=" << eleL1 << " eleL2=" << eleL2
                  << " posL1=" << posL1 << " posL2=" << posL2 << std::endl;
        std::cout << "[iso_sig debug] ele_L1_iso=" << ele_L1_iso
                  << " pos_L1_iso=" << pos_L1_iso << std::endl;
    }

    if (eleL1 && eleL2 && posL1 && posL2) {
        if (ele_L1_iso < 9999.0) {
            double ele_py_sign = TMath::Sign(1, ele_trk.getMomentum()[1]);
            double ele_z0 = ele_trk.getZ0();
            double ele_z0err = ele_trk.getZ0Err();
            ele_L1_iso_significance = (2 * ele_L1_iso + ele_py_sign * ele_z0) / ele_z0err;
            if (debug_) {
                std::cout << "[iso_sig debug] ele: iso=" << ele_L1_iso
                          << " py_sign=" << ele_py_sign
                          << " z0=" << ele_z0
                          << " z0err=" << ele_z0err
                          << " numerator=(2*iso + sign*z0)=" << (2 * ele_L1_iso + ele_py_sign * ele_z0)
                          << " significance=" << ele_L1_iso_significance << std::endl;
            }
        } else if (debug_) {
            std::cout << "[iso_sig debug] ele: iso=9999 (no L1 hit), significance not calculated" << std::endl;
        }

        if (pos_L1_iso < 9999.0) {
            double pos_py_sign = TMath::Sign(1, pos_trk.getMomentum()[1]);
            double pos_z0 = pos_trk.getZ0();
            double pos_z0err = pos_trk.getZ0Err();
            pos_L1_iso_significance = (2 * pos_L1_iso + pos_py_sign * pos_z0) / pos_z0err;
            if (debug_) {
                std::cout << "[iso_sig debug] pos: iso=" << pos_L1_iso
                          << " py_sign=" << pos_py_sign
                          << " z0=" << pos_z0
                          << " z0err=" << pos_z0err
                          << " numerator=(2*iso + sign*z0)=" << (2 * pos_L1_iso + pos_py_sign * pos_z0)
                          << " significance=" << pos_L1_iso_significance << std::endl;
            }
        } else if (debug_) {
            std::cout << "[iso_sig debug] pos: iso=9999 (no L1 hit), significance not calculated" << std::endl;
        }
    } else if (debug_) {
        std::cout << "[iso_sig debug] L1L2 hit requirement not met, significance left at 9999" << std::endl;
    }
    bus_.set("ele_L1_iso_significance", ele_L1_iso_significance);
    bus_.set("pos_L1_iso_significance", pos_L1_iso_significance);

    // Recompute isolation significance using raw (pre-correction) z0
    double ele_L1_iso_significance_raw{9999.0}, pos_L1_iso_significance_raw{9999.0};
    if (debug_) {
        std::cout << "[iso_sig debug] raw z0: ele_z0_raw=" << ele_z0_raw
                  << " pos_z0_raw=" << pos_z0_raw << std::endl;
    }
    if (eleL1 && eleL2 && posL1 && posL2) {
        if (ele_L1_iso < 9999.0) {
            double ele_py_sign = TMath::Sign(1, ele_trk.getMomentum()[1]);
            ele_L1_iso_significance_raw = (2 * ele_L1_iso + ele_py_sign * ele_z0_raw) / ele_trk.getZ0Err();
            if (debug_) {
                std::cout << "[iso_sig debug] ele raw: iso=" << ele_L1_iso
                          << " py_sign=" << ele_py_sign
                          << " z0_raw=" << ele_z0_raw
                          << " z0err=" << ele_trk.getZ0Err()
                          << " numerator=" << (2 * ele_L1_iso + ele_py_sign * ele_z0_raw)
                          << " significance_raw=" << ele_L1_iso_significance_raw << std::endl;
            }
        }
        if (pos_L1_iso < 9999.0) {
            double pos_py_sign = TMath::Sign(1, pos_trk.getMomentum()[1]);
            pos_L1_iso_significance_raw = (2 * pos_L1_iso + pos_py_sign * pos_z0_raw) / pos_trk.getZ0Err();
            if (debug_) {
                std::cout << "[iso_sig debug] pos raw: iso=" << pos_L1_iso
                          << " py_sign=" << pos_py_sign
                          << " z0_raw=" << pos_z0_raw
                          << " z0err=" << pos_trk.getZ0Err()
                          << " numerator=" << (2 * pos_L1_iso + pos_py_sign * pos_z0_raw)
                          << " significance_raw=" << pos_L1_iso_significance_raw << std::endl;
            }
        }
    }
    bus_.set("ele_L1_iso_significance_raw", ele_L1_iso_significance_raw);
    bus_.set("pos_L1_iso_significance_raw", pos_L1_iso_significance_raw);

    TVector3 ele_mom(ele_trk.getMomentum()[0], ele_trk.getMomentum()[1], ele_trk.getMomentum()[2]);
    TVector3 pos_mom(pos_trk.getMomentum()[0], pos_trk.getMomentum()[1], pos_trk.getMomentum()[2]);
    TVector3 psum = ele_mom + pos_mom;

    if (debug_) {
        std::cout << "Preselection psum calculation:" << std::endl;
        std::cout << "  Electron: px=" << ele_mom.X() << " py=" << ele_mom.Y() << " pz=" << ele_mom.Z()
                  << " |p|=" << ele_mom.Mag() << std::endl;
        std::cout << "  Positron: px=" << pos_mom.X() << " py=" << pos_mom.Y() << " pz=" << pos_mom.Z()
                  << " |p|=" << pos_mom.Mag() << std::endl;
        std::cout << "  Psum:     px=" << psum.X() << " py=" << psum.Y() << " pz=" << psum.Z()
                  << " |psum|=" << psum.Mag() << std::endl;
    }

    bus_.set("psum", psum.Mag());
    bus_.set("psum_scalar", ele_mom.Mag() + pos_mom.Mag());
    bus_.set("epem_opening_angle", ele_mom.Angle(pos_mom));
    bus_.set("recoil_theta", calculate_theta_R(ele_mom, pos_mom));

    Track thirdTrack = createInferredTrack(&vtx, &ele_trk, &pos_trk);

    // calculate target projection and its significance
    if (not v0proj_fits_.empty()) {
        double vtx_proj_x{-1.0}, vtx_proj_y{-1.0};
        double vtx_proj_x_sig{-1.0}, vtx_proj_y_sig{-1.0};
        double vtx_proj_x_centered{-999.0}, vtx_proj_y_centered{-999.0};
        double vtx_proj_sig{-1.0};

        TVector3 vtx_p = vtx.getP();
        vtx_proj_sig = utils::v0_projection_to_target_significance(v0proj_fits_, eh.getRunNumber(), vtx_proj_x,
                                                                   vtx_proj_y, vtx_proj_x_sig, vtx_proj_y_sig,
                                                                   vtx_proj_x_centered, vtx_proj_y_centered,
                                                                   vtx.getX(), vtx.getY(), vtx.getZ(),
                                                                   vtx_p.X(), vtx_p.Y(), vtx_p.Z(),
                                                                   debug_);

        //utils::debug_target_projection(-1.1, &vtx);

        bus_.set("vtx_proj_sig", vtx_proj_sig);
        bus_.set("vtx_proj_x", vtx_proj_x);
        bus_.set("vtx_proj_x_sig", vtx_proj_x_sig);
        bus_.set("vtx_proj_y", vtx_proj_y);
        bus_.set("vtx_proj_y_sig", vtx_proj_y_sig);
        bus_.set("vtx_proj_x_centered", vtx_proj_x_centered);
        bus_.set("vtx_proj_y_centered", vtx_proj_y_centered);

        double vtx_proj_x_lcio{-1.0}, vtx_proj_y_lcio{-1.0};
        double vtx_proj_x_sig_lcio{-1.0}, vtx_proj_y_sig_lcio{-1.0};
        double vtx_proj_x_err_lcio{-1.0}, vtx_proj_y_err_lcio{-1.0};
        double vtx_proj_x_centered_lcio{-999.0}, vtx_proj_y_centered_lcio{-999.0};
        double vtx_proj_sig_lcio = utils::v0_projection_to_target_significance(
                v0proj_fits_, eh.getRunNumber(),
                vtx_proj_x_lcio, vtx_proj_y_lcio,
                vtx_proj_x_sig_lcio, vtx_proj_y_sig_lcio,
                &vtx, vtx_proj_x_err_lcio, vtx_proj_y_err_lcio,
                vtx_proj_x_centered_lcio, vtx_proj_y_centered_lcio, debug_);

        bus_.set("vtx_proj_sig_lcio", vtx_proj_sig_lcio);
        bus_.set("vtx_proj_x_lcio", vtx_proj_x_lcio);
        bus_.set("vtx_proj_x_sig_lcio", vtx_proj_x_sig_lcio);
        bus_.set("vtx_proj_y_lcio", vtx_proj_y_lcio);
        bus_.set("vtx_proj_y_sig_lcio", vtx_proj_y_sig_lcio);
        bus_.set("vtx_proj_x_err_lcio", vtx_proj_x_err_lcio);
        bus_.set("vtx_proj_y_err_lcio", vtx_proj_y_err_lcio);
        bus_.set("vtx_proj_x_centered_lcio", vtx_proj_x_centered_lcio);
        bus_.set("vtx_proj_y_centered_lcio", vtx_proj_y_centered_lcio);
    }

    // vertical impact parameters
    double min_y0{9999.0}, max_y0err{-1.0}, min_y0err{-1.0};

    if (fabs(ele_trk.getZ0()) < fabs(pos_trk.getZ0())) {
        min_y0 = ele_trk.getZ0();
    } else {
        min_y0 = pos_trk.getZ0();
    }
    min_y0 = fabs(min_y0);

    max_y0err = std::max(ele_trk.getZ0Err(), pos_trk.getZ0Err());
    min_y0err = std::min(ele_trk.getZ0Err(), pos_trk.getZ0Err());

    // min z0 distance to vertex y directly
    double vtx_y = vtx.getY();
    double min_y0_vtx = std::min(fabs(ele_trk.getZ0() - vtx_y), fabs(pos_trk.getZ0() - vtx_y));

    // min z0 distance to vertex y projected back to z=0 along vertex momentum
    // solve z(t)=0: t = -vtx_z / vtx_pz, then y_proj = vtx_y + py*t
    TVector3 vtx_mom = vtx.getP();
    double vtx_y_at_zero = (vtx_mom.Z() != 0.0)
        ? vtx_y - (vtx_mom.Y() / vtx_mom.Z()) * vtx.getZ()
        : vtx_y;
    double ele_y0_proj = fabs(ele_trk.getZ0() - vtx_y_at_zero);
    double pos_y0_proj = fabs(pos_trk.getZ0() - vtx_y_at_zero);
    double min_y0_vtx_proj  = std::min(ele_y0_proj, pos_y0_proj);
    double delta_y0_vtx_proj = ele_y0_proj - pos_y0_proj;

    bus_.set("min_y0", min_y0);
    bus_.set("max_y0err", max_y0err);
    bus_.set("min_y0err", min_y0err);
    bus_.set("ele_y0err", ele_trk.getZ0Err());
    bus_.set("pos_y0err", pos_trk.getZ0Err());
    bus_.set("min_y0_vtx", min_y0_vtx);
    bus_.set("min_y0_vtx_proj", min_y0_vtx_proj);
    bus_.set("vtx_y_at_zero", vtx_y_at_zero);
    bus_.set("delta_y0_vtx_proj", delta_y0_vtx_proj);

    // set vertex object and tracks
    bus_.set("weight", eh.getWeight());
    bus_.set("vertex", vtx);
    bus_.set("ele", ele);
    bus_.set("pos", pos);
    bus_.set("third_track", thirdTrack);

    //set multi-track quantities
    if (calcMultiTrackVars_ && bus_.has(trkColl_)) {
        const auto& all_tracks = bus_.get<std::vector<Track*>>(trkColl_);
        calculatePairwiseQuantities(all_tracks, ele, pos);
    } else if (saveAllTracks_ && bus_.has(trkColl_)) {
        // if we only want to save tracks but not calculate pairwise quantities
        const auto& all_tracks = bus_.get<std::vector<Track*>>(trkColl_);
        std::vector<Track> all_good_tracks;
        
        for (auto* trk : all_tracks) {
            if (isQualityTrack(trk, pos)) {
                all_good_tracks.push_back(*trk);
            }
        }
        
        bus_.set("all_preselected_tracks", all_good_tracks);
    }
    

    /**
     * This is where the output TTree is filled,
     * if we leave before this point, then the event will not
     * be kept as part of pre-selection.
     *
     * We wait until here to copy over the MCParticles to avoid
     * unnecessary copying if the event is not going to be kept
     * anyways.
     */
    bus_.set("ele_truth_p", TVector3(0., 0., 0.));
    bus_.set("pos_truth_p", TVector3(0., 0., 0.));
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
                if (ptr->getPDG() == apPDG_) {
                    n_ap++;
                    ap = ptr;
                } else if (ptr->getID() == truth_ele_id) {
                    ele_is_rad_ele = (ptr->getMomPDG() == apPDG_);
                    trueEleP = ROOT::Math::PxPyPzEVector(lP.at(0), lP.at(1), lP.at(2), ptr->getEnergy());
                } else if (ptr->getPDG() == -11 && ptr->getMomPDG() == apPDG_) {
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

        if (debug_) {
            std::cout << "[AP debug] MCParticle PDGs in event (run=" << run_number << "):" << std::endl;
            for (MCParticle* ptr : mc_ptr) {
                std::cout << "  PDG=" << ptr->getPDG()
                          << "  momPDG=" << ptr->getMomPDG()
                          << "  id=" << ptr->getID()
                          << "  p=" << ptr->getMomentum()[2] << std::endl;
            }
            std::cout << "[AP debug] searching for apPDG=" << apPDG_ << "  n_ap=" << n_ap << "  ap=" << (ap ? "found" : "null") << std::endl;
        }

        bus_.set("ele_truth_p", TVector3(trueEleP.Px(), trueEleP.Py(), trueEleP.Pz()));
        bus_.set("pos_truth_p", TVector3(truePosP.Px(), truePosP.Py(), truePosP.Pz()));

        /**
         * Truth-link flags for the selected vertex. Computed here rather than in
         * the vertex loop above: that loop runs over every raw vertex, so a
         * bus_.set() from inside it describes the last candidate iterated rather
         * than the one eventually selected.
         */
        bus_.set("ele_has_truth_link", utils::hasTruthMatch(ele_trk, &mc_ptr, debug_));
        bus_.set("pos_has_truth_link", utils::hasTruthMatch(pos_trk, &mc_ptr, debug_));

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

            std::vector<double> vdMom = vd->getMomentum();
            double vd_p = std::sqrt(vdMom[0]*vdMom[0] + vdMom[1]*vdMom[1] + vdMom[2]*vdMom[2]);
            bus_.set("true_vd_betagamma", vd_p / vd->getMass());

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

            // Compute lab-frame decay distance and Lorentz boost for lifetime reweighting
            std::vector<double> vtxPos = ap->getVertexPosition();
            std::vector<double> endPos = ap->getEndPoint();
            double dx = endPos[0] - vtxPos[0];
            double dy = endPos[1] - vtxPos[1];
            double dz = endPos[2] - vtxPos[2];
            bus_.set("true_decay_len", std::sqrt(dx*dx + dy*dy + dz*dz));

            std::vector<double> apMom = ap->getMomentum();
            double ap_p = std::sqrt(apMom[0]*apMom[0] + apMom[1]*apMom[1] + apMom[2]*apMom[2]);
            bus_.set("true_ap_betagamma", ap_p / ap->getMass());
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
    multivtx_chosen_chi2_h_->Write();
    multivtx_discarded_chi2_h_->Write();
    vertex_cf_.save();
    event_cf_.save();
    outF_->Close();
}

DECLARE_PROCESSOR(PreselectAndCategorize2021);
