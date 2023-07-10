#ifndef __SIMPZBI_ANAPROCESSOR_H__
#define __SIMPZBI_ANAPROCESSOR_H__

// HPSTR
#include "Processor.h"
#include "ZBiHistos.h"
#include "IterativeCutSelector.h"
#include "SimpEquations.h"
#include "MutableTTree.h"

// ROOT 
#include "TFile.h"
#include "TTree.h"
#include "TRefArray.h"
#include "TBranch.h"
#include "TVector3.h"
#include "TLorentzVector.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TF1.h"
#include "TEfficiency.h"
#include "TGraph.h"

// C++ 
#include <memory>

class SimpZBiOptimizationProcessor : public Processor {

    public:

        SimpZBiOptimizationProcessor(const std::string& name, Process& process);

        ~SimpZBiOptimizationProcessor();

        virtual void configure(const ParameterSet& parameters);

        virtual void initialize(TTree* tree) {};

        virtual bool process(IEvent* event) {};

        virtual void finalize();

        virtual bool process();

        virtual void initialize(std::string inFilename, std::string outFilename);

        double calculateZBi(double n_on, double n_off, double tau);

        bool failPersistentCuts(MutableTTree* MTT);

        bool failTestCut(std::string cutname, MutableTTree* MTT);

        void writeGraph(TFile* outF, std::string folder, TGraph* g);

        double round(double var);

        void getSignalMCAnaVtxZ_h(std::string signalMCAnaFilename, std::string signal_pdgid);

        void addNewVariables(MutableTTree* MTT, std::string variable, double param);

        void fillEventHistograms(std::shared_ptr<ZBiHistos> histos, MutableTTree* MTT);

    private:

        //  Configuration parameters    //
        int debug_{0}; 
        int year_ = 2016;
        std::string cuts_cfgFile_{""};
        std::string outFileName_{"zbi_out.root"};
        TFile* outFile_{nullptr};
        std::vector<std::string> cutVariables_;
        std::vector<std::string> new_variables_;
        std::vector<double> new_variable_params_;
        std::string eq_cfgFile_{""};
        bool scan_zcut_ = false;
        double step_size_ = 0.01;
        int max_iteration_ = 75;
        
        //Background config
        std::string bkgVtxAnaFilename_{""};
        std::string bkgVtxAnaTreename_{""};
        MutableTTree* bkgMTT_{nullptr};
        double min_ztail_events_ = 0.5;
        double background_sf_;

        // Signal //
        std::string variableHistCfgFilename_{""};
        std::string signalVtxAnaFilename_{""};
        std::string signalVtxAnaTreename_{""};
        std::string signalMCAnaFilename_{""};
        std::string signal_pdgid_{""};
        TH1F* signalSimZ_h_{nullptr};
        MutableTTree* signalMTT_{nullptr};
        double signal_sf_ = 1.0;
        double signal_mass_;
        double logEps2_;
        double massResolution_;
        double mass_window_nsigma_;

        //Histograms
        std::shared_ptr<ZBiHistos> signalHistos_;
        std::shared_ptr<ZBiHistos> bkgHistos_;
        std::shared_ptr<ZBiHistos> testCutHistos_;
        std::shared_ptr<ZBiHistos> processorHistos_;

        //cuts
        typedef std::map<std::string, std::pair<double,int>>::iterator cut_iter_;
        std::map<std::string,double> initialIntegrals_;

        //Cuts
        IterativeCutSelector *testCutsSelector_{nullptr};
        std::map<std::string, std::pair<double,int>>* testCutsPtr_;
        IterativeCutSelector *persistentCutsSelector_{nullptr};
        std::map<std::string, std::pair<double,int>>* persistentCutsPtr_;

        //background
        std::string tritrigFilename_{""};
        std::map<std::string,double*> tritrig_tuple_;
        TTree* tritrigTree_{nullptr};

        //simp equations
        SimpEquations* simpEqs_{nullptr};

        //mass window
        double highMass_;
        double lowMass_;

        //Dev
        bool testSpecialCut_ = false;
};

#endif
