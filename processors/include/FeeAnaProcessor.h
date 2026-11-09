#ifndef __FEE_ANAPROCESSOR_H__
#define __FEE_ANAPROCESSOR_H__

//-----------------//
//   C++  StdLib   //
//-----------------//
#include <iostream>
#include <string>

//----------//
//   ROOT   //
//----------//
#include "TClonesArray.h"
#include "TFile.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TTree.h"

//-----------//
//   hpstr   //
//-----------//
#include "Processor.h"
#include "BaseSelector.h"
#include "Track.h"
#include "Particle.h"
#include "Event.h"
#include "CalCluster.h"
#include "TrackerHit.h"
#include "EventHeader.h"
#include "TSData.h"
#include "TrackHistos.h"
#include "TrackSmearingTool.h"
#include "AnaHelpers.h"
#include "MCParticle.h"
#include "TruthMatchingUtils.h"

/**
 * @brief Select Full Energy Electron (FEE) events for the momentum-smearing calibration.
 *
 * Unlike TrackingAnaProcessor (which loops over a bare Track collection), this processor
 * loops over the FinalStateParticles, where each Track is already associated to its ECal
 * cluster at the LCIO stage. An FEE candidate is an electron (charge < 0) whose associated
 * cluster looks like a full-energy electron (energy / time / E-over-p), passing the usual
 * track-quality cuts.
 */
class FeeAnaProcessor : public Processor {

    public:
        /**
         * @brief Class constructor.
         *
         * @param name Name for this instance of the class.
         * @param process The Process class associated with Processor, provided
         *                by the processing framework.
         */
        FeeAnaProcessor(const std::string& name, Process& process);

        /** Destructor */
        ~FeeAnaProcessor();

        /**
         * @brief Configure the Ana Processor
         *
         * @param parameters The configuration parameters
         */
        virtual void configure(const ParameterSet& parameters);

        /**
         * @brief Process the event and put new data products into it.
         *
         * @param ievent The Event to process.
         */
        virtual bool process(IEvent* ievent);

        /**
         * @brief Callback for the Processor to take any necessary
         *        action when the processing of events starts.
         *
         * @param tree
         */
        virtual void initialize(TTree* tree);

        /**
         * @brief Callback for the Processor to take any necessary
         *        action when the processing of events finishes.
         */
        virtual void finalize();

        /**
         * @brief Set output TFile and create the output TTree.
         */
        virtual void setFile(TFile* outFile);

    private:

        /** Container to hold all FinalStateParticle objects (track + associated cluster). */
        std::vector<Particle*>* fsps_{};
        TBranch* bfsps_{nullptr}; //!< description

        /** Event header branch. */
        TBranch* bevth_{nullptr}; //!

        // Event Header
        EventHeader* evth_{nullptr}; //!

        // 2021 trigger-scaler bank (TSBank), used to require the FEE trigger.
        TSData* tsdata_{nullptr}; //!
        TBranch* btsdata_{nullptr}; //!

        // Full event collections, used to veto additional in-time tracks/clusters.
        std::vector<CalCluster*>* ecalClusters_{nullptr}; //!
        TBranch* becalClusters_{nullptr}; //!
        std::vector<Track*>* allTracks_{nullptr}; //!
        TBranch* ballTracks_{nullptr}; //!

        // MCParticle collection, used for hit-based truth matching (MC only).
        std::string mcColl_{"MCParticle"}; //!< MCParticle collection name
        std::vector<MCParticle*>* mcParticles_{nullptr}; //!
        TBranch* bmcParticles_{nullptr}; //!

        std::string trkCollName_; //!< Track Collection name (histo/smearing naming)
        std::string fspCollName_{"FinalStateParticles_KF"}; //!< FinalStateParticle collection name

        // Track Selector configuration
        std::string selectionCfg_;
        std::shared_ptr<BaseSelector> trkSelector_; //!< description
        std::vector<std::string> regionSelections_; //!< track selections
        std::map<std::string, std::shared_ptr<BaseSelector>> reg_selectors_; //!< description
        std::map<std::string, std::shared_ptr<TrackHistos>> reg_histos_; //!< description
        typedef std::map<std::string, std::shared_ptr<TrackHistos>>::iterator reg_it; //!< description

        // Containers to hold histogrammer info
        std::string histCfgFilename_; //!< description
        std::string truthHistCfgFilename_; //!< description
        TrackHistos* trkHistos_{nullptr}; //!< description
        TrackHistos* truthHistos_{nullptr}; //!< description

        std::vector<std::string> regions_; //!

