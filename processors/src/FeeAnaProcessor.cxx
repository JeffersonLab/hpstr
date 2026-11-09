#include "FeeAnaProcessor.h"
#include <iomanip>
#include <cmath>
#include <algorithm>
#include <fstream>
#include "utilities.h"

FeeAnaProcessor::FeeAnaProcessor(const std::string& name, Process& process)
    : Processor(name, process) {
    }

FeeAnaProcessor::~FeeAnaProcessor() {
}

void FeeAnaProcessor::configure(const ParameterSet& parameters) {

    std::cout << "Configuring FeeAnaProcessor" << std::endl;
    try
    {
        debug_                = parameters.getInteger("debug",debug_);
        seed_                 = parameters.getInteger("seed",seed_);
        trkCollName_          = parameters.getString("trkCollName",trkCollName_);
        fspCollName_          = parameters.getString("fspCollName",fspCollName_);
        mcColl_               = parameters.getString("mcColl",mcColl_);
        histCfgFilename_      = parameters.getString("histCfg",histCfgFilename_);
        doTruth_              = (bool) parameters.getInteger("doTruth",doTruth_);
        truthHistCfgFilename_ = parameters.getString("truthHistCfg",truthHistCfgFilename_);
        selectionCfg_         = parameters.getString("selectionjson",selectionCfg_);
        isData_               = parameters.getInteger("isData",isData_);
        regionSelections_     = parameters.getVString("regionDefinitions",regionSelections_);

        //Momentum smearing closure test
        pSmearingFile_            = parameters.getString("pSmearingFile",pSmearingFile_);
        smearingCfgFile_          = parameters.getString("smearingCfg",smearingCfgFile_);

        // Master switch for smearing (default false for backward compatibility)
        doSmearing_ = parameters.getInteger("doSmearing", 0) != 0;

        // Factor to multiply smearing parameters by (default 1.0)
        smearingFactor_ = parameters.getDouble("smearingFactor", 1.0);

        // Require truth match for smearing (default false)
        requireTruthMatch_ = parameters.getInteger("requireTruthMatch", 0) != 0;

        // Use omega (curvature) smearing instead of p smearing (default false)
        smearOmega_ = parameters.getInteger("smearOmega", 0) != 0;

        // Explicit smearing lookup variable: "flat", "nHits", "tanLambda", "phi0"
        // Empty string (default) accepts whatever the JSON specifies
        smearingVariable_ = parameters.getString("smearingVariable", "");
        scaleCorrVariable_ = parameters.getString("scaleCorrVariable", "");

        // Per-hit_pattern momentum scale correction (data -> beam energy)
        hitPatternScaleFile_     = parameters.getString("hitPatternScaleFile", hitPatternScaleFile_);
        doHitPatternScale_       = parameters.getInteger("doHitPatternScale", doHitPatternScale_ ? 1 : 0) != 0;
        hitPatternScaleDataOnly_ = parameters.getInteger("hitPatternScaleDataOnly", hitPatternScaleDataOnly_ ? 1 : 0) != 0;

        // FEE selection
        feeClusterEnergyMin_ = parameters.getDouble("feeClusterEnergyMin", feeClusterEnergyMin_);
        clusterTimeMin_      = parameters.getDouble("clusterTimeMin",      clusterTimeMin_);
        clusterTimeMax_      = parameters.getDouble("clusterTimeMax",      clusterTimeMax_);
        clusterTimeMinMC_    = parameters.getDouble("clusterTimeMinMC",    clusterTimeMinMC_);
        clusterTimeMaxMC_    = parameters.getDouble("clusterTimeMaxMC",    clusterTimeMaxMC_);
        calTimeOffset_       = parameters.getDouble("CalTimeOffset",       calTimeOffset_);
        calTimeOffsetMC_     = parameters.getDouble("CalTimeOffsetMC",     calTimeOffsetMC_);
        mcTimeOffset_        = parameters.getDouble("mcTimeOffset",        mcTimeOffset_);
        requireElectron_     = parameters.getInteger("requireElectron", requireElectron_ ? 1 : 0) != 0;
        requireCluster_      = parameters.getInteger("requireCluster",  requireCluster_  ? 1 : 0) != 0;
        requireFeeTrigger_   = parameters.getInteger("requireFeeTrigger", requireFeeTrigger_ ? 1 : 0) != 0;
        eopMin_              = parameters.getDouble("eopMin", eopMin_);
        eopMax_              = parameters.getDouble("eopMax", eopMax_);

        // In-time isolation veto
        vetoExtraClusters_    = parameters.getInteger("vetoExtraClusters", vetoExtraClusters_ ? 1 : 0) != 0;
        vetoExtraTracks_      = parameters.getInteger("vetoExtraTracks",   vetoExtraTracks_   ? 1 : 0) != 0;
        vetoClusterTimeWindow_ = parameters.getDouble("vetoClusterTimeWindow", vetoClusterTimeWindow_);
        vetoTrackTimeWindow_   = parameters.getDouble("vetoTrackTimeWindow",   vetoTrackTimeWindow_);
        vetoClusterEnergyMin_  = parameters.getDouble("vetoClusterEnergyMin",  vetoClusterEnergyMin_);

    }
    catch (std::runtime_error& error)
    {
        std::cout << error.what() << std::endl;
    }

    if (!isData_)
      time_offset_ = mcTimeOffset_;

    // Load the per-hit_pattern momentum scale corrections (data -> beam energy).
    if (doHitPatternScale_ && !hitPatternScaleFile_.empty()) {
      std::ifstream ifs(hitPatternScaleFile_);
      if (!ifs.good()) {
        std::cout << "[FeeAnaProcessor] ERROR: cannot open hitPatternScaleFile "
                  << hitPatternScaleFile_ << " -- hit-pattern scale correction DISABLED" << std::endl;
        doHitPatternScale_ = false;
      } else {
        nlohmann::json j;
        ifs >> j;
        hitPatternDefaultScale_ = j.value("default_scale", 1.0);
        if (j.contains("pattern_to_scale")) {
          const nlohmann::json& p2s = j["pattern_to_scale"];
          for (auto it = p2s.begin(); it != p2s.end(); ++it)
            hitPatternScale_[std::stoi(it.key())] = it.value().get<double>();
        }
        std::cout << "[FeeAnaProcessor] Loaded " << hitPatternScale_.size()
                  << " hit-pattern scale corrections (default_scale=" << hitPatternDefaultScale_
                  << (hitPatternScaleDataOnly_ ? ", data-only" : ", data+MC")
                  << ") from " << hitPatternScaleFile_ << std::endl;
      }
    }
}

