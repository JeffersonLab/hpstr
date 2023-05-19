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

        void printZBiMatrix();

        bool failPersistentCuts(MutableTTree* MTT);

        bool failTestCut(std::string cutname, MutableTTree* MTT);

        bool failTestCut(std::string cutname, std::map<std::string,double*> tuple);

        void getVdSimZ();

        void writeGraph(TFile* outF, std::string folder, TGraph* g);

        double round(double var);

        //get signal truth vtx z distribution. Used to calculate expected
        //signal
        void getSignalMCAnaVtxZ_h(std::string signalMCAnaFilename, std::string signal_pdgid);

        void testImpactParameterCut(MutableTTree* MTT, ZBiHistos* histos);

        void addNewVariables(MutableTTree* MTT, std::string variable, double param);

        void fillEventHistograms(std::shared_ptr<ZBiHistos> histos, MutableTTree* MTT);

    private:

        //  Configuration parameters    //
        int year_ = 2016;
        std::string cuts_cfgFile_{""};
        std::string outFileName_{"zbi_out.root"};
        std::vector<std::string> cutVariables_;
        std::vector<std::string> new_variables_;
        std::vector<double> new_variable_params_;
        std::string eq_cfgFile_{""};
        bool scan_zcut_ = false;
        double step_size_ = 0.01;
        int max_iteration_ = 75;
        
        //Background config
        double min_ztail_events_ = 0.5;
        double background_sf_;
        std::string bkgVtxAnaFilename_{""};
        std::string bkgVtxAnaTreename_{""};

        // Signal //
        std::string variableHistCfgFilename_{""};
        std::string signalVtxAnaFilename_{""};
        std::string signalVtxAnaTreename_{""};
        std::string signalMCAnaFilename_{""};
        std::string signal_pdgid_{""};
        double signal_mass_;
        double massResolution_;
        double mass_window_nsigma_;

        //Expected Signal Calculation //
        double signal_sf_ = 1.0;
        double radFrac_;
        double radAcc_ = 0.0;
        double dNdm_;
        double logEps2_;

        //Histograms
        std::shared_ptr<ZBiHistos> signalHistos_;
        std::shared_ptr<ZBiHistos> bkgHistos_;
        std::shared_ptr<ZBiHistos> testCutHistos_;
        std::shared_ptr<ZBiHistos> processorHistos_;
        /*
        ZBiHistos* signalHistos_{nullptr};
        ZBiHistos* bkgHistos_{nullptr};
        ZBiHistos* cutHistos_{nullptr};
        ZBiHistos* debugHistos_{nullptr};
        ZBiHistos* summaryHistos_{nullptr};
        */

        std::map<std::string,double> mcScale_;
        int debug_{0}; 
        TFile* outFile_{nullptr};
        double vdMassMeV_;
        double ApMassMeV_;

        //cuts
        typedef std::map<std::string, std::pair<double,int>>::iterator cut_iter_;
        std::map<std::string,double> initialIntegrals_;
        std::map<std::string,std::vector<std::pair<double,double>>> global_ZBi_map_;

        //Cuts
        IterativeCutSelector *testCutsSelector_{nullptr};
        std::map<std::string, std::pair<double,int>>* testCutsPtr_;
        IterativeCutSelector *persistentCutsSelector_{nullptr};
        std::map<std::string, std::pair<double,int>>* persistentCutsPtr_;


        //signal

        //background
        std::string tritrigFilename_{""};
        std::map<std::string,double*> tritrig_tuple_;
        TTree* tritrigTree_{nullptr};

        //simp equations
        SimpEquations* simpEqs_{nullptr};

        //mass window
        double highMass_;
        double lowMass_;

        TH1F* signalSimZ_h_{nullptr};
        MutableTTree* signalMTT_{nullptr};

        // Background //
        MutableTTree* bkgMTT_{nullptr};

        double luminosity_;


        // ZAlpha Cut Variable 
        double zalpha_slope_;;

        //Dev
        bool testSpecialCut_ = false;
};



#endif
