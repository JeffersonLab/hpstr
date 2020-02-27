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
  double GaussExpTails_f(double* x, double *par);
  double twoGaussExp_f(double* x, double *par);
  double twoGauss_f(double*x, double* par);
  int IterativeGaussFit(TH1* hist, double &mu, double &mu_err, double &sigma, double &sigma_err,int m_PrintLevel = 0);
  TF1* IterativeGeneralFit(TH1* hist, double(*fcn)(double *, double *),const std::vector<float>& initial, const std::vector<float>& fitRange, TFitResultPtr *fitr_p, int m_PrintLevel);
  void profileYwithIterativeGaussFit(TH2* hist, TH1* mu_graph, TH1* sigma_graph, int num_bins = 1, int m_PrintLevel = 0);
  void profileZwithIterativeGaussFit(TH3* hist, TH2* mu_graph, TH2* sigma_graph, int num_bins, TH2* mu_err_graph, TH2* sigma_err_graph);
  void   HistogramConditioning (TH1* hist, int m_PrintLevel =0);
  void OpenProjectionFile();
  void CloseProjectionFile();  
  TFile* outFile_for_projections; 
  
} 
  
#endif
  
