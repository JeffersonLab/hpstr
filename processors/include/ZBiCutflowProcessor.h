#ifndef __ZBICUTFLOW_ANAPROCESSOR_H__
#define __ZBICUTFLOW_ANAPROCESSOR_H__

// HPSTR
#include "Processor.h"
#include "ZBiHistos.h"
#include "IterativeCutSelector.h"

// ROOT 
#include "TFile.h"
#include "TTree.h"
#include "TRefArray.h"
#include "TBranch.h"
#include "TVector3.h"
#include "TLorentzVector.h"

// C++ 
#include <memory>

class ZBiCutflowProcessor : public Processor {

    public:

        ZBiCutflowProcessor(const std::string& name, Process& process);

        ~ZBiCutflowProcessor();

        virtual void configure(const ParameterSet& parameters);

        virtual void initialize(TTree* tree) {};

        virtual bool process(IEvent* event) {};

        virtual void finalize();

        virtual bool process();

        virtual void initialize(std::string inFilename, std::string outFilename);

        void readFlatTuple(TTree* tree, std::map<std::string, double*> &tuple_map);


    private:

        int debug_{0}; 
        std::string outFileName_{"zbi_out.root"};
        std::string cuts_cfgFile_{""};
        std::string radSlicFilename_{""};
        std::string vdSimFilename_{""};
        double vdMassMeV_;
        double ApMassMeV_;

        std::map<std::string,double> mcScale_;

        std::vector<std::string> cutlist_strings_{};
        std::vector<std::string> cut_vars_{};

        TFile* outFile_{nullptr};

        //cuts
        typedef std::map<std::string, std::pair<double,int>>::iterator cut_iter_;
        std::map<std::string, std::pair<double,int>> cuts_;
        std::map<std::string,double> initialIntegrals_;

        //cut selector
        IterativeCutSelector *cutSelector_{nullptr};
        ZBiHistos* cutHistos_{nullptr};

        //signal
        std::string signalHistCfgFilename_{""};
        std::string signalFilename_{""};
        ZBiHistos* signalHistos_{nullptr};
        std::map<std::string,double*> signal_tuple_;
        TTree* signalTree_{nullptr};

        //background
        std::string tritrigFilename_{""};
        ZBiHistos* tritrigHistos_{nullptr};
        std::map<std::string,double*> tritrig_tuple_;
        TTree* tritrigTree_{nullptr};
        
        

};



#endif
