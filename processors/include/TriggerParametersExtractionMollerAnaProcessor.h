#ifndef __TRIGGERPARAMETERSEXTRACTIONMOLLER_ANAPROCESSOR_H__
#define __TRIGGERPARAMETERSEXTRACTIONMOLLER_ANAPROCESSOR_H__


//HPSTR
#include "HpsEvent.h"
#include "Collections.h"
#include "Track.h"
#include "CalCluster.h"
#include "CalHit.h"
#include "MCParticle.h"
#include "Particle.h"
#include "Vertex.h"
#include "Processor.h"
#include "HistoManager.h"
#include "TriggerParametersExtractionMollerAnaHistos.h"
#include "AnaHelpers.h"

#include "BaseSelector.h"

//ROOT
#include "TFile.h"
#include "TTree.h"
#include "TRefArray.h"
#include "TBranch.h"
#include "TVector3.h"
#include "TLorentzVector.h"
#include "TMath.h"

//C++
#include <memory>


struct char_cmp {
    bool operator () (const char *a,const char *b) const
    {
        return strcmp(a,b)<0;
    }
};


class TriggerParametersExtractionMollerAnaProcessor : public Processor {

    public:
		TriggerParametersExtractionMollerAnaProcessor(const std::string& name, Process& process);
        ~TriggerParametersExtractionMollerAnaProcessor();
        virtual bool process(IEvent* ievent);

        virtual void initialize(TTree* tree);

        virtual void finalize();

        virtual void configure(const ParameterSet& parameters);

    private:
        std::shared_ptr<AnaHelpers> _ah;

        //Containers to hold histogrammer info
        TriggerParametersExtractionMollerAnaHistos* histos{nullptr};
        std::string  histCfgFilename_;

        TTree* tree_{nullptr};
        TBranch* btrks_{nullptr};
        TBranch* bgtpClusters_{nullptr};
        TBranch* bmcParts_{nullptr};
        TBranch* bvtxs_{nullptr};

        std::vector<Track*>  * trks_{};
        std::vector<CalCluster*> * gtpClusters_{};
        std::vector<MCParticle*>  * mcParts_{};
        std::vector<Vertex*> * vtxs_{};

        std::string anaName_{"vtxAna"};
        std::string trkColl_{"GBLTracks"};
        std::string gtpClusColl_{"RecoEcalClustersGTP"};
        std::string mcColl_{"MCParticle"};
        std::string vtxColl_{"Vertices"};


        double beamE_{3.7};
        int isData_{0};
        std::string analysis_{"TriggerParametersExtractionMoller"};

        //Debug level
        int debug_{0};

        /*
         * Parameters for all cut functions depend on beam energy.
         * Here, the setup is for 3.7 GeV.
         */

        //Cut functions for X
        TF1 *func_top_topCutX;
        TF1 *func_top_botCutX;

        TF1 *func_bot_topCutX;
        TF1 *func_bot_botCutX;

        //Cut functions for Y
        TF1 *func_top_topCutY;
        TF1 *func_top_botCutY;

        TF1 *func_bot_topCutY;
        TF1 *func_bot_botCutY;

        //Parameters of cut functions for X
        double top_topCutX[2] = {22.4311, 0.865347};
        double top_botCutX[2] = {-20.9517, 0.889685};

        double bot_topCutX[2] = {23.5512, 0.867873};
        double bot_botCutX[2] = {-21.6512, 0.888276};

        //Parameters of cut functions for Y
        double top_topCutY[2] = {7.1885, 0.900118};
        double top_botCutY[2] = {-5.84294, 0.903104};

        double bot_topCutY[2] = {4.91401, 0.88867};
        double bot_botCutY[2] = {-6.39967, 0.910099};

        //NHits dependence energy
        TF1 *func_nhde;
        double pars_nhde[2] = {1.48511, 0.0147336}; // 5 sigma

        //Upper limit for position dependent energy
        TF1 *func_pde_moller;
        double pars_pde_moller[3] = {2.73972, 0.147281, 0.00258355}; // 3 sigma


        // Kinematic equations
        TF1* func_E_vs_theta_before_roation;
        TF1* func_theta1_vs_theta2_before_roation;


};

#endif
