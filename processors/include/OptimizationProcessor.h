#ifndef __OPTIMIZATION_ANAPROCESSOR_H__
#define __OPTIMIZATION_ANAPROCESSOR_H__

// HPSTR
#include "BaseSelector.h"
#include "Processor.h"
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
 *@brief Cutflow optimization tool
 */

class OptimizationProcessor : public Processor {
  public:
    OptimizationProcessor(const std::string& name, Process& process);

    ~OptimizationProcessor();

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
    void writeGraph(TFile* outF, std::string folder, TGraph* g);

    /**
     *@brief description
     */
    void writeTH1F(TFile* outF, std::string folder, TH1F* h);

  protected:
    typedef std::map<std::string, std::pair<double, int>>::iterator cut_iter_;  //<! iterator for BaseCutSelector

    // Configuration parameters
    int debug_{0};     //<! debug level
    int year_ = 2021;  //<! year

    std::string cuts_cfgFile_{""};      //<! configuration file for cuts
    std::string outFileName_{""};       //<! output file name
    TFile* outFile_{nullptr};           //<! output file pointer
    TFile* signalVtxAnaFile_{nullptr};  //<! signal vertex analysis file pointer

    std::vector<std::string> cutVariables_;    //<! list of cut variables
    std::vector<std::string> new_variables_;   //<! list of new variables
    std::vector<double> new_variable_params_;  //<! list of new variable parameters
    std::string variableHistCfgFilename_{""};  //<! histogram config file

    // Background config
    std::string bkgVtxAnaFilename_{""};  //<! description
    std::string bkgVtxAnaTreename_{""};  //<! description
    double background_sf_;               //<! background scale factor

    // Signal config
    std::string signalVtxAnaFilename_{""};  //<! description
    std::string signalVtxAnaTreename_{""};  //<! description
    double signal_sf_ = 1.0;                //<! signal scale factor

    std::string signalMCAnaFilename_{""};  //<! pre-trigger signal MC analysis file
    std::string signal_pdgid_{""};         //<! PDG ID of signal particle

    double signal_mass_;         //<! signal mass in GeV
    double eps_;                 //<! coupling strength
    double massResolution_;      //<! mass resolution at signal mass
    double mass_window_nsigma_;  //<! width of mass window in number of sigmas
    double lowMass_;             //<! low edge of mass window
    double highMass_;            //<! high edge of mass window

    // Histograms
    std::shared_ptr<ZBiHistos> signalHistos_;     //<! description
    std::shared_ptr<ZBiHistos> bkgHistos_;        //<! description
    std::shared_ptr<ZBiHistos> testCutHistos_;    //<! description
    std::shared_ptr<ZBiHistos> processorHistos_;  //<! description

    // Cuts
    BaseSelector* testCutsSelector_{nullptr};                           //<! cuts to be optimized
    std::map<std::string, std::pair<double, int>>* testCutsPtr_;        //<! pointer to test cut
    BaseSelector* persistentCutsSelector_{nullptr};                     //<! cuts that are always applied
    std::map<std::string, std::pair<double, int>>* persistentCutsPtr_;  //<! pointer to persistent cut

    double step_size_ = 0.01;  //<! Cut step_size% of signal with each iteration
    int max_iteration_ = 75;   //<! maximum number of iterations
};

#endif
