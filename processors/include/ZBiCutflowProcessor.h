#ifndef __ZBICUTFLOW_ANAPROCESSOR_H__
#define __ZBICUTFLOW_ANAPROCESSOR_H__

// HPSTR
#include "Processor.h"
#include "ZBiHistos.h"
#include "IterativeCutSelector.h"
#include "SimpEquations.h"

// ROOT 
#include "TFile.h"
#include "TTree.h"
#include "TRefArray.h"
#include "TBranch.h"
#include "TVector3.h"
#include "TLorentzVector.h"
#include "TH1F.h"
#include "TEfficiency.h"

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

        void initializeFlatTuple(TTree* tree, std::map<std::string, double*> &tuple_map);

        double calculateZBi(double n_on, double n_off, double tau);

        bool doesCutVariableExist(std::string cutvariable);

        void printZBiMatrix();

        bool failImpactParameterCut(std::map<std::string, double*> tuple);

        void calculateImpactParameterCut();

        bool failPersistentCuts(std::map<std::string,double*> tuple);

        bool failTestCut(std::string cutname, std::map<std::string,double*> tuple);

        double transformImpactParameterZalpha(double track_z0, double vtx_z);

        bool failImpactParameterZalphaCut(double ele_track_z0, double pos_track_z0, double vtx_z, double zalpha_cut_lt);


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
        std::map<std::string, std::pair<double,int>>* testCuts_;
        std::map<std::string, std::pair<double,int>>* persistentCuts_;
        std::vector<std::string> cutVariables_;
        std::map<std::string,double> initialIntegrals_;
        std::map<std::string,std::vector<std::pair<double,double>>> global_ZBi_map_;

        //cut selector
        IterativeCutSelector *testCutsSelector_{nullptr};
        IterativeCutSelector *persistentCutsSelector_{nullptr};
        ZBiHistos* cutHistos_{nullptr};

        //impact parameter cut parameters for each Test Cut
        //std::map<std::string,std::vector<double>> impact_param_cuts_;
        std::vector<double> impact_param_cut_;

        //histos
        ZBiHistos* debugHistos_{nullptr};

        //signal
        std::string signalHistCfgFilename_{""};
        std::string signalFilename_{""};
        ZBiHistos* signalHistos_{nullptr};
        ZBiHistos* zcutsignalHistos_{nullptr};
        std::map<std::string,double*> signal_tuple_;
        TTree* signalTree_{nullptr};
        //signal pretrigger vtx distribution
        TH1F* vdSimZ_h_{nullptr};

        //background
        std::string tritrigFilename_{""};
        ZBiHistos* tritrigHistos_{nullptr};
        std::map<std::string,double*> tritrig_tuple_;
        TTree* tritrigTree_{nullptr};

        double ztail_nevents_ = 1.0;

        //simp equations
        SimpEquations* simpEqs_{nullptr};

        //mass window
        double highMass_;
        double lowMass_;
};



#endif
