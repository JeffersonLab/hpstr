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

        void getHistosFromFile(TFile* inFile, std::vector<std::string> hybrid);
        void Chi2GausFit(std::map<std::string, TH2F*> histos2d, int nPointsDer_,int rebin_i, int xmin_,int minStats_, FlatTupleMaker* flat_tuple_);
        
    private:
        
        TH1F* fitHistos{nullptr};

    protected:
        std::map<std::string, TH2F*> histos2d;
        std::map<std::string, TH1F*> histos1d;
};

#endif
