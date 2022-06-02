#ifndef CLUSTERHISTOS_H
#define CLUSTERHISTOS_H

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


class Svt2DBlHistos : public HistoManager{

    public:
        Svt2DBlHistos(const std::string& inputName, ModuleMapper* mmapper_);
        ~Svt2DBlHistos();

        void DefineHistos();
        void FillHistograms(std::vector<RawSvtHit*> *rawSvtHits_,float weight = 1.);


    private:

        int Event_number=0;
        int debug_ = 1;

        TH1F* svtCondHisto{nullptr};  
        //ModuleMapper
        ModuleMapper* mmapper_;

};


#endif
