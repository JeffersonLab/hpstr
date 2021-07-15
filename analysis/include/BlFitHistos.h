#ifndef BLFITHISTOS_H
#define BLFITHISTOS_H

#include "TFile.h"
#include "HistoManager.h"
#include "TGraphErrors.h"
#include "TKey.h"
#include "TList.h"
#include "TH1.h"
#include "TrackerHit.h"
#include "RawSvtHit.h"
#include "HistoManager.h"
#include "ModuleMapper.h"
#include "TF1.h"
#include <string>
#include "FlatTupleMaker.h"
#include <math.h>
#include "TCanvas.h"
#include <vector>
#include "ModuleMapper.h"
#include "TFitResult.h"
#include "TF1.h"

class BlFitHistos : public HistoManager{
    
    public:
        BlFitHistos();
        ~BlFitHistos();
    
        std::map<std::string,TH1F*> get1dHistos() {
                return histos1d;
        }
        std::map<std::string,TH2F*> get2dHistos() {
                return histos2d;
        }

        void setSimpleGausFit(std::string simpleGausFit){
            if(simpleGausFit.find("rue") != std::string::npos)
                simpleGausFit_ = true ;
            else
                simpleGausFit_ = false;
        }

        void getHistosFromFile(TFile* inFile, std::string layer = "");
        void Chi2GausFit(std::map<std::string, TH2F*> histos2d, int nPointsDer_,int rebin_i, int xmin_,int minStats_, int noisyRMS_, int deadRMS_, FlatTupleMaker* flat_tuple_);
        TF1* singleGausIterative(TH1D* hist, double sigmaRange, double min, double max, bool iterchi2);
        void iterativeChi2GausFit(TH1D* hist, TF1* fit, double min, double max);
        //TFitResultPtr singleGausIterative(TH1D* hist, double sigmaRange, double min, double max);
        
    private:
        
        TH1F* fitHistos{nullptr};

    protected:
        std::map<std::string, TH2F*> histos2d;
        std::map<std::string, TH1F*> histos1d;
        std::map<std::string,std::map<int,int>> svtIDMap;
        bool simpleGausFit_;
        ModuleMapper * mmapper_;
};

#endif