        bool doTruth_{false}; //!< description
        int isData_{1}; //! is data
        int debug_{0}; //!< debug level
        int seed_{0};  //!< seed
        float time_offset_{0}; //! time offset

        // FEE selection
        double feeClusterEnergyMin_{2.5};  //!< minimum energy of the associated FEE cluster
        // Cluster-time window is applied to the OFFSET-CORRECTED time
        // (cluster.getTime() - calTimeOffset), which is centered near 0.
        double clusterTimeMin_{-6.0};      //!< minimum corrected cluster time (data)
        double clusterTimeMax_{ 6.0};      //!< maximum corrected cluster time (data)
        double clusterTimeMinMC_{-6.0};    //!< minimum corrected cluster time (MC)
        double clusterTimeMaxMC_{ 6.0};    //!< maximum corrected cluster time (MC)
        double calTimeOffset_{0.0};        //!< cluster-time offset subtracted before the cut (data)
        double calTimeOffsetMC_{0.0};      //!< cluster-time offset subtracted before the cut (MC)
        double mcTimeOffset_{5.0};         //!< time offset applied to MC tracks
        bool   requireElectron_{true};     //!< require the FSP to be an electron (charge < 0)
        bool   requireCluster_{true};      //!< require an associated FEE cluster passing E/time/(E/p)
        bool   requireFeeTrigger_{false};  //!< require the FEE trigger (TSBank FEE_Top||FEE_Bot); data only
        double eopMin_{0.0};               //!< minimum cluster-energy / track-momentum (E/p)
        double eopMax_{99.0};              //!< maximum cluster-energy / track-momentum (E/p)

        // In-time isolation veto: a clean FEE has a single track and a single cluster.
        // When enabled, a candidate is vetoed if any *additional* cluster/track is
        // in-time with the primary FEE cluster (i.e. >1 in-time object of that type).
        bool   vetoExtraClusters_{false};      //!< veto if extra clusters are in-time with the FEE cluster
        bool   vetoExtraTracks_{false};        //!< veto if extra tracks are in-time with the FEE cluster
        double vetoClusterTimeWindow_{8.0};    //!< |t_clu - t_FEEclu| window for the cluster veto (ns)
        double vetoTrackTimeWindow_{8.0};      //!< |t_trk - t_FEEclu(corrected)| window for the track veto (ns)
        double vetoClusterEnergyMin_{0.5};     //!< only clusters above this energy count toward the cluster veto (GeV)

        //Momentum smearing closure test
        std::shared_ptr<TrackSmearingTool> smearingTool_;
        std::shared_ptr<TrackSmearingTool> smearingToolRel_;
        std::string pSmearingFile_{""};
        std::string smearingCfgFile_{""};
        bool doSmearing_{false};  //!< Master switch for smearing
        double smearingFactor_{1.0};  //!< Factor to multiply smearing parameters by
        bool requireTruthMatch_{false};  //!< Require truth match for smearing
        bool smearOmega_{false};  //!< Use omega (curvature) smearing instead of p smearing
        std::string smearingVariable_{""};  //!< "flat", "nHits", "tanLambda", "phi0" — explicit lookup; "" = JSON default
        std::string scaleCorrVariable_{""};  //!< if set, omega data-mode scale uses pBinned_ means

        // Per-hit_pattern momentum scale correction (data momentum -> beam energy).
        // Loaded from a JSON with a "pattern_to_scale" map (hit_pattern -> scale) plus a
        // "default_scale" fallback. p_corr = p * scale. Applied on data only by default.
        std::string hitPatternScaleFile_{""};   //!< JSON of per-hit_pattern scale factors
        bool   doHitPatternScale_{false};        //!< master switch for the hit-pattern scale correction
        bool   hitPatternScaleDataOnly_{true};   //!< only apply the scale on data (MC left untouched)
        std::map<int,double> hitPatternScale_;   //!< hit_pattern -> scale factor
        double hitPatternDefaultScale_{1.0};     //!< fallback scale for unlisted patterns

        // AnaHelpers for decoding per-sensor hit layers
        std::shared_ptr<AnaHelpers> ah_{nullptr};

