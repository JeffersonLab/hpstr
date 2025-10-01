#ifndef __APOPTIMIZATION_ANAPROCESSOR_H__
#define __APOPTIMIZATION_ANAPROCESSOR_H__

// HPSTR
#include "OptimizationProcessor.h"
#include "Processor.h"
#include "SimpEquations.h"
#include "TreeCutSelector.h"
#include "Vertex.h"
#include "ZBiHistos.h"

// ROOT
#include "TBranch.h"
// #include "TDataFrame.h"
#include <ROOT/RDataFrame.hxx>

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

    /**
     *@brief description
     */
    bool failPersistentCuts(TTree* tree);

    /**
     *@brief description
     */
    bool failTestCut(std::string cutname, TTree* tree);

    /**
     *@brief description
     */
    void addNewVariables(TTree* tree, std::string variable, double param);

    /**
     *@brief description
     */
    void fillEventHistograms(std::shared_ptr<ZBiHistos> histos, RDataFrame df, bool isBkg = false);

    void configureGraphs(TGraph* zcutscan_zbi_g, TGraph* zcutscan_nsig_g, TGraph* zcutscan_nbkg_g, TGraph* nbkg_zbi_g,
                         TGraph* nsig_zbi_g, std::string cutname);

    RDF::RInterface<Detail::RDF::RLoopManager, void> prepareDF(RDataFrame df);

  private:
    //  Configuration parameters
    std::string outFileName_{"zbi_out.root"};  //<! description

    std::string variableHistCfgFilename_{""};         //<! histogram config file
    bool scan_zcut_ = false;                          //<! use zBi to optimize zcut
    double min_ztail_events_ = 0.5;                   //<! number of events (from fit) past zcut
    std::map<std::string, double> initialIntegrals_;  //<! description

    // Signal config
    std::string signalVtxMCSelection_{""};  //<! description
    TTree* signal_tree_{nullptr};           //<! description

    // Background config
    std::string tritrigFilename_{""};               //<! description
    std::map<std::string, double*> tritrig_tuple_;  //<! description
    TTree* bkg_tree_{nullptr};                      //<! description

    // Total A' Rate terms
    double radFrac_ = 0.0;        //<! radiative fraction (rad/(tritrig+wab))
    std::string eq_cfgFile_{""};  //<! equations config file

    typedef std::map<std::string, std::pair<std::pair<double, double>, int>>::iterator
        range_cut_iter_;  //<! iterator for range cuts

    TreeCutSelector* testCutsSelector_{nullptr};        //<! cuts to be optimized
    TreeCutSelector* persistentCutsSelector_{nullptr};  //<! cuts that are always applied
    std::map<std::string, std::pair<std::pair<double, double>, int>> persistentCutsLog_;  //<! description

    std::map<std::string, std::pair<std::pair<double, double>, int>>* persistentCutsPtr_{nullptr};
    std::map<std::string, std::pair<std::pair<double, double>, int>>* testCutsPtr_{nullptr};

    std::map<std::string, TH1F*> testVarPDFs_;         //<! PDFs for test variables
    std::map<std::string, double*> testVarQuantiles_;  //<! quantiles for test variables
};

#endif