void FeeAnaProcessor::setFile(TFile* outFile) {
    Processor::setFile(outFile);
    output_tree_ = std::make_unique<TTree>("tracks", "Selected FEE Tracks", 99, outFile);
    output_tree_->Branch("track.",         &track_out_,         100000, 3);
    output_tree_->Branch("track_smeared.", &track_smeared_out_, 100000, 3);
    output_tree_->Branch("p_smear_ratio",  &p_smear_ratio_out_);
    output_tree_->Branch("clu_E",     &clu_E_out_);
    output_tree_->Branch("clu_time",  &clu_time_out_);
    output_tree_->Branch("eop",       &eop_out_);
    output_tree_->Branch("n_clusters_intime", &n_clusters_intime_out_);
    output_tree_->Branch("n_tracks_intime",   &n_tracks_intime_out_);
    output_tree_->Branch("L1_axial",  &L1_axial_out_);
    output_tree_->Branch("L1_stereo", &L1_stereo_out_);
    output_tree_->Branch("L2_axial",  &L2_axial_out_);
    output_tree_->Branch("L2_stereo", &L2_stereo_out_);
    output_tree_->Branch("L3_axial",  &L3_axial_out_);
    output_tree_->Branch("L3_stereo", &L3_stereo_out_);
    output_tree_->Branch("L4_axial",  &L4_axial_out_);
    output_tree_->Branch("L4_stereo", &L4_stereo_out_);
    output_tree_->Branch("L1",    &L1_out_);
    output_tree_->Branch("L2",    &L2_out_);
    output_tree_->Branch("L3",    &L3_out_);
    output_tree_->Branch("L4",    &L4_out_);
    output_tree_->Branch("is_L1", &is_L1_out_);
    output_tree_->Branch("is_L2", &is_L2_out_);
    output_tree_->Branch("is_L3", &is_L3_out_);
    output_tree_->Branch("hit_pattern", &hit_pattern_out_);
    output_tree_->Branch("n_hits",      &n_hits_out_);
    output_tree_->Branch("track_corr.", &track_corr_out_, 100000, 3);
    output_tree_->Branch("hit_pattern_scale",            &hit_pattern_scale_out_);
    output_tree_->Branch("hit_pattern_scale_isdefault",  &hit_pattern_scale_isdefault_out_);
    output_tree_->Branch("truth_matched",  &truth_matched_out_);
    output_tree_->Branch("has_truth_link", &has_truth_link_out_);
}

