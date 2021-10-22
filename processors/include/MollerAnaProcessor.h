#ifndef __MOLLER_ANAPROCESSOR_H__
#define __MOLLER_ANAPROCESSOR_H__


//HPSTR
#include "HpsEvent.h"
#include "TSData.h"
#include "Collections.h"
#include "Track.h"
#include "CalCluster.h"
#include "CalHit.h"
#include "Particle.h"
#include "Vertex.h"
#include "Processor.h"
#include "HistoManager.h"
#include "MollerAnaHistos.h"

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


class MollerAnaProcessor : public Processor {

    public:
		MollerAnaProcessor(const std::string& name, Process& process);
        ~MollerAnaProcessor();
        virtual bool process(IEvent* ievent);

        virtual void initialize(TTree* tree);

        virtual void finalize();

        virtual void configure(const ParameterSet& parameters);

    private:
        std::shared_ptr<BaseSelector> trackSelector;
        std::string trackSelectionCfg_;

        //Containers to hold histogrammer info
        MollerAnaHistos* trackHistos{nullptr};
        std::string  histCfgFilename_;

        TTree* tree_{nullptr};
        TBranch* btsData_{nullptr};
        TBranch* btrks_{nullptr};
        TBranch* bvtxs_{nullptr};

        TSData* tsData_{};
        std::vector<Track*>  * trks_{};
        std::vector<Vertex*> * vtxs_{};

        std::string anaName_{"mollerAna"};
        std::string tsColl_{"TSBank"};
        std::string trkColl_{"KalmanFullTracks"};
        std::string vtxColl_{"UnconstrainedMollerVertices"};


        double beamE_{3.7};
        int isData_{1};
        std::string analysis_{"MollerAnalysis"};

        //Debug level
        int debug_{0};

        std::shared_ptr<AnaHelpers> _ah;

        // Kinematic equations
        TF1* func_E_vs_theta_before_roation;
        TF1* func_theta1_vs_theta2_before_roation;

        // save a tree for information of tracks from vertices
        std::shared_ptr<FlatTupleMaker> _reg_tracks_from_vertices;

        //Scale for momentum correction
        double MOMSCALE = 1.761/2.09;


};

#endif
