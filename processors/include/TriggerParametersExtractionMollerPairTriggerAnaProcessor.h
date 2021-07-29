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

#include "FlatTupleMaker.h"
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


class TriggerParametersExtractionMollerPairTriggerAnaProcessor : public Processor {

    public:
	TriggerParametersExtractionMollerPairTriggerAnaProcessor(const std::string& name, Process& process);
        ~TriggerParametersExtractionMollerPairTriggerAnaProcessor();
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


        double beamE_{1.92};
        int isData_{0};
        std::string analysis_{"TriggerParametersExtractionMoller"};

        //Debug level
        int debug_{0};

        /*
         * Parameters for all cut functions depend on beam energy.
         * Here, the setup is for 1.92 GeV.
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
        double top_topCutX[2] = {21.8429, 0.856399};
        double top_botCutX[2] = {-20.3696, 0.91452};

        double bot_topCutX[2] = {24.3557, 0.862553};
        double bot_botCutX[2] = {-22.3814, 0.910335};

        //Parameters of cut functions for Y
        double top_topCutY[2] = {9.84386, 0.893539};
        double top_botCutY[2] = {-7.78579, 0.900762};

        double bot_topCutY[2] = {6.73299, 0.888867};
        double bot_botCutY[2] = {-8.71712, 0.909765};

        //Upper limit for position dependent energy
        TF1 *func_pde_moller;
        double pars_pde_moller[3] = {1.79097, 0.118131, 0.00254604}; // 3 sigma

        // Kinematic equations
        TF1* func_E_vs_theta_before_roation;
        TF1* func_theta1_vs_theta2_before_roation;

        // save a tree with tuple
        std::shared_ptr<FlatTupleMaker> _reg_tuple;
};

#endif