void FeeAnaProcessor::initialize(TTree* tree) {

    ah_ = std::make_shared<AnaHelpers>();

    //Init histos
    trkHistos_ = new TrackHistos(trkCollName_);
    trkHistos_->loadHistoConfig(histCfgFilename_);
    trkHistos_->doTrackComparisonPlots(false);
    trkHistos_->DefineHistos();

    // FEE tracks come from the FinalStateParticle collection, where the track and its
    // ECal cluster are already associated at the LCIO stage (no manual matching needed).
    tree->SetBranchAddress(fspCollName_.c_str(), &fsps_, &bfsps_);

    if (!selectionCfg_.empty()) {
        trkSelector_ = std::make_shared<BaseSelector>(name_+"_trkSelector",selectionCfg_);
        trkSelector_->setDebug(debug_);
        trkSelector_->LoadSelection();
    }

    if (doTruth_) {
      truthHistos_ = new TrackHistos(trkCollName_+"_truthComparison");
        truthHistos_->loadHistoConfig(histCfgFilename_);
        truthHistos_->DefineHistos();
        truthHistos_->loadHistoConfig(truthHistCfgFilename_);
        truthHistos_->DefineHistos();
        truthHistos_->doTrackComparisonPlots(false);
    }

    // Setup track selections plots
    for (unsigned int i_reg = 0;
         i_reg < regionSelections_.size();
         i_reg++) {
      std::string regname = AnaHelpers::getFileName(regionSelections_[i_reg],false);
      std::cout<< "Setting up region "<< regname<<std::endl;

      reg_selectors_[regname] = std::make_shared<BaseSelector>(regname, regionSelections_[i_reg]);
      reg_selectors_[regname]->setDebug(false);
      reg_selectors_[regname]->LoadSelection();

      reg_histos_[regname] = std::make_shared<TrackHistos>(regname);
      reg_histos_[regname]->loadHistoConfig(histCfgFilename_);
      reg_histos_[regname]->doTrackComparisonPlots(false);
      reg_histos_[regname]->DefineTrkHitHistos();

      regions_.push_back(regname);

    }

    //Get event header information for trigger
    tree->SetBranchAddress("EventHeader", &evth_ , &bevth_);

    // 2021 trigger-scaler bank, needed to require the FEE trigger (data only).
    if (requireFeeTrigger_)
        tree->SetBranchAddress("TSBank", &tsdata_, &btsdata_);

    // Full event collections, used to count in-time tracks/clusters (and optionally veto).
    // These are always branched in so the in-time multiplicity is recorded in the output
    // tree regardless of whether the veto is applied.
    tree->SetBranchAddress("RecoEcalClusters", &ecalClusters_, &becalClusters_);
    tree->SetBranchAddress(trkCollName_.c_str(), &allTracks_, &ballTracks_);

    // MCParticle collection for hit-based truth matching (MC only; absent on data).
    if (!isData_ && !mcColl_.empty())
        tree->SetBranchAddress(mcColl_.c_str(), &mcParticles_, &bmcParticles_);

    //Momentum smearing closure test
    // Determine which smearing file to use (smearingCfg takes precedence)
    std::string smearingFile = !smearingCfgFile_.empty() ? smearingCfgFile_ : pSmearingFile_;

    if (!smearingFile.empty()) {
      std::cout<<"Loading smearing config from "<<smearingFile<<std::endl;
      std::cout<<"Smearing Tool Seed "<<seed_<<std::endl;
      std::cout<<"Using smearing factor: "<<smearingFactor_<<std::endl;
      std::cout<<"doSmearing: "<<(doSmearing_ ? "true" : "false")<<std::endl;
      std::cout<<"smearOmega: "<<(smearOmega_ ? "true" : "false")<<std::endl;
      // Match PreselectAndCategorize2021: relSmearingP=true (relative), relSmearingZ0=false (absolute)
      // JSON files will override with their own relSmearingP/relSmearingZ0 values
      smearingTool_ = std::make_shared<TrackSmearingTool>(smearingFile, true, false, seed_, trkCollName_, smearingFactor_);
      smearingTool_->setIsData(isData_);
      smearingTool_->setApplyMeanCorr(isData_);
      smearingTool_->setDebug(debug_ > 0);
      smearingTool_->setForcedVariable(smearingVariable_);
      if (!scaleCorrVariable_.empty())
        smearingTool_->setScaleCorrVariable(scaleCorrVariable_);
      smearingTool_->setRequireTruthMatch(requireTruthMatch_);
      smearingTool_->printConfig();

      psmear_h_     =   new TH1D("psmear_h",
                                 "psmear_h",200,2,6);
      psmear_top_h_ =   new TH1D("psmear_top_h",
                                 "psmear_top_h",200,2,6);
      psmear_bot_h_ =   new TH1D("psmear_bot_h",
                                 "psmear_bot_h",200,2,6);

      psmear_vs_nHits_hh_ =  new TH2D("psmear_vs_nHits_hh",
                                      "psmear_vs_nHits_hh",
                                      5,8,13,
                                      200,0,6);
      psmear_vs_nHits_top_hh_ =  new TH2D("psmear_vs_nHits_top_hh",
                                          "psmear_vs_nHits_top_hh",
                                          5,8,13,
                                          200,0,6);
      psmear_vs_nHits_bot_hh_ =  new TH2D("psmear_vs_nHits_bot_hh",
                                          "psmear_vs_nHits_bot_hh",
                                          5,8,13,
                                          200,0,6);


      psmear_rel_h_     =   new TH1D("psmear_rel_h",
                                     "psmear_rel_h",200,0,6);

      psmear_vs_nHits_rel_hh_ =  new TH2D("psmear_vs_nHits_rel_hh",
                                          "psmear_vs_nHits_rel_hh",
                                          5,8,13,
                                          200,0,6);
      psmear_vs_nHits_top_rel_hh_ =  new TH2D("psmear_vs_nHits_top_rel_hh",
                                              "psmear_vs_nHits_top_rel_hh",
                                              5,8,13,
                                              200,0,6);
      psmear_vs_nHits_bot_rel_hh_ =  new TH2D("psmear_vs_nHits_bot_rel_hh",
                                              "psmear_vs_nHits_bot_rel_hh",
                                              5,8,13,
                                              200,0,6);

      // z0 smearing validation histograms (binning matches Z0_h from feeSmearing_2021.json)
      z0smear_h_     = new TH1D("z0smear_h", "z0smear_h", 100, -0.5, 0.5);
      z0smear_top_h_ = new TH1D("z0smear_top_h", "z0smear_top_h", 100, -0.5, 0.5);
      z0smear_bot_h_ = new TH1D("z0smear_bot_h", "z0smear_bot_h", 100, -0.5, 0.5);

      // omega smearing validation histograms
      omega_h_           = new TH1D("omega_h", "omega_h", 100, -0.00015, 0.00015);
      omega_top_h_       = new TH1D("omega_top_h", "omega_top_h", 100, -0.00015, 0.00015);
      omega_bot_h_       = new TH1D("omega_bot_h", "omega_bot_h", 100, -0.00015, 0.00015);
      omegasmear_h_      = new TH1D("omegasmear_h", "omegasmear_h", 100, -0.00015, 0.00015);
      omegasmear_top_h_  = new TH1D("omegasmear_top_h", "omegasmear_top_h", 100, -0.00015, 0.00015);
      omegasmear_bot_h_  = new TH1D("omegasmear_bot_h", "omegasmear_bot_h", 100, -0.00015, 0.00015);
      omega_vs_p_hh_     = new TH2D("omega_vs_p_hh", "omega_vs_p_hh", 100, 1.0, 4.5, 100, -0.00015, 0.00015);

      // omega vs tanLambda and phi0 (unsmeared and smeared)
      omega_vs_tanL_hh_          = new TH2D("omega_vs_tanL_hh",          "omega_vs_tanL_hh",          80, -0.08, 0.08, 100, -0.00015, 0.00015);
      omega_vs_tanL_top_hh_      = new TH2D("omega_vs_tanL_top_hh",      "omega_vs_tanL_top_hh",      40,  0.00, 0.08, 100, -0.00015, 0.00015);
      omega_vs_tanL_bot_hh_      = new TH2D("omega_vs_tanL_bot_hh",      "omega_vs_tanL_bot_hh",      40, -0.08, 0.00, 100, -0.00015, 0.00015);
      omegasmear_vs_tanL_hh_     = new TH2D("omegasmear_vs_tanL_hh",     "omegasmear_vs_tanL_hh",     80, -0.08, 0.08, 100, -0.00015, 0.00015);
      omegasmear_vs_tanL_top_hh_ = new TH2D("omegasmear_vs_tanL_top_hh", "omegasmear_vs_tanL_top_hh", 40,  0.00, 0.08, 100, -0.00015, 0.00015);
      omegasmear_vs_tanL_bot_hh_ = new TH2D("omegasmear_vs_tanL_bot_hh", "omegasmear_vs_tanL_bot_hh", 40, -0.08, 0.00, 100, -0.00015, 0.00015);

      omega_vs_phi0_hh_          = new TH2D("omega_vs_phi0_hh",          "omega_vs_phi0_hh",          80, -0.2, 0.2, 100, -0.00015, 0.00015);
      omega_vs_phi0_top_hh_      = new TH2D("omega_vs_phi0_top_hh",      "omega_vs_phi0_top_hh",      80, -0.2, 0.2, 100, -0.00015, 0.00015);
      omega_vs_phi0_bot_hh_      = new TH2D("omega_vs_phi0_bot_hh",      "omega_vs_phi0_bot_hh",      80, -0.2, 0.2, 100, -0.00015, 0.00015);
      omegasmear_vs_phi0_hh_     = new TH2D("omegasmear_vs_phi0_hh",     "omegasmear_vs_phi0_hh",     80, -0.2, 0.2, 100, -0.00015, 0.00015);
      omegasmear_vs_phi0_top_hh_ = new TH2D("omegasmear_vs_phi0_top_hh", "omegasmear_vs_phi0_top_hh", 80, -0.2, 0.2, 100, -0.00015, 0.00015);
      omegasmear_vs_phi0_bot_hh_ = new TH2D("omegasmear_vs_phi0_bot_hh", "omegasmear_vs_phi0_bot_hh", 80, -0.2, 0.2, 100, -0.00015, 0.00015);

    }


}

