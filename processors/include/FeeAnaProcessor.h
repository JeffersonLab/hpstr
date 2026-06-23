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
#include "EventHeader.h"
#include "TrackHistos.h"
#include "TrackSmearingTool.h"
#include "AnaHelpers.h"

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
        double eopMin_{0.0};               //!< minimum cluster-energy / track-momentum (E/p)
        double eopMax_{99.0};              //!< maximum cluster-energy / track-momentum (E/p)

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
        // per-layer axial/stereo hit flags (L1-L3)
        bool L1_axial_out_{false};
        bool L1_stereo_out_{false};
        bool L2_axial_out_{false};
        bool L2_stereo_out_{false};
        bool L3_axial_out_{false};
        bool L3_stereo_out_{false};
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