        // Output TTree
        std::unique_ptr<TTree> output_tree_{nullptr};
        Track  track_out_;               //!< original (unsmeared) track
        Track  track_smeared_out_;       //!< smeared track (identical to track_out_ if no smearing)
        double p_smear_ratio_out_{1.0}; //!< smeared_p / original_p (1 if no smearing)
        // associated FEE cluster (from the FinalStateParticle)
        double clu_E_out_{-9999.0};      //!< associated cluster energy
        double clu_time_out_{-9999.0};   //!< associated cluster time
        double eop_out_{-1.0};           //!< associated cluster E / track p
        // in-time isolation counts (include the primary FEE object; clean FEE => 1)
        int    n_clusters_intime_out_{0}; //!< clusters in-time with the FEE cluster (>energy threshold)
        int    n_tracks_intime_out_{0};   //!< tracks in-time with the FEE cluster
        // per-layer axial/stereo hit flags (L1-L4)
        bool L1_axial_out_{false};
        bool L1_stereo_out_{false};
        bool L2_axial_out_{false};
        bool L2_stereo_out_{false};
        bool L3_axial_out_{false};
        bool L3_stereo_out_{false};
        bool L4_axial_out_{false};
        bool L4_stereo_out_{false};
        // per-layer "both axial+stereo hit" flags (eleLN/posLN analog, top/bottom independent)
        bool L1_out_{false};
        bool L2_out_{false};
        bool L3_out_{false};
        bool L4_out_{false};
        // earliest-layer category with outward hit requirement (single-track isL1L1/isL2L2/isL3L3 analog)
        bool is_L1_out_{false};
        bool is_L2_out_{false};
        bool is_L3_out_{false};
        // full per-sensor hit pattern encoded as a bitmask: bit i (i=0..13) set if sensor
        // slot i has a hit. Slots follow the GetTrackHitLayers convention (0/1 = L1
        // axial/stereo, 2/3 = L2, ...). Use this to bin resolution by exact hit pattern.
        int  hit_pattern_out_{0};
        int  n_hits_out_{0};   //!< number of tracker hits on the FEE track
        // per-hit_pattern momentum scale correction (data->beam). track_corr_out_ is the
        // original track with its momentum scaled by hit_pattern_scale_out_ (omega scaled by
        // 1/scale to stay consistent). hit_pattern_scale_isdefault_out_ is true when the
        // track's hit_pattern was not in the JSON map and the default_scale was used.
        Track  track_corr_out_;                         //!< momentum-scale-corrected track
        double hit_pattern_scale_out_{1.0};             //!< scale factor applied to this track's momentum
        bool   hit_pattern_scale_isdefault_out_{false}; //!< true if the default (fallback) scale was used
        // truth matching (MC only; false on data)
        // truth_matched_out_ : hit-based majority match to an e^-/e^+ (utils::hasTruthMatch),
        //                      identical definition to ele/pos_has_truth_link in the 2021 preselection.
        // has_truth_link_out_: raw Track TRef truth link resolves to an object (no PDG requirement).
        bool truth_matched_out_{false};
        bool has_truth_link_out_{false};
        TH1D* psmear_h_;
        TH1D* psmear_top_h_;
        TH1D* psmear_bot_h_;
        TH2D* psmear_vs_nHits_hh_;
        TH2D* psmear_vs_nHits_top_hh_;
        TH2D* psmear_vs_nHits_bot_hh_;

        TH1D* psmear_rel_h_;
        TH2D* psmear_vs_nHits_rel_hh_;
        TH2D* psmear_vs_nHits_top_rel_hh_;
        TH2D* psmear_vs_nHits_bot_rel_hh_;

        // z0 smearing validation histograms
        TH1D* z0smear_h_;
        TH1D* z0smear_top_h_;
        TH1D* z0smear_bot_h_;

        // omega smearing validation histograms
        TH1D* omega_h_;           // unsmeared omega
        TH1D* omega_top_h_;
        TH1D* omega_bot_h_;
        TH1D* omegasmear_h_;      // smeared omega (when omega smearing enabled)
        TH1D* omegasmear_top_h_;
        TH1D* omegasmear_bot_h_;
        TH2D* omega_vs_p_hh_;     // omega vs momentum

        // omega vs track parameters (unsmeared and smeared)
        TH2D* omega_vs_tanL_hh_;
        TH2D* omega_vs_tanL_top_hh_;
        TH2D* omega_vs_tanL_bot_hh_;
        TH2D* omegasmear_vs_tanL_hh_;
        TH2D* omegasmear_vs_tanL_top_hh_;
        TH2D* omegasmear_vs_tanL_bot_hh_;

        TH2D* omega_vs_phi0_hh_;
        TH2D* omega_vs_phi0_top_hh_;
        TH2D* omega_vs_phi0_bot_hh_;
        TH2D* omegasmear_vs_phi0_hh_;
        TH2D* omegasmear_vs_phi0_top_hh_;
        TH2D* omegasmear_vs_phi0_bot_hh_;

}; // FeeAnaProcessor

#endif // __FEE_ANAPROCESSOR_H__