bool FeeAnaProcessor::process(IEvent* ievent) {

    double weight = 1.;
    int n_sel_tracks = 0;

    double minTime   = isData_ ? clusterTimeMin_  : clusterTimeMinMC_;
    double maxTime   = isData_ ? clusterTimeMax_  : clusterTimeMaxMC_;
    double calOffset = isData_ ? calTimeOffset_   : calTimeOffsetMC_;

    if (debug_ > 0) {
        std::cout << "[FeeAna] --- new event  isData=" << isData_
                  << "  feeEnergyMin=" << feeClusterEnergyMin_
                  << "  timeWindow=[" << minTime << "," << maxTime << "]"
                  << "  nFSPs=" << fsps_->size() << std::endl;
    }

    // Require the FEE trigger on data. MC is generated with a single trigger and the
    // TSBank bits are not filled, so the requirement is applied to data only (mirrors
    // the trigger handling in PreselectAndCategorize2021).
    if (requireFeeTrigger_ && isData_) {
        if (!tsdata_ || !tsdata_->isFEETrigger()) {
            if (debug_ > 0) std::cout << "[FeeAna]   FAIL FEE trigger" << std::endl;
            return true;
        }
    }

    // FEE selection: loop over FinalStateParticles. Each particle carries its track and
    // (if matched at the LCIO stage) its ECal cluster, so no manual track-cluster matching
    // is needed. We require an electron whose associated cluster looks like a full-energy
    // electron (energy / time / E-over-p), then apply the usual track-quality cuts.
    for (unsigned int ipart = 0; ipart < fsps_->size(); ++ipart) {

        if (trkSelector_) trkSelector_->getCutFlowHisto()->Fill(0.,weight);

        Particle* part = fsps_->at(ipart);

        // Electron requirement (FEEs are electrons; charge < 0 in HPS convention)
        if (requireElectron_ && part->getCharge() >= 0) {
            if (debug_ > 0) std::cout << "[FeeAna]   FAIL not an electron: charge=" << part->getCharge() << std::endl;
            continue;
        }

        // Associated FEE cluster (already matched to the track at the LCIO stage).
        // A particle with no associated cluster has the sentinel cluster energy.
        CalCluster clu  = part->getCluster();
        double clu_E    = clu.getEnergy();
        double clu_traw = clu.getTime();          // raw cluster time
        double clu_t    = clu_traw - calOffset;   // offset-corrected (centered near 0)

        // DIAGNOSTIC: dump cluster time for every electron with a full-energy cluster,
        // *before* the time cut, to verify the CalTimeOffset centers the prompt peak.
        if (debug_ > 0 && part->getCharge() < 0 && clu_E > feeClusterEnergyMin_) {
            std::cout << "[FeeAna][cluTime] rawCluTime=" << clu_traw
                      << "  corrCluTime=" << clu_t
                      << "  cluE=" << clu_E
                      << "  trkTime=" << part->getTrack().getTrackTime()
                      << std::endl;
        }

        if (requireCluster_) {
            if (clu_E < feeClusterEnergyMin_) {
                if (debug_ > 0) std::cout << "[FeeAna]   FAIL cluster E: " << clu_E << " < " << feeClusterEnergyMin_ << std::endl;
                continue;
            }
            if (clu_t < minTime || clu_t > maxTime) {
                if (debug_ > 0) std::cout << "[FeeAna]   FAIL cluster time (corrected): " << clu_t << " not in [" << minTime << "," << maxTime << "]" << std::endl;
                continue;
            }
        }

        // Track associated to this particle (returned by value)
        Track  track_obj = part->getTrack();
        Track* track     = &track_obj;
        int n2dhits_onTrack = !track->isKalmanTrack() ? track->getTrackerHitCount() * 2 : track->getTrackerHitCount();

        // E/p of the FEE candidate
        double eop = track->getP() > 0 ? clu_E / track->getP() : -1.0;
        if (requireCluster_ && (eop < eopMin_ || eop > eopMax_)) {
            if (debug_ > 0) std::cout << "[FeeAna]   FAIL E/p: " << eop << " not in [" << eopMin_ << "," << eopMax_ << "]" << std::endl;
            continue;
        }

        TVector3 trk_mom;
        trk_mom.SetX(track->getMomentum()[0]);
        trk_mom.SetY(track->getMomentum()[1]);
        trk_mom.SetZ(track->getMomentum()[2]);

        // Decode per-sensor hit layer vector and determine top/bottom.
        bool isTop = track->getTanLambda() > 0;
        // The FEE track comes from the FinalStateParticle, whose embedded Track does NOT
        // carry the per-sensor hit-layer vector: FinalStateParticleProcessor fills the hit
        // count and the SvtHit refs but never calls addHitLayer(), so getHitLayers() is
        // empty here. The matching KalmanFullTracks entry (same track ID) does carry it, so
        // match by ID and decode the layers from that. Fall back to the FSP track's own
        // SvtHits if no match is found, and finally to the (empty) FSP layer vector.
        Track* layerTrack = track;
        if (allTracks_) {
            for (Track* t : *allTracks_) {
                if (t && t->getID() == track->getID()) { layerTrack = t; break; }
            }
        }
        std::vector<int> layers = ah_->GetTrackHitLayers(layerTrack);
        if (std::none_of(layers.begin(), layers.end(), [](int v){ return v == 1; })) {
            // Matched full track had no layer vector; decode from this track's SvtHits.
            TRefArray hits = track->getSvtHits();
            for (int ih = 0; ih < hits.GetEntries(); ++ih) {
                TrackerHit* th = (TrackerHit*) hits.At(ih);
                if (th && th->getLayer() >= 0 && th->getLayer() < (int) layers.size())
                    layers.at(th->getLayer()) = 1;
            }
        }
        // Encode the full per-sensor hit pattern as a bitmask (bit i = sensor slot i hit).
        int hit_pattern = 0;
        for (int i = 0; i < (int) layers.size(); ++i)
            if (layers.at(i) == 1) hit_pattern |= (1 << i);

        // Per-hit_pattern momentum scale correction (data momentum -> beam energy).
        // p_corr = p * scale, keyed by hit_pattern (default_scale for unlisted patterns).
        double hitPatternScale = 1.0;
        bool   hitPatternScaleIsDefault = false;
        if (doHitPatternScale_ && (!hitPatternScaleDataOnly_ || isData_)) {
            auto it = hitPatternScale_.find(hit_pattern);
            if (it != hitPatternScale_.end()) {
                hitPatternScale = it->second;
            } else {
                hitPatternScale = hitPatternDefaultScale_;
                hitPatternScaleIsDefault = true;
            }
        }
        // Corrected track: scale the momentum magnitude by hitPatternScale (omega scales as
        // 1/scale since p is inversely proportional to |omega|).
        Track trk_corr = *track;
        if (hitPatternScale != 1.0) {
            std::vector<double> mom = trk_corr.getMomentum();
            for (double& c : mom) c *= hitPatternScale;
            trk_corr.setMomentum(mom);
            trk_corr.setOmega(trk_corr.getOmega() / hitPatternScale);
        }

        // Convention from sensor_locations.txt:
        //   Top:    even sensor index = axial,  odd = stereo
        //   Bottom: even sensor index = stereo, odd = axial
        bool L1_axial  = isTop ? (layers.at(0) == 1) : (layers.at(1) == 1);
        bool L1_stereo = isTop ? (layers.at(1) == 1) : (layers.at(0) == 1);
        bool L2_axial  = isTop ? (layers.at(2) == 1) : (layers.at(3) == 1);
        bool L2_stereo = isTop ? (layers.at(3) == 1) : (layers.at(2) == 1);
        bool L3_axial  = isTop ? (layers.at(4) == 1) : (layers.at(5) == 1);
        bool L3_stereo = isTop ? (layers.at(5) == 1) : (layers.at(4) == 1);
        bool L4_axial  = isTop ? (layers.at(6) == 1) : (layers.at(7) == 1);
        bool L4_stereo = isTop ? (layers.at(7) == 1) : (layers.at(6) == 1);

        // Per-layer "has both axial+stereo hit" flags (top/bottom independent),
        // matching the eleLN/posLN definition in PreselectAndCategorize2021.
        bool L1 = (layers.at(0) == 1 && layers.at(1) == 1);
        bool L2 = (layers.at(2) == 1 && layers.at(3) == 1);
        bool L3 = (layers.at(4) == 1 && layers.at(5) == 1);
        bool L4 = (layers.at(6) == 1 && layers.at(7) == 1);

        // Earliest-layer category with the outward hit requirement, identical logic to the
        // ele_L1/ele_L2/ele_L3 vertex categories in PreselectAndCategorize2021. For a single
        // FEE track these are the per-track analog of the isL1L1/isL2L2/isL3L3 vertex flags:
        //   is_L1 = track starts at L1, is_L2 = starts at L2, is_L3 = starts at L3.
        bool is_L1 =  L1 && L2 && L3;
        bool is_L2 = !L1 && L2 && L3;
        bool is_L3 = !L1 && !L2 && L3;

        if (debug_ > 0) {
            std::cout << "[FeeAna]  fsp[" << ipart << "]"
                      << "  charge=" << part->getCharge()
                      << "  nHits=" << n2dhits_onTrack
                      << "  chi2ndf=" << track->getChi2Ndf()
                      << "  p=" << trk_mom.Mag()
                      << "  cluE=" << clu_E
                      << "  E/p=" << eop
                      << "  tanL=" << track->getTanLambda()
                      << "  ecalX=" << track->getPositionAtEcal()[0]
                      << "  trkTime=" << track->getTrackTime()
                      << "  trkTime-offset=" << track->getTrackTime() - time_offset_
                      << std::endl;
        }

        //Track Selection
        if (trkSelector_ && !trkSelector_->passCutGt("n_hits_gt",n2dhits_onTrack,weight)) {
            if (debug_ > 0) std::cout << "[FeeAna]   FAIL n_hits_gt: " << n2dhits_onTrack << std::endl;
            continue;
        }

        if (trkSelector_ && !trkSelector_->passCutLt("chi2ndf_lt",track->getChi2Ndf(),weight)) {
            if (debug_ > 0) std::cout << "[FeeAna]   FAIL chi2ndf_lt: " << track->getChi2Ndf() << std::endl;
            continue;
        }

        if (trkSelector_ && !trkSelector_->passCutGt("p_gt",trk_mom.Mag(),weight)) {
            if (debug_ > 0) std::cout << "[FeeAna]   FAIL p_gt: " << trk_mom.Mag() << std::endl;
            continue;
        }

        if (trkSelector_ && !trkSelector_->passCutLt("p_lt",trk_mom.Mag(),weight)) {
            if (debug_ > 0) std::cout << "[FeeAna]   FAIL p_lt: " << trk_mom.Mag() << std::endl;
            continue;
        }

        if (trkSelector_ && !trkSelector_->passCutLt("trk_ecal_lt",track->getPositionAtEcal()[0],weight)) {
            if (debug_ > 0) std::cout << "[FeeAna]   FAIL trk_ecal_lt: " << track->getPositionAtEcal()[0] << std::endl;
            continue;
        }

        if (trkSelector_ && !trkSelector_->passCutGt("trk_ecal_gt",track->getPositionAtEcal()[0],weight)) {
            if (debug_ > 0) std::cout << "[FeeAna]   FAIL trk_ecal_gt: " << track->getPositionAtEcal()[0] << std::endl;
            continue;
        }

        if (trkSelector_ && !trkSelector_->passCutGt("trk_time_gt",track->getTrackTime()-time_offset_,weight)) {
            if (debug_ > 0) std::cout << "[FeeAna]   FAIL trk_time_gt: " << track->getTrackTime() - time_offset_ << std::endl;
            continue;
        }

        if (trkSelector_ && !trkSelector_->passCutLt("trk_time_lt",track->getTrackTime()-time_offset_,weight)) {
            if (debug_ > 0) std::cout << "[FeeAna]   FAIL trk_time_lt: " << track->getTrackTime() - time_offset_ << std::endl;
            continue;
        }

        // In-time isolation: a clean FEE event has a single in-time cluster and a single
        // in-time track. Count objects in-time with the primary FEE cluster (the primary
        // itself is always counted). The counts are always computed and written to the
        // output tree; the veto flags below only control whether a candidate is dropped.
        // Cluster-cluster timing uses raw times (the offset cancels); track timing is
        // compared to the offset-corrected cluster time, using the same MC track-time
        // convention (time_offset_) as the track-time selection above.
        int n_clusters_intime = 0;
        int n_tracks_intime    = 0;

        if (ecalClusters_) {
            for (CalCluster* c : *ecalClusters_) {
                if (!c || c->getEnergy() < vetoClusterEnergyMin_) continue;
                if (std::fabs(c->getTime() - clu_traw) < vetoClusterTimeWindow_) ++n_clusters_intime;
            }
        }
        if (allTracks_) {
            for (Track* t : *allTracks_) {
                if (!t) continue;
                if (std::fabs((t->getTrackTime() - time_offset_) - clu_t) < vetoTrackTimeWindow_) ++n_tracks_intime;
            }
        }

        n_clusters_intime_out_ = n_clusters_intime;
        n_tracks_intime_out_   = n_tracks_intime;

        if (vetoExtraClusters_ && n_clusters_intime > 1) {
            if (debug_ > 0) std::cout << "[FeeAna]   FAIL extra in-time clusters: " << n_clusters_intime << std::endl;
            continue;
        }
        if (vetoExtraTracks_ && n_tracks_intime > 1) {
            if (debug_ > 0) std::cout << "[FeeAna]   FAIL extra in-time tracks: " << n_tracks_intime << std::endl;
            continue;
        }

        if (debug_ > 0) std::cout << "[FeeAna]   PASS all cuts" << std::endl;

        Track* truth_track = nullptr;

        // Truth matching (MC only; always false on data). Computed independently of doTruth_
        // so the flags are available in the output tree.
        //  truth_matched : hit-based majority match to an e^-/e^+, using the same
        //                  utils::hasTruthMatch definition as the 2021 preselection.
        //  has_truth_link: raw Track TRef truth link resolves to an object (no PDG check).
        bool truth_matched  = utils::hasTruthMatch(track_obj, mcParticles_, debug_ > 0);
        bool has_truth_link = (track->getTruthLink().GetObject() != nullptr);

        //Get the truth track
        if (doTruth_) {
            truth_track = (Track*) track->getTruthLink().GetObject();
            if (!truth_track)
                std::cout<<"Warnings::FeeAnaProcessor::Requested Truth track but couldn't find it in the ntuple"<<std::endl;
        }

        if(debug_ > 0)
        {
            std::cout<<"========================================="<<std::endl;
            std::cout<<"========================================="<<std::endl;
            std::cout<<"Track params:           "<<std::endl;
            track->Print();
        }

        trkHistos_->Fill1DHistograms(track);
        trkHistos_->Fill2DTrack(track);

        if (truthHistos_) {
            truthHistos_->Fill1DHistograms(truth_track);
            truthHistos_->Fill2DTrack(track);
            truthHistos_->Fill1DTrackTruth(track, truth_track);
        }

        n_sel_tracks++;

        //Fill histograms for FEE smearing analysis
        trkHistos_->Fill2DHisto("xypos_at_ecal_hh",
                                track->getPositionAtEcal()[0],
                                track->getPositionAtEcal()[1]);

        trkHistos_->Fill3DHisto("p_vs_TanLambda_Phi_hhh",
                                track->getPhi(),
                                track->getTanLambda(),
                                track->getP());

        trkHistos_->Fill2DHisto("p_vs_nHits_hh",
                                track->getTrackerHitCount(),
                                track->getP());

        if (track->getTanLambda() > 0 )
          trkHistos_->Fill2DHisto("p_vs_nHits_top_hh",
                                  track->getTrackerHitCount(),
                                  track->getP());
        else
          trkHistos_->Fill2DHisto("p_vs_nHits_bot_hh",
                                  track->getTrackerHitCount(),
                                  track->getP());

        trkHistos_->Fill3DHisto("p_vs_TanLambda_nHits_hhh",
                                track->getTanLambda(),
                                track->getTrackerHitCount(),
                                track->getP());



        // Track-level quantities for output tree and smearing histograms.
        // trk_smeared starts as a copy of the original; update* methods modify it in-place.
        Track  trk_smeared     = *track;
        double omega_unsmeared = track->getOmega();
        double omega_smeared   = omega_unsmeared;
        double p_smear_ratio   = 1.0;

        if (smearingTool_) {

          double nhits = track->getTrackerHitCount();

          if (doSmearing_) {
            if (smearOmega_) {
              double scale  = smearingTool_->updateWithSmearOmega(trk_smeared);
              // scale = |omega/omega_smeared| = |p_smeared/p_original|
              omega_smeared = omega_unsmeared / scale;
              p_smear_ratio = scale;
            } else {
              p_smear_ratio = smearingTool_->updateWithSmearP(trk_smeared);
            }
            smearingTool_->updateWithSmearZ0(trk_smeared);
          }

          double pval  = trk_smeared.getP();
          double z0val = trk_smeared.getZ0();

          psmear_h_->Fill(pval);
          psmear_vs_nHits_hh_->Fill(nhits, pval);

          if (isTop) {
            psmear_top_h_->Fill(pval);
            psmear_vs_nHits_top_hh_->Fill(nhits, pval);
          }
          else {
            psmear_bot_h_->Fill(pval);
            psmear_vs_nHits_bot_hh_->Fill(nhits, pval);
          }

          psmear_rel_h_->Fill(pval);
          psmear_vs_nHits_rel_hh_->Fill(nhits, pval);

          if (isTop) {
            psmear_vs_nHits_top_rel_hh_->Fill(nhits, pval);
          }
          else {
            psmear_vs_nHits_bot_rel_hh_->Fill(nhits, pval);
          }

          // z0 smearing validation
          z0smear_h_->Fill(z0val);
          if (isTop) {
            z0smear_top_h_->Fill(z0val);
          }
          else {
            z0smear_bot_h_->Fill(z0val);
          }

          // omega smearing validation
          double tanL = track->getTanLambda();
          double phi0 = track->getPhi();

          omega_h_->Fill(omega_unsmeared);
          omegasmear_h_->Fill(omega_smeared);
          omega_vs_p_hh_->Fill(track->getP(), omega_unsmeared);
          omega_vs_tanL_hh_->Fill(tanL, omega_unsmeared);
          omega_vs_phi0_hh_->Fill(phi0, omega_unsmeared);
          omegasmear_vs_tanL_hh_->Fill(tanL, omega_smeared);
          omegasmear_vs_phi0_hh_->Fill(phi0, omega_smeared);

          if (isTop) {
            omega_top_h_->Fill(omega_unsmeared);
            omegasmear_top_h_->Fill(omega_smeared);
            omega_vs_tanL_top_hh_->Fill(tanL, omega_unsmeared);
            omega_vs_phi0_top_hh_->Fill(phi0, omega_unsmeared);
            omegasmear_vs_tanL_top_hh_->Fill(tanL, omega_smeared);
            omegasmear_vs_phi0_top_hh_->Fill(phi0, omega_smeared);
          }
          else {
            omega_bot_h_->Fill(omega_unsmeared);
            omegasmear_bot_h_->Fill(omega_smeared);
            omega_vs_tanL_bot_hh_->Fill(tanL, omega_unsmeared);
            omega_vs_phi0_bot_hh_->Fill(phi0, omega_unsmeared);
            omegasmear_vs_tanL_bot_hh_->Fill(tanL, omega_smeared);
            omegasmear_vs_phi0_bot_hh_->Fill(phi0, omega_smeared);
          }

        } // smearing validation

        // Fill output tree (one entry per selected FEE track)
        if (output_tree_) {
          track_out_         = *track;       // original, unsmeared
          track_smeared_out_ = trk_smeared;  // smeared (same as original if doSmearing_=false)
          p_smear_ratio_out_ = p_smear_ratio;
          clu_E_out_         = clu_E;
          clu_time_out_      = clu_t;
          eop_out_           = eop;
          L1_axial_out_      = L1_axial;
          L1_stereo_out_     = L1_stereo;
          L2_axial_out_      = L2_axial;
          L2_stereo_out_     = L2_stereo;
          L3_axial_out_      = L3_axial;
          L3_stereo_out_     = L3_stereo;
          L4_axial_out_      = L4_axial;
          L4_stereo_out_     = L4_stereo;
          L1_out_            = L1;
          L2_out_            = L2;
          L3_out_            = L3;
          L4_out_            = L4;
          is_L1_out_         = is_L1;
          is_L2_out_         = is_L2;
          is_L3_out_         = is_L3;
          hit_pattern_out_   = hit_pattern;
          n_hits_out_        = n2dhits_onTrack;
          track_corr_out_               = trk_corr;
          hit_pattern_scale_out_        = hitPatternScale;
          hit_pattern_scale_isdefault_out_ = hitPatternScaleIsDefault;
          truth_matched_out_  = truth_matched;
          has_truth_link_out_ = has_truth_link;
          output_tree_->Fill();
        }

    }//Loop on FinalStateParticles

    trkHistos_->Fill1DHisto("n_tracks_h",n_sel_tracks);

    return true;
}

