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

#include "ModuleMapper.h"

#include <string>

class BlFitHistos : public HistoManager{
    
    public:
        BlFitHistos(const std::string& inputName);
        ~BlFitHistos();
    
        void FillHistograms();
        
    private:
        
        TH1F* fitHistos{nullptr};
        std::map<std::string, TGraphErrors*> baselineGraphs;
};

#endif
