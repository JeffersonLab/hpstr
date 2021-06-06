#ifndef __TRIGGERPARAMETERSEXTRACTIONFEE_ANAPROCESSOR_H__
#define __TRIGGERPARAMETERSEXTRACTIONFEE_ANAPROCESSOR_H__


//HPSTR
#include "HpsEvent.h"
#include "Collections.h"
#include "Track.h"
#include "CalCluster.h"
#include "CalHit.h"
#include "MCParticle.h"
#include "Particle.h"
#include "Processor.h"
#include "HistoManager.h"
#include "TriggerParametersExtractionFEEAnaHistos.h"

#include "BaseSelector.h"

//ROOT
#include "TFile.h"
#include "TTree.h"
#include "TRefArray.h"
#include "TBranch.h"
#include "TVector3.h"
#include "TLorentzVector.h"

//C++
#include <memory>


struct char_cmp {
    bool operator () (const char *a,const char *b) const
    {
        return strcmp(a,b)<0;
    }
};


class TriggerParametersExtractionFEEAnaProcessor : public Processor {

    public:
		TriggerParametersExtractionFEEAnaProcessor(const std::string& name, Process& process);
        ~TriggerParametersExtractionFEEAnaProcessor();
        virtual bool process(IEvent* ievent);

        virtual void initialize(TTree* tree);

        virtual void finalize();

        virtual void configure(const ParameterSet& parameters);

    private:
        //Containers to hold histogrammer info
        TriggerParametersExtractionFEEAnaHistos* histos{nullptr};
        std::string  histCfgFilename_;

        TTree* tree_{nullptr};
        TBranch* btrks_{nullptr};
        TBranch* becalClusters_{nullptr};
        TBranch* bmcParts_{nullptr};

        std::vector<Track*>  * trks_{};
        std::vector<CalCluster*> * ecalClusters_{};
        std::vector<MCParticle*>  * mcParts_{};

        std::string anaName_{"vtxAna"};
        std::string trkColl_{"GBLTracks"};
        std::string ecalClusColl_{"EcalClustersCorr"};
        std::string mcColl_{"MCParticle"};


        double beamE_{3.7};
        int isData_{0};
        std::string analysis_{"triggerParametersExtraction"};

        //Debug level
        int debug_{0};

        // Cut setup, default as for 3.7 GeV
		double CHI2NDFTHRESHOLD = 20;
		double TRACKPMIN = 3.05; // 4 sigma
		double TRACKPMAX = 4.32; // 4 sigma
		double CLUSTERENERGYTHRESHOLD = 0.1; // threshold of cluster energy for analyzable events
		double CLUSTERENERGYMINNOCUT = 2.47; // for no cut; minimum of cluster energy; 3 sigma for double gaussians
		double CLUSTERENERGYMAXNOCUT = 3.60; // for no cut; maximum of cluster energy; 5 sigma for double gaussians
		double CLUSTERENERGYMINANALYZABLE = 2.52; // for analyzable events, minimum of cluster energy; 3 sigma for double gaussians
		double CLUSTERENERGYMAXANALYZABLE = 3.58; // for analyzable events, maximum of cluster energy; 5 sigma for double gaussians
		double CLUSTERNHTSMINNOCUT = 3; // for no cut, minimum for number of cluster's hits
		double CLUSTERNHTSMINANALYZABLE  = 3; // for analyzable events,  minimum for number of cluster's hits

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


        //Cut function for energy slope
        TF1 *func_energy_slope;

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
};

#endif