void FeeAnaProcessor::finalize() {

    if (output_tree_) {
        outF_->cd();
        output_tree_->Write();
    }

    trkHistos_->saveHistos(outF_,trkCollName_);
    delete trkHistos_;
    trkHistos_ = nullptr;
    if (trkSelector_)
        trkSelector_->getCutFlowHisto()->Write();

    if (truthHistos_) {
        truthHistos_->saveHistos(outF_,trkCollName_+"_truth");
        delete truthHistos_;
        truthHistos_ = nullptr;
    }

    for (reg_it it = reg_histos_.begin(); it!=reg_histos_.end(); ++it) {
      std::string dirName = it->first;
      (it->second)->saveHistos(outF_,dirName);
      outF_->cd(dirName.c_str());
      reg_selectors_[it->first]->getCutFlowHisto()->Write();
    }

    if (smearingTool_) {
      outF_->cd(trkCollName_.c_str());
      psmear_h_->Write();
      psmear_top_h_->Write();
      psmear_bot_h_->Write();

      psmear_vs_nHits_hh_->Write();
      psmear_vs_nHits_top_hh_->Write();
      psmear_vs_nHits_bot_hh_->Write();
      delete psmear_h_;
      delete psmear_top_h_;
      delete psmear_bot_h_;
      delete psmear_vs_nHits_hh_;
      delete psmear_vs_nHits_top_hh_;
      delete psmear_vs_nHits_bot_hh_;

      psmear_rel_h_->Write();
      psmear_vs_nHits_rel_hh_->Write();
      psmear_vs_nHits_top_rel_hh_->Write();
      psmear_vs_nHits_bot_rel_hh_->Write();
      delete psmear_rel_h_;
      delete psmear_vs_nHits_rel_hh_;
      delete psmear_vs_nHits_top_rel_hh_;
      delete psmear_vs_nHits_bot_rel_hh_;

      // z0 smearing validation histograms
      z0smear_h_->Write();
      z0smear_top_h_->Write();
      z0smear_bot_h_->Write();
      delete z0smear_h_;
      delete z0smear_top_h_;
      delete z0smear_bot_h_;

      // omega smearing validation histograms
      omega_h_->Write();
      omega_top_h_->Write();
      omega_bot_h_->Write();
      omegasmear_h_->Write();
      omegasmear_top_h_->Write();
      omegasmear_bot_h_->Write();
      omega_vs_p_hh_->Write();
      omega_vs_tanL_hh_->Write();
      omega_vs_tanL_top_hh_->Write();
      omega_vs_tanL_bot_hh_->Write();
      omegasmear_vs_tanL_hh_->Write();
      omegasmear_vs_tanL_top_hh_->Write();
      omegasmear_vs_tanL_bot_hh_->Write();
      omega_vs_phi0_hh_->Write();
      omega_vs_phi0_top_hh_->Write();
      omega_vs_phi0_bot_hh_->Write();
      omegasmear_vs_phi0_hh_->Write();
      omegasmear_vs_phi0_top_hh_->Write();
      omegasmear_vs_phi0_bot_hh_->Write();

      delete omega_h_;
      delete omega_top_h_;
      delete omega_bot_h_;
      delete omegasmear_h_;
      delete omegasmear_top_h_;
      delete omegasmear_bot_h_;
      delete omega_vs_p_hh_;
      delete omega_vs_tanL_hh_;
      delete omega_vs_tanL_top_hh_;
      delete omega_vs_tanL_bot_hh_;
      delete omegasmear_vs_tanL_hh_;
      delete omegasmear_vs_tanL_top_hh_;
      delete omegasmear_vs_tanL_bot_hh_;
      delete omega_vs_phi0_hh_;
      delete omega_vs_phi0_top_hh_;
      delete omega_vs_phi0_bot_hh_;
      delete omegasmear_vs_phi0_hh_;
      delete omegasmear_vs_phi0_top_hh_;
      delete omegasmear_vs_phi0_bot_hh_;

    }

}

DECLARE_PROCESSOR(FeeAnaProcessor);
