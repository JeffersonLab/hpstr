#ifndef __RECOPARTICLE_ANAPROCESSOR_H__
#define __RECOPARTICLE_ANAPROCESSOR_H__

//HPSTR
#include "HpsEvent.h"
#include "Collections.h"
#include "RecoParticleAnaHistos.h"


//ROOT
#include "Processor.h"
#include "TClonesArray.h"
#include "TBranch.h"
#include "TTree.h"
#include "TFile.h"

class TTree;


class RecoParticleAnaProcessor : public Processor {

    public:

		RecoParticleAnaProcessor(const std::string& name, Process& process);

        ~RecoParticleAnaProcessor();

        virtual bool process(IEvent* ievent);

        virtual void initialize(TTree* tree);

        virtual void finalize();

        virtual void configure(const ParameterSet& parameters);

    private:

        //Containers to hold histogrammer info
        RecoParticleAnaHistos* histos{nullptr};
        std::string  histCfgFilename_;

        //TODO Change this to be held from HPSEvent
        TTree* tree_;
        TBranch* bfsps_{nullptr};
        std::vector<Particle*> * fsps_{};

        std::string anaName_{"recoParticleAna"};
        std::string fspCollRoot_{"FinalStateParticles_KF"};


        //Debug Level
        int debug_{0};

};

#endif
