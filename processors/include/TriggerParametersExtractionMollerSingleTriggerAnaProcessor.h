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


class TriggerParametersExtractionMollerSingleTriggerAnaProcessor : public Processor {

    public:
		TriggerParametersExtractionMollerSingleTriggerAnaProcessor(const std::string& name, Process& process);
        ~TriggerParametersExtractionMollerSingleTriggerAnaProcessor();
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

        // Cut setup, default as for 3.7 GeV
		double CHI2NDFTHRESHOLD = 20;
		double CLUSTERENERGYTHRESHOLD= 0.1; // threshold of cluster energy for analyzable events
		double CLUSTERENERGYMIN = 0.72; // minimum of cluster energy; 3sigma
		double CLUSTERENERGYMAX = 1.52; // maximum of cluster energy; 5sigma
		double CLUSTERXMIN = -13; // minimum of x index
		double CLUSTERXMAX = -10; // maximum of x index
		double CLUSTERYMIN = -1; // minimum of y index
		double CLUSTERYMAX = 1; // maximum of y index
		double DIFFENERGYMIN = -0.34; // minimum for difference between measured and calculated energy
		double DIFFENERGYMAX = 0.33; // maximum for difference between measured and calculated energy
		double DIFFTHETAMIN = -0.0032; // minimum for difference between measured and calculated theta before rotation
		double DIFFTHETAMAX = 0.0047; // maximum for difference between measured and calculated theta before rotation

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
        double top_topCutX[2] = {21.9999, 0.866766};
        double top_botCutX[2] = {-20.7497, 0.903365};

        double bot_topCutX[2] = {23.4249, 0.867771};
        double bot_botCutX[2] = {-21.4538, 0.905457};

        //Parameters of cut functions for Y
        double top_topCutY[2] = {7.71179, 0.902755};
        double top_botCutY[2] = {-5.92585, 0.905328};

        double bot_topCutY[2] = {4.7885, 0.889888};
        double bot_botCutY[2] = {-6.75031, 0.913876};

        //Upper limit for position dependent energy
        TF1 *func_pde_moller;
        double pars_pde_moller[3] = {2.31531, 0.0953637, 0.000841784}; // 3 sigma


        // Kinematic equations
        TF1* func_E_vs_theta_before_roation;
        TF1* func_theta1_vs_theta2_before_roation;

        // save a tree for information of tracks from vertices
        std::shared_ptr<FlatTupleMaker> _reg_tracks_from_vertices;


};

#endif
