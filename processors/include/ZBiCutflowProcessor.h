#ifndef __ZBICUTFLOW_ANAPROCESSOR_H__
#define __ZBICUTFLOW_ANAPROCESSOR_H__

// HPSTR
#include "Processor.h"
#include "ZBiHistos.h"

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
        std::string signalHistCfgFilename_{""};
        std::string outFileName_{"zbi_out.root"};
        std::string cuts_cfgFile_{""};
        std::string tritrigFilename_{""};
        std::string radSlicFilename_{""};
        std::string vdSimFilename_{""};
        double vdMassMeV_;
        double ApMassMeV_;

        std::vector<std::string> cutlist_strings_{};
        std::vector<std::string> cut_vars_{};

        TFile* outFile_{nullptr};

        typedef std::map<std::string, double>::iterator itercuts;
        std::map<std::string, double> cuts_;

        //signal
        std::string signalFilename_{""};
        ZBiHistos* signalHistos_{nullptr};
        std::map<std::string,double*> signal_tuple_;
        TTree* signalTree_{nullptr};
        

};



#endif
