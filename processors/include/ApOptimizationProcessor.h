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
// #include "TDataFrame.h"
#include <ROOT/RDataFrame.hxx>
#include <ROOT/RSnapshotOptions.hxx>

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
    void fillEventHistograms(std::shared_ptr<ZBiHistos> histos, RDF::RInterface<Detail::RDF::RLoopManager, void> df,
                             bool isBkg = false);

    void configureGraphs(TGraph* zcutscan_zbi_g, TGraph* zcutscan_nsig_g, TGraph* zcutscan_nbkg_g, TGraph* nbkg_zbi_g,
                         TGraph* nsig_zbi_g, std::string cutname);

    RDF::RInterface<Detail::RDF::RLoopManager, void> prepareDF(RDataFrame df);

    std::vector<std::string> getListOfVariables(json histo_cfg, bool isBkg = false);

    RDF::RInterface<Detail::RDF::RJittedFilter, void> applyFilter(RDF::RInterface<Detail::RDF::RLoopManager, void> df,
                                                                  std::string filter, std::string filter_name = "") {
        return df.Filter(filter, filter_name);
    };

    RDF::RInterface<Detail::RDF::RJittedFilter, void> applyFilter(RDF::RInterface<Detail::RDF::RJittedFilter, void> df,
                                                                  std::string filter, std::string filter_name = "") {
        return df.Filter(filter, filter_name);
    };

    double computeTruthSignalShape(double z, double EAp);
    double computePromptYield(TH1D* h_mass_data_rad_cuts, double bin_width);
    double computeDisplacedYield(TH1D* h_mass_data_rad_cuts, TH1D* h_chi_eff, double EAp, double bin_width);

    std::vector<double> fitZBkgTail(RDF::RResultPtr<TH1D> h_bkg_vtxz, std::string fitname, bool doGausAndTail = false);

    double* getQuantileArray(std::pair<double, double> range, int nquantiles, double original_cut = -999.9);

    double* getBinsAndLimits(json histo_cfg, std::string varname);

    // Get mass resolution in GeV
    double getMassResolution(double mass);

    std::string getHitCategoryCut();

    std::vector<std::pair<double, double>> getZoffsetAlpha(RDF::RResultPtr<TH2D> h_y0_vs_z, int n_quantiles,
                                                           int nbins = 10) {
        double* quantiles = getQuantileArray(std::make_pair(0, 9999.9), n_quantiles);
        std::vector<std::pair<double, double>> zoffset_alpha;
        processorHistos_->addHistoFromDF(h_y0_vs_z);
        for (int q = 0; q < n_quantiles; q++) {
            TGraphErrors* g_y0_vs_z =
                processorHistos_->configureGraph(("g_y0_vs_z_q" + std::to_string(q)).c_str(), "z_{vtx}/mm", "y_{0}/mm");
            processorHistos_->addGraph(g_y0_vs_z);
        }
        std::cout << int(h_y0_vs_z->GetNbinsX() / nbins) << std::endl;
        for (int i = 1; i <= int(h_y0_vs_z->GetNbinsX() / nbins); i++) {
            std::cout << nbins * i << ", " << nbins * (i + 1) << std::endl;
            auto h_proj =
                h_y0_vs_z->ProjectionY(("h_y0_vs_z_py_" + std::to_string(i)).c_str(), nbins * i, nbins * (i + 1));
            processorHistos_->addHisto1d(h_proj);
            if (h_proj->GetEntries() < 50) continue;
            double* quantile_pos = new double[n_quantiles];
            h_proj->GetQuantiles(n_quantiles, quantile_pos, quantiles);
            for (int q = 0; q < n_quantiles; q++) {
                processorHistos_->getGraph(("g_y0_vs_z_q" + std::to_string(q)).c_str())
                    ->AddPoint((double)((TAxis*)h_y0_vs_z->GetXaxis())->GetBinCenter(int(nbins * (i + 0.5))),
                               quantile_pos[q]);
                double error = 0.;
                // if (h_proj->GetEntries() > 0) error = 10. / sqrt(h_proj->GetEntries());
                processorHistos_->getGraph(("g_y0_vs_z_q" + std::to_string(q)).c_str())->SetPointError(q, 0.0, error);
            }
        }
        for (int q = 0; q < n_quantiles; q++) {
            if (q == 0) {
                zoffset_alpha.push_back(std::make_pair(0.0, 0.0));
                continue;
            }
            TF1* f1 = new TF1(("f1_y0_vs_z_q" + std::to_string(q)).c_str(), "[0] + [1]*x", -0.5, 15);
            processorHistos_->getGraph(("g_y0_vs_z_q" + std::to_string(q)).c_str())->Fit(f1, "QRS");

            double zoffset = -f1->GetParameter(0) / f1->GetParameter(1);
            zoffset_alpha.push_back(std::make_pair(zoffset, f1->GetParameter(1)));
            std::cout << "Quantile " << q << " : zoffset = " << zoffset << " alpha = " << f1->GetParameter(1)
                      << std::endl;
        }
        return zoffset_alpha;
    }

  private:
    //  Configuration parameters
    std::string outFileName_{"zbi_out.root"};  //<! description

    std::string variableHistCfgFilename_{""};         //<! histogram config file
    bool scan_zcut_ = false;                          //<! use zBi to optimize zcut
    double min_ztail_events_ = 0.5;                   //<! number of events (from fit) past zcut
    std::map<std::string, double> initialIntegrals_;  //<! description

    // Signal config
    std::string signalVtxSubsetAnaFilename_{""};  //<! description
    std::string signalVtxMCSelection_{""};        //<! description
    TTree* signal_tree_{nullptr};                 //<! description
    TTree* signal_pretrig_sim_tree_{nullptr};     //<! description

    // Background config
    std::string tritrigFilename_{""};               //<! description
    std::map<std::string, double*> tritrig_tuple_;  //<! description
    TTree* bkg_tree_{nullptr};                      //<! description

    // Total A' Rate terms
    double radFrac_ = 0.05;       //<! radiative fraction (rad/(tritrig+wab))
    std::string eq_cfgFile_{""};  //<! equations config file

    double ztarget_ = -0.5;  //<! target position in mm
    double psum_cut_ = 3.0;  //<! Psum cut in GeV

    typedef std::map<std::string, std::pair<std::pair<double, double>, int>>::iterator
        range_cut_iter_;  //<! iterator for range cuts

    TreeCutSelector* testCutsSelector_{nullptr};        //<! cuts to be optimized
    TreeCutSelector* persistentCutsSelector_{nullptr};  //<! cuts that are always applied
    std::map<std::string, std::pair<std::pair<double, double>, int>> persistentCutsLog_;  //<! description

    std::map<std::string, std::pair<std::pair<double, double>, int>>* persistentCutsPtr_{nullptr};
    std::map<std::string, std::pair<std::pair<double, double>, int>>* testCutsPtr_{nullptr};

    std::map<std::string, TH1F*> testVarPDFs_;         //<! PDFs for test variables
    std::map<std::string, TH1F*> testVarCDFs_;         //<! CDFs for test variables
    std::map<std::string, double*> testVarQuantiles_;  //<! quantiles for test variables
    std::map<std::string, std::vector<std::pair<double, double>>>
        testZoffsetAlpha_;  //<! zoffset and alpha for z0 cut, key is cutname, sorted by quantile

    RDF::RSnapshotOptions opts_ = RDF::RSnapshotOptions();  //<! snapshot options

    TH1D* h_pretrig_signal_vtxz_;
    TH1D* h_signal_vtxz_rad_;
    // TH1D* h_signal_mass_;
    // TH1F* h_xi_eff_;
    TF1* f_xi_eff_;

    std::string hit_category_{""};
    std::string massWindow_{""};
};

#endif
