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

class BlFitHistos : public HistoManager{
    
    public:
        BlFitHistos(const std::string& inputName);
        ~BlFitHistos();
    
        void Chi2GausFit( HistoManager* inputHistos_, int nPointsDer_,int rebin_i, int xmin_,int minStats_, FlatTupleMaker* flat_tuple_);
        void FillHistograms();
        void Mean2DHistoOverlay(HistoManager* inputHistos_, BlFitHistos* outputHistos_);
        
    private:
        
        TH1F* fitHistos{nullptr};
        std::map<std::string, TGraphErrors*> baselineGraphs;
        //int rebin_{};
        //int nPointsDer_{};
};

#endif
