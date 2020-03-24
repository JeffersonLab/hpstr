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
        BlFitHistos(const std::string& inputName);
        ~BlFitHistos();
    
        void Chi2GausFit( HistoManager* inputHistos_, int nPointsDer_,int rebin_i, int xmin_,int minStats_, FlatTupleMaker* flat_tuple_);
        
    private:
        
        TH1F* fitHistos{nullptr};
        std::map<std::string, TGraphErrors*> baselineGraphs;
};

#endif
