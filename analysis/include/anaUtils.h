#ifndef BLABLA_H
#define BLABLA_H


#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <algorithm>

#include "TH1F.h"
#include "TCanvas.h"
#include "TString.h"
#include "TFile.h"
#include "TKey.h"
#include "TGraphErrors.h"
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include "dirent.h"
#include "TMath.h"
#include "TLatex.h"
#include "TF1.h"
#include "TH2D.h"
//#include "TIter.h"
#include "TKey.h"
#include "TRandom3.h"
using namespace std;

namespace utils {
    Double_t langaufun(Double_t *x, Double_t *par);

    TF1 *langaufit(TH1F *his, Double_t *fitrange, Double_t *startvalues, Double_t *parlimitslo, Double_t *parlimitshi, Double_t *fitparams, Double_t *fiterrors, Double_t *ChiSqr, Int_t *NDF);

    void GetMean(TH1* hist, std::pair<float,float> &info, int method = 0,TFile* outfile =nullptr,float* fitRange=nullptr);

    void GetMean(TH1F* hist, std::pair<float,float> &info, std::pair<float,float>& info_width,int method =
            0,TFile* outfile = nullptr,std::string runNumber="0",std::string name="");

    void getAll2DHistos(std::vector<TH2D*>& histos, const string& filename, const string& regEx = "");
    TH1F* getHisto(const string &histoname,const  string &filename);

    std::pair<TGraphErrors*,TGraphErrors*> FillPlot(TH2* histo2d, const std::string y_title, int method,TFile* outfile,int binning,float* fitRange);

    std::pair<TGraphErrors*,TGraphErrors*> FillPlot(const string & plotname, const string& layer, const vector<string> &sample, int colour, const string&  y_title, bool runNo, int method, TFile* outFile);

    string getSubString(const string& s, const string& start_delim,const string& stop_delim);

    void Get2DHistosFromFile(std::map<std::string, TH2F*>& histos2d, std::vector<std::string>& histos2dk,TFile* inFile, std::string folder="", std::string timesample="");

    void fitCheck(std::map<std::string, TH2F*> histos2d, std::string histo_key, std::map<std::string, TH1D*>& singleChannel_h, std::map<std::string, double> means);

    void getFitParams(TFile* inFile, std::map<std::string, double>& means, std::string sensor);
}
#endif
