#ifndef __APOPTIMIZATION_ANAPROCESSOR_H__
#define __APOPTIMIZATION_ANAPROCESSOR_H__

// HPSTR
#include "OptimizationProcessor.h"
#include "Particle.h"
#include "Processor.h"
#include "SimpEquations.h"
#include "TreeCutSelector.h"
#include "Vertex.h"
#include "ZBiHistos.h"

// ROOT
#include "TBranch.h"
#include "TEfficiency.h"
#include "TF1.h"
#include "TFile.h"
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TLorentzVector.h"
#include "TRefArray.h"
#include "TTree.h"
#include "TTreeReader.h"
#include "TVector3.h"

// C++
#include <memory>

using namespace ROOT;

/**
 *@brief Cutflow optimization tool for A'
 */
class ApOptimizationProcessor : public OptimizationProcessor {
  public:
    ApOptimizationProcessor(const std::string& name, Process& process);

    ~ApOptimizationProcessor();

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

    // void configureGraphs(TGraph* zcutscan_zbi_g, TGraph* zcutscan_nsig_g, TGraph* zcutscan_nbkg_g, TGraph*
    // nbkg_zbi_g,
    //                      TGraph* nsig_zbi_g, std::string cutname);

    void determineSignalVertexEfficiencyXi(TTree* signal_subset_tree);
    double computeTruthSignalShape(double z, double EAp);
    double computePromptYield();
    double computeDisplacedYield(TH1* h_chi_eff, double EAp);

    std::vector<double> fitZBkgTail(TH1* h_bkg_vtxz, std::string fitname);

    double* getBinsAndLimits(json histo_cfg, std::string varname);

    // Get mass resolution in GeV
    double getMassResolution(double mass);

    std::string getHitCategoryCut();
    void initializeTestCutPDF(std::string cut_name, std::string cut_var, std::string persistent_cuts);
    void initializeGraphs(std::string cut_name);

    std::vector<std::pair<double, double>> getZoffsetAlpha(TH2F* h_y0_vs_z, int n_quantiles, int nbins);
    double* getQuantileArray(std::pair<double, double> range, int n_quantiles, double initial_cut_frac = -999.9);

    std::vector<double> findZcut(TH1* h_bkg_vtxz_cut, bool fromTailFit, int iteration = 0);

  private:
    //  Configuration parameters
    std::string outFileName_{"zbi_out.root"};  //<! description

    std::string variableHistCfgFilename_{""};  //<! histogram config file
    bool scan_zcut_ = false;                   //<! use zBi to optimize zcut
    double min_ztail_events_ = 1;              //<! number of events (from fit) past zcut
    double fixed_zcut_ = -1.0;                 //<! fixed zcut value in mm

    // Signal config
    std::string signalVtxSubsetAnaFilename_{""};  //<! description
    std::string signalVtxMCSelection_{""};        //<! description
    TTree* signal_tree_{nullptr};                 //<! description
    TTree* signal_pretrig_sim_tree_{nullptr};     //<! description

    // Background config
    TTree* bkg_tree_{nullptr};  //<! description

    // Total A' Rate terms
    double radFrac_ = 0.05;  //<! radiative fraction (rad/(tritrig+wab))

    double ztarget_ = -0.5;  //<! target position in mm
    double psum_cut_ = 3.0;  //<! Psum cut in GeV

    typedef std::map<std::string, std::pair<std::pair<double, double>, int>>::iterator
        cut_iter_;  //<! iterator for cuts

    TreeCutSelector* testCutsSelector_{nullptr};        //<! cuts to be optimized
    TreeCutSelector* persistentCutsSelector_{nullptr};  //<! cuts that are always applied

    std::map<std::string, std::string> persistentCutStrings_;

    std::map<std::string, std::pair<std::pair<double, double>, int>>* persistentCutsPtr_{nullptr};
    std::map<std::string, std::pair<std::pair<double, double>, int>>* testCutsPtr_{nullptr};

    std::map<std::string, TH1F*> testVarPDFs_;           //<! PDFs for test variables
    std::map<std::string, TH1F*> testVarCDFs_;           //<! CDFs for test variables
    std::map<std::string, double*> testVarQuantiles_;    //<! quantiles for test variables
    std::map<std::string, double*> testVarQuantilePos_;  //<! quantile positions for test variables
    std::map<std::string, std::vector<std::pair<double, double>>>
        testZoffsetAlpha_;  //<! zoffset and alpha for z0 cut, key is cutname, sorted by quantile

    TH1D* h_pretrig_signal_vtxz_;
    TH1D* h_signal_vtxz_rad_subset_;
    TH1D* h_signal_vtxz_rad_;
    TH1D* h_data_mass_rad_;
    // TH1D* h_signal_mass_;
    // TH1F* h_xi_eff_;
    TF1* f_xi_eff_;

    std::string hit_category_{""};
    std::string massWindow_{""};
    std::string initialCuts_{""};

    double zbins_[3]{400., -50., 150.};
    std::string zbin_str_ =
        "(" + std::to_string((int)zbins_[0]) + "," + std::to_string(zbins_[1]) + "," + std::to_string(zbins_[2]) + ")";
    double z_bin_width_ = (zbins_[2] - zbins_[1]) / zbins_[0];
    double massbins_[3]{200., 0., 0.4};
    double mass_bin_width_ = (massbins_[2] - massbins_[1]) / massbins_[0];
};

#endif
