#ifndef __BEAMROTATIONTARGETOFFSETTING_ANAPROCESSOR_H__
#define __BEAMROTATIONTARGETOFFSETTING_ANAPROCESSOR_H__

//HPSTR
#include "HpsEvent.h"
#include "Collections.h"

//ROOT
#include "Processor.h"
#include "TClonesArray.h"
#include "TBranch.h"
#include "TTree.h"
#include "TFile.h"

#include "Vertex.h"
#include "Particle.h"

#include "RecoParticleAnaHistos.h"

class TTree;


class BeamRotationTargetOffsettingAnaProcessor : public Processor {

    public:

		BeamRotationTargetOffsettingAnaProcessor(const std::string& name, Process& process);

        ~BeamRotationTargetOffsettingAnaProcessor();

        virtual bool process(IEvent* ievent);

        virtual void initialize(TTree* tree);

        virtual void finalize();

        virtual void configure(const ParameterSet& parameters);

    private:

        //Containers to hold histogrammer info
        RecoParticleAnaHistos* histosParticle{nullptr};
        std::string  histCfgFilenameParticle_;

        //TODO Change this to be held from HPSEvent
        TTree* tree_;
        TBranch* bvtxs_{nullptr};
        TBranch* bfsps_{nullptr};

        std::vector<Vertex*> * vtxs_{};
        std::vector<Particle*> * fsps_{};

        std::string anaName_{"beamRotationTargetOffsettingAnaProcessor"};
        std::string vtxColl_{"UnconstrainedV0Vertices_KF"};
        std::string fspCollRoot_{"FinalStateParticles_KF"};

        //Debug Level
        int debug_{0};

};

#endif
