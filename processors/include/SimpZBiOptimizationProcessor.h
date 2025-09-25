#ifndef __SIMPZBI_ANAPROCESSOR_H__
#define __SIMPZBI_ANAPROCESSOR_H__

// HPSTR
#include "IterativeCutSelector.h"
#include "OptimizationProcessor.h"
#include "Processor.h"
#include "SimpAnaTTree.h"
#include "SimpEquations.h"
#include "ZBiHistos.h"

// ROOT
#include "TBranch.h"
#include "TEfficiency.h"
#include "TF1.h"
#include "TFile.h"
#include "TGraph.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TLorentzVector.h"
#include "TRefArray.h"
#include "TTree.h"
#include "TVector3.h"

// C++
#include <memory>

/**
 *@brief Cutflow optimization tool for SIMPS
 */

class SimpZBiOptimizationProcessor : public OptimizationProcessor {
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
    double countControlRegionBackgroundRate(std::string inFilename, std::string tree_name, double m_Ap,
                                            double Mbin = 30.0, double dNdm_sf = 1.0);

    /**
     *@brief description
     */
    void addNewVariables(SimpAnaTTree* MTT, std::string variable, double param);

    /**
     *@brief description
     */
    void fillEventHistograms(std::shared_ptr<ZBiHistos> histos, SimpAnaTTree* MTT);

  private:
    //  Configuration parameters
    int year_ = 2016;                          //<! description
    std::string outFileName_{"zbi_out.root"};  //<! description

    std::string eq_cfgFile_{""};                      //<! equations config file
    std::string variableHistCfgFilename_{""};         //<! histogram config file
    bool scan_zcut_ = false;                          //<! use zBi to optimize zcut
    double min_ztail_events_ = 0.5;                   //<! number of events (from fit) past zcut
    std::map<std::string, double> initialIntegrals_;  //<! description

    // Signal config
    std::string signalVtxMCSelection_{""};  //<! description
    TH1F* signalSimZ_h_{nullptr};           //<! truth z distribution of signal vertex
    SimpAnaTTree* signalMTT_{nullptr};      //<! description

    double logEps2_;      //<! log10(eps^2)
    double E_Vd_ = 1.35;  //<! mean truth signal energy...default is 1.35 GeV

    // SIMP equations
    SimpEquations* simpEqs_{nullptr};  //<! description

    // Background config
    SimpAnaTTree* bkgMTT_{nullptr};                 //<! description
    std::string tritrigFilename_{""};               //<! description
    std::map<std::string, double*> tritrig_tuple_;  //<! description
    TTree* tritrigTree_{nullptr};                   //<! description

    // Total A' Rate terms
    double radFrac_ = 0.0;  //<! radiative fraction (rad/(tritrig+wab))
    double radAcc_ = 0.0;
    double dNdm_ = 0.0;
    double dNdm_sf_ = 1.0;
    std::string bkgControlRegionFilename_{""};
    std::string bkgControlRegionTreename_{""};
};

#endif
