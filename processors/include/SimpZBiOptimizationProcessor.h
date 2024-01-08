#ifndef __SIMPZBI_ANAPROCESSOR_H__
#define __SIMPZBI_ANAPROCESSOR_H__

// HPSTR
#include "Processor.h"
#include "ZBiHistos.h"
#include "IterativeCutSelector.h"
#include "SimpEquations.h"
#include "SimpAnaTTree.h"

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

/**
 *@brief Cutflow optimization tool for SIMPS
 */

class SimpZBiOptimizationProcessor : public Processor {

    public:

        SimpZBiOptimizationProcessor(const std::string& name, Process& process);

        ~SimpZBiOptimizationProcessor();

        /**
         *@brief description
         */
        virtual void configure(const ParameterSet& parameters);

        /**
         *@brief description
         */
        virtual void initialize(TTree* tree) {};

        /**
         *@brief description
         */
        virtual bool process(IEvent* event) {};

        /**
         *@brief description
         */
        virtual void finalize();

        /**
         *@brief description
         */
        virtual bool process();

        /**
         *@brief description
         */
        virtual void initialize(std::string inFilename, std::string outFilename);

        /**
         *@brief description
         */
        double calculateZBi(double n_on, double n_off, double tau);

        /**
         *@brief description
         */
        bool failPersistentCuts(SimpAnaTTree* MTT);

        /**
         *@brief description
         */
        bool failTestCut(std::string cutname, SimpAnaTTree* MTT);

        /**
         *@brief description
         */
        void writeGraph(TFile* outF, std::string folder, TGraph* g);

        /**
         *@brief description
         */
        void writeTH1F(TFile* outF, std::string folder, TH1F* h);

        /**
         *@brief description
         */
        double round(double var);

        /**
         *@brief description
         */
        void getSignalMCAnaVtxZ_h(std::string signalMCAnaFilename, std::string signal_pdgid);

        /**
         *@brief description
         */
        double countControlRegionBackgroundRate(std::string inFilename, std::string tree_name, 
                double m_Ap, double Mbin=30.0, double dNdm_sf = 1.0);

        /**
         *@brief description
         */
        void addNewVariables(SimpAnaTTree* MTT, std::string variable, double param);

        /**
         *@brief description
         */
        void fillEventHistograms(std::shared_ptr<ZBiHistos> histos, SimpAnaTTree* MTT);

    private:

        //  Configuration parameters    //
        int debug_{0}; //!< //<! description
        int year_ = 2016; //<! description
        std::string cuts_cfgFile_{""}; //<! description
        std::string outFileName_{"zbi_out.root"}; //<! description
        TFile* outFile_{nullptr}; //<! description
        std::vector<std::string> cutVariables_; //<! description
        std::vector<std::string> new_variables_; //<! description
        std::vector<double> new_variable_params_; //<! description
        std::string eq_cfgFile_{""}; //<! description
        bool scan_zcut_ = false; //<! description
        double step_size_ = 0.01; //<! description
        int max_iteration_ = 75; //<! description

        //Background config
        std::string bkgVtxAnaFilename_{""}; //<! description
        std::string bkgVtxAnaTreename_{""}; //<! description
        SimpAnaTTree* bkgMTT_{nullptr}; //<! description
        double min_ztail_events_ = 0.5; //<! description
        double background_sf_; //<! description

        // Signal //
        std::string variableHistCfgFilename_{""}; //<! description
        std::string signalVtxAnaFilename_{""}; //<! description
        std::string signalVtxAnaTreename_{""}; //<! description
        std::string signalVtxMCSelection_{""}; //<! description
        std::string signalMCAnaFilename_{""}; //<! description
        std::string signal_pdgid_{""}; //<! description
        TH1F* signalSimZ_h_{nullptr}; //<! description
        SimpAnaTTree* signalMTT_{nullptr}; //<! description
        double signal_sf_ = 1.0; //<! description
        double signal_mass_; //<! description
        double logEps2_; //<! description
        double massResolution_; //<! description
        double mass_window_nsigma_; //<! description
        double E_Vd_ = 1.35; //<! mean truth signal energy...default is 1.35

        //Histograms 
        std::shared_ptr<ZBiHistos> signalHistos_; //<! description
        std::shared_ptr<ZBiHistos> bkgHistos_; //<! description
        std::shared_ptr<ZBiHistos> testCutHistos_; //<! description
        std::shared_ptr<ZBiHistos> processorHistos_; //<! description

        //cuts 
        typedef std::map<std::string, std::pair<double,int>>::iterator cut_iter_; //<! description
        std::map<std::string,double> initialIntegrals_; //<! description

        //Cuts
        IterativeCutSelector *testCutsSelector_{nullptr}; //<! description
        std::map<std::string, std::pair<double,int>>* testCutsPtr_; //<! description
        IterativeCutSelector *persistentCutsSelector_{nullptr}; //<! description
        std::map<std::string, std::pair<double,int>>* persistentCutsPtr_; //<! description

        //backgroun
        std::string tritrigFilename_{""}; //<! description
        std::map<std::string,double*> tritrig_tuple_; //<! description
        TTree* tritrigTree_{nullptr}; //<! description

        //simp equations
        SimpEquations* simpEqs_{nullptr}; //<! description

        //Total A' Rate terms
        double radFrac_ = 0.0;
        double radAcc_ = 0.0;
        double dNdm_ = 0.0;
        double dNdm_sf_ = 1.0;
        std::string bkgControlRegionFilename_{""};
        std::string bkgControlRegionTreename_{""};

        //mass window 
        double highMass_; //<! description
        double lowMass_; //<! description

};

#endif
