#ifndef ZBIHISTOS_H
#define ZBIHISTOS_H

#include <math.h>

// #include <ROOT/RDF/RInterface.hxx>
#include <ROOT/RDataFrame.hxx>
#include <fstream>
#include <string>
#include <vector>

#include "HistoManager.h"
#include "MutableTTree.h"
#include "TCanvas.h"
#include "TF1.h"
#include "TFile.h"
#include "TFitResult.h"
#include "TFitResultPtr.h"
#include "TGraphErrors.h"
#include "TH1.h"
#include "TH1F.h"
#include "TKey.h"
#include "TRandom3.h"

using namespace ROOT;

/**
 * @brief description
 *
 * more details
 */
class ZBiHistos : public HistoManager {
  public:
    ZBiHistos();
    ~ZBiHistos();

    ZBiHistos(const std::string& inputName);
    /**
     * @brief description
     *
     */
    virtual void Define3DHistos() {};

    /**
     * @brief description
     *
     */
    virtual void Define2DHistos() {};

    /**
     * @brief description
     *
     */
    void writeHistos(TFile* outF, std::string folder);

    void writeHistosFromDF(TFile* outF, std::string folder);

    /**
     * @brief description
     *
     */
    void resetHistograms1d();

    /**
     * @brief description
     *
     */
    void resetHistograms2d();

    /**
     * @brief description
     *
     */
    void addHisto1d(std::string histoname, std::string xtitle, int nbinsX, float xmin, float xmax);

    void addHisto1d(TH1* histo);

    void addGraph(TGraphErrors* graph);

    /**
     * @brief description
     *
     */
    void addHisto2d(std::string histoname, std::string xtitle, int nbinsX, float xmin, float xmax, std::string ytitle,
                    int nbinsY, float ymin, float ymax);

    void addHisto2d(TH2* histo);
    /**
     * @brief description
     *
     */
    void set2DHistoYlabel(std::string histoName, int ybin, std::string ylabel);

    /**
     * @brief description
     *
     */
    std::vector<double> defineImpactParameterCut(double alpha = 0.15);

    /**
     * @brief description
     *
     */
    void defineZBiCutflowProcessorHistograms();

    /**
     * @brief Set debug
     *
     * @param value
     */
    void setDebug(bool value) { debug_ = value; };

    /**
     * @brief description
     *
     */
    double cutFractionOfSignalVariable(std::string cutvariable, bool isCutGreaterThan, double cutFraction,
                                       double initialIntegral);

    /**
     * @brief description
     *
     */
    double integrateHistogram1D(std::string histoname);

    /**
     * @brief description
     *
     */
    TF1* fitExponentialTail(std::string histogramName, double start_nevents);

    /**
     * @brief description
     *
     */
    TF1* fitExponentialPlusConst(std::string histogramName, double starnt_nevents);
    /**
     * @brief description
     *
     */
    TF1* fitExponentialPlusExp(std::string histogramName, double starnt_nevents);
    /**
     * @brief description
     *
     */
    double fitZTail(std::string zVtxHistoname, double max_tail_events);

    /**
     * @brief description
     *
     */
    double shosFitZTail(std::string cutname, double max_tail_events);

    /**
     * @brief description
     *
     */
    TF1* getZTailFit(std::string cutname);

    /**
     * @brief description
     *
     */
    void writeGraphs(TFile* outF, std::string folder);

    TH1* getPDF(std::string histoname);

    void addHistoFromDF(ROOT::RDF::RResultPtr<TH1D> df_histo);

    void addHistoFromDF(ROOT::RDF::RResultPtr<TH2D> df_histo);

    json getConfig() { return _h_configs; };

    TGraphErrors* getGraph(std::string graphname) {
        if (graphs_.find(m_name + "_" + graphname) != graphs_.end()) {
            return graphs_[m_name + "_" + graphname];
        } else {
            std::cout << "Error: could not find graph " << m_name + "_" + graphname << std::endl;
            return nullptr;
        }
    };

    TGraphErrors* configureGraph(std::string name, std::string xlabel, std::string ylabel);

  private:
    std::map<std::string, TGraphErrors*> graphs_;  //!< hold graphs
    std::vector<ROOT::RDF::RResultPtr<TH1D>> df_histos_;
    std::vector<ROOT::RDF::RResultPtr<TH2D>> df_histos2d_;
};

#endif
