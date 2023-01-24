#include "ZBiHistos.h"
#include <sstream>

ZBiHistos::ZBiHistos() {
}

ZBiHistos::~ZBiHistos() {
}

ZBiHistos::ZBiHistos(const std::string& inputName){
    m_name = inputName;
}

void ZBiHistos::addHistoClone1d(TH1F* parentHisto, std::string clone_histoname){
    std::string histoname = m_name + clone_histoname;
    TH1F* clone = (TH1F*)parentHisto->Clone(histoname.c_str());    
    histos1d[histoname] = clone;
}

void ZBiHistos::addHisto1d(std::string histoname, std::string xtitle, int nbinsX, float xmin, float xmax){
    histos1d[m_name+"_"+histoname] = plot1D(m_name+"_"+histoname, xtitle, nbinsX, xmin, xmax);
}

double ZBiHistos::getIntegral(std::string histoname){
    int xmax; 
    int xmin;
    double integral;

    if(!histos1d.count(histoname)){
        std::cout << "NO HISTOGRAM NAMED " << histoname << " FOUND IN DEFINED HISTOS";
        integral = -9999.9;
    }
    else{
        xmax = histos1d[histoname]->FindLastBinAbove(0.0);
        xmin = histos1d[histoname]->FindFirstBinAbove(0.0);
        integral = histos1d[histoname]->Integral(xmin,xmax);
    }
    return integral;
}

double ZBiHistos::cutFractionOfIntegral(std::string histoname, bool isCutGreaterThan, double cutFraction, double initialIntegral){

    TH1F* histo = histos1d[histoname];
    int xmax = histo->FindLastBinAbove(0.0);
    int xmin = histo->FindFirstBinAbove(0.0);
    
    double cutvalue;
    if(isCutGreaterThan){
       cutvalue = histo->GetXaxis()->GetBinLowEdge(xmin); 
       while(histo->Integral(xmin,xmax) > initialIntegral*(1.0-cutFraction)){
           xmin = xmin + 1;
           cutvalue = histo->GetXaxis()->GetBinLowEdge(xmin);
       }
    }
    else {
       cutvalue = histo->GetXaxis()->GetBinUpEdge(xmax); 
       while(histo->Integral(xmin,xmax) > initialIntegral*(1.0-cutFraction)){
           xmax = xmax - 1;
           cutvalue = histo->GetXaxis()->GetBinUpEdge(xmax);
       }
    }

    if(debug_)
        std::cout << "[ZBiHistos] cut value = " << cutvalue << std::endl;
    return cutvalue;
}

double ZBiHistos::fitZTail(std::string zVtxHistoname, double max_tail_events){
    TF1* fitFunc = new TF1("fitfunc","[0]*exp( (((x-[1])/[2])<[3])*(-0.5*(x-[1])^2/[2]^2) + (((x-[1])/[2])>=[3])*(0.5*[3]^2-[3]*(x-[1])/[2]))", -100.0, 100.0);
    
    std::string histoname = m_name+"_"+zVtxHistoname;
    if(histos1d[histoname]->GetEntries() < 1)
        return -4.3;
    TFitResultPtr gausResult = (TFitResultPtr)histos1d[histoname]->Fit("gaus","QS"); 
    double gaus1 = gausResult->GetParams()[1];
    double gaus2 = gausResult->GetParams()[2];
    gausResult = histos1d[histoname]->Fit("gaus","QS","",gaus1-3.0*gaus2, gaus1+10.0*gaus2);
    gaus1 = gausResult->GetParams()[1];
    gaus2 = gausResult->GetParams()[2];
    double tailZ = gaus1 + 3.0*gaus2;
    double bestChi2 = -99.9;
    double bestParams[4] = {999.9,999.9,999.9,999.9};
    double bestFitInit[4] = {999.9,999.9,999.9,999.9};

    fitFunc->SetParameters(gausResult->GetParams()[0], gaus1, gaus2, 3.0);
    TFitResultPtr fitResult = (TFitResultPtr)histos1d[histoname]->Fit(fitFunc, "LSIM", "", gaus1-2.0*gaus2, gaus1+10.0*gaus2);
    
    double zcut = -6.0;
    double testIntegral = fitFunc->Integral(zcut, 90.0);
    while(testIntegral > max_tail_events){
        zcut = zcut+0.1;
        testIntegral = fitFunc->Integral(zcut, 90.0);
    }
    return zcut;
}
