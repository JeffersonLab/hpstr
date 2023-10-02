#ifndef __VERTEX_ANAPROCESSOR_H__
#define __VERTEX_ANAPROCESSOR_H__

// HPSTR
#include "HpsEvent.h"
#include "Collections.h"
#include "EventHeader.h"
#include "TSData.h"
#include "Vertex.h"
#include "Track.h"
#include "TrackerHit.h"
#include "MCParticle.h"
#include "Particle.h"
#include "Processor.h"
#include "BaseSelector.h"
#include "TrackHistos.h"
#include "MCAnaHistos.h"
#include "utilities.h"

#include "FlatTupleMaker.h"
#include "AnaHelpers.h"

// ROOT
#include "TFile.h"
#include "TTree.h"
#include "TRefArray.h"
#include "TBranch.h"
#include "TVector3.h"
#include "TLorentzVector.h"

// C++ 
#include <memory>

struct char_cmp {
    bool operator () (const char *a,const char *b) const
    {
        return strcmp(a,b)<0;
    }
};

/**
 * @brief Insert description here.
 * more details
 */
class VertexAnaProcessor : public Processor {

    public:
        /**
         * @brief Constructor
         * 
         * @param name 
         * @param process 
         */
        VertexAnaProcessor(const std::string& name, Process& process);

        ~VertexAnaProcessor();

        /**
         * @brief description
         * 
         * @param ievent 
         * @return true 
         * @return false 
         */
        virtual bool process(IEvent* ievent);

        /**
         * @brief description
         * 
         * @param tree 
         */
        virtual void initialize(TTree* tree);

        /**
         * @brief description
         * 
         */
        virtual void finalize();

        /**
         * @brief description
         * 
         * @param parameters 
         */
        virtual void configure(const ParameterSet& parameters);

    private:
        std::shared_ptr<BaseSelector> vtxSelector; //!< description
        std::vector<std::string> regionSelections_; //!< description

        std::string selectionCfg_; //!< description
        std::map<const char*, int, char_cmp> brMap_; //!< description
        TBranch* bts_{nullptr}; //!< description
        TBranch* bvtxs_{nullptr}; //!< description
        TBranch* bhits_{nullptr}; //!< description
        TBranch* btrks_{nullptr}; //!< description
        TBranch* bmcParts_{nullptr}; //!< description
        TBranch* bevth_{nullptr}; //!< description
        TBranch* becal_{nullptr}; //!< description

        EventHeader* evth_{nullptr}; //!< description
        TSData* ts_{nullptr}; //!< description
        std::vector<CalCluster*>* ecal_{}; //!< description
        std::vector<Vertex*>* vtxs_{}; //!< description
        std::vector<Track*>* trks_{}; //!< description
        std::vector<TrackerHit*>* hits_{}; //!< description
        std::vector<MCParticle*>* mcParts_{}; //!< description

        std::string anaName_{"vtxAna"}; //!< description
        std::string tsColl_{"TSBank"}; //!< description
        std::string vtxColl_{"Vertices"}; //!< description
        std::string hitColl_{"RotatedHelicalTrackHits"}; //!< description
        std::string trkColl_{"GBLTracks"}; //!< description
        std::string ecalColl_{"RecoEcalClusters"}; //!< description
        std::string mcColl_{"MCParticle"}; //!< description
        int isRadPDG_{622}; //!< description
        int makeFlatTuple_{0}; //!< make true in config to save flat tuple
        TTree* tree_{nullptr}; //!< description

        std::shared_ptr<TrackHistos> _vtx_histos; //!< description
        std::shared_ptr<MCAnaHistos> _mc_vtx_histos; //!< description

        /** \todo Duplicate.. We can make a single class.. ? */
        std::map<std::string, std::shared_ptr<BaseSelector>> _reg_vtx_selectors; //!< description
        std::map<std::string, std::shared_ptr<TrackHistos>> _reg_vtx_histos; //!< description
        std::map<std::string, std::shared_ptr<MCAnaHistos>> _reg_mc_vtx_histos; //!< description
        std::map<std::string, std::shared_ptr<FlatTupleMaker>> _reg_tuples; //!< description

        std::vector<std::string> _regions; //!< description

        typedef std::map<std::string, std::shared_ptr<TrackHistos>>::iterator reg_it; //!< description
        typedef std::map<std::string, std::shared_ptr<MCAnaHistos>>::iterator reg_mc_it; //!< description

        std::string histoCfg_{""}; //!< description
        std::string mcHistoCfg_{""}; //!< description
        double timeOffset_{-999}; //!< description
        double beamE_{2.3}; //!< In GeV. Default is 2016 value;
        int isData_{0}; //!< description
        std::string analysis_{"vertex"}; //!< description

        std::shared_ptr<AnaHelpers> _ah; //!< description

        int debug_{0}; //!< Debug level
        std::string beamPosCfg_{""}; //!< json containing run dep beamspot positions
        json bpc_configs_; //!< json object
        std::vector<double> beamPosCorrections_ = {0.0,0.0,0.0}; //!< holds beam position corrections
        std::string v0ProjectionFitsCfg_{""};//!< json file w run dependent v0 projection fits
        json v0proj_fits_;//!< json object v0proj
        double eleTrackTimeBias_ = 0.0;
        double posTrackTimeBias_ = 0.0;
        int current_run_number_{-999}; //!< track current run number

        bool mc_reg_on_ = false;
};

#endif
