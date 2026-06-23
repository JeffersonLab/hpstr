#ifndef __TRACKING_ANAPROCESSOR_H__
#define __TRACKING_ANAPROCESSOR_H__

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
#include "Event.h"
#include "CalCluster.h"
#include "EventHeader.h"
#include "TrackHistos.h"
#include "TrackSmearingTool.h"
#include "AnaHelpers.h"

/**
 * @brief Insert description here.
 * more details
 */
class TrackingAnaProcessor : public Processor {

    public:
        /**
         * @brief Class constructor.
         *
         * @param name Name for this instance of the class.
         * @param process The Process class associated with Processor, provided
         *                by the processing framework.
         */
        TrackingAnaProcessor(const std::string& name, Process& process);

        /** Destructor */
        ~TrackingAnaProcessor();

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

        /** Container to hold all Track objects. */
        std::vector<Track*>* tracks_{};
        TBranch* btracks_{nullptr}; //!< description

        /** Event header branch. */
        TBranch* bevth_{nullptr}; //!

        /** Clusters */
        TBranch* becal_{nullptr}; //!

        // Event Header
        EventHeader* evth_{nullptr}; //!
        std::vector<CalCluster*>* ecal_{}; //!<

        std::string trkCollName_; //!< Track Collection name
        std::string ecalCollName_{"RecoEcalClusters"}; //!< Cluster Collection name

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
        double feeClusterEnergyMin_{2.5};  //!< minimum cluster energy for FEE selection
        double clusterTimeMin_{40.0};      //!< minimum cluster time (data)
        double clusterTimeMax_{70.0};      //!< maximum cluster time (data)
        double clusterTimeMinMC_{40.0};    //!< minimum cluster time (MC)
        double clusterTimeMaxMC_{70.0};    //!< maximum cluster time (MC)
        double mcTimeOffset_{5.0};         //!< time offset applied to MC tracks

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

}; // TrackingAnaProcessor

#endif // __TRACKING_ANAPROCESSOR_
