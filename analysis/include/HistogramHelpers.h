#ifndef HISTOGRAMHELPERS_H
#define HISTOGRAMHELPERS_H


#include "TF1.h"
#include "TH3.h"
#include "TH2.h"
#include "TH1.h"
#include "TFitResultPtr.h"
#include "TCanvas.h"
#include "TFile.h"
#include "TStyle.h"

using namespace std;


namespace HistogramHelpers {
    /**
     * @brief description
     * 
     * @param x 
     * @param par 
     * @return double 
     */
    double GaussExpTails_f(double* x, double *par);

    /**
     * @brief description
     * 
     * @param x 
     * @param par 
     * @return double 
     */
    double twoGaussExp_f(double* x, double *par);

    /**
     * @brief description
     * 
     * @param x 
     * @param par 
     * @return double 
     */
    double twoGauss_f(double*x, double* par);

    /**
     * @brief description
     * 
     * @param hist 
     * @param mu 
     * @param mu_err 
     * @param sigma 
     * @param sigma_err 
     * @param m_PrintLevel 
     * @return int 
     */
    int IterativeGaussFit(TH1* hist, double &mu, double &mu_err, double &sigma,
                          double &sigma_err, int m_PrintLevel = 0);

    /**
     * @brief description
     * 
     * @param hist 
     * @param fcn 
     * @param initial 
     * @param fitRange 
     * @param fitr_p 
     * @param m_PrintLevel 
     * @return TF1* 
     */
    TF1* IterativeGeneralFit(TH1* hist, double(*fcn)(double *, double *),
                             const std::vector<float>& initial,
                             const std::vector<float>& fitRange,
                             TFitResultPtr *fitr_p, int m_PrintLevel);

    /**
     * @brief description
     * 
     * @param hist 
     * @param mu_graph 
     * @param sigma_graph 
     * @param num_bins 
     * @param m_PrintLevel 
     */
    void profileYwithIterativeGaussFit(TH2* hist, TH1* mu_graph, TH1* sigma_graph,
                                       int num_bins = 1, int m_PrintLevel = 0);

    /**
     * @brief description
     * 
     * @param hist 
     * @param mu_graph 
     * @param sigma_graph 
     * @param num_bins 
     * @param mu_err_graph 
     * @param sigma_err_graph 
     */
    void profileZwithIterativeGaussFit(TH3* hist, TH2* mu_graph, TH2* sigma_graph,
                                       int num_bins, TH2* mu_err_graph, TH2* sigma_err_graph);

    /**
     * @brief description
     * 
     * @param hist 
     * @param m_PrintLevel 
     */
    void   HistogramConditioning (TH1* hist, int m_PrintLevel =0);

    /**
     * @brief description
     * 
     */
    void OpenProjectionFile();

    /**
     * @brief description
     * 
     */
    void CloseProjectionFile();

    /** description */
    TFile* outFile_for_projections;
} 
  
#endif
