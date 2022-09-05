#ifndef RAWSVTHITHISTOS_H
#define RAWSVTHITHISTOS_H

#include "TFile.h"
#include "HistoManager.h"
#include "TGraphErrors.h"
#include "TKey.h"
#include "TList.h"
#include "TH1.h"
#include "RawSvtHit.h"
//include "BaseSelectorSVT.h"
//#include "AnaHelpers.h"

//#include "BaseSelector.h"
//#include "AnaHelpers.h"

#include "ModuleMapper.h"

#include <string>


class RawSvtHitHistos2 : public HistoManager{

    public:
        RawSvtHitHistos(const std::string& inputName, ModuleMapper* mmapper_);
        ~RawSvtHitHistos();

        void DefineHistos();
        void FillHistograms(RawSvtHit* rawSvtHit,float weight = 1.,int Ireg=0,unsigned int nhit = 0,Float_t TimeDiff = -42069.0,Float_t AmpDiff = -42069.0);
        void saveHistosSVT(TFile* outF,std::string folder);
    private:

        int Event_number=0;
        int debug_ = 1;

        TH1F* svtCondHisto{nullptr};  
        //std::map<std::string,std::shared_ptr<BaseSelector>> reg_selectors_;
        //ModuleMapperi
        ModuleMapper* mmapper_;
        //std::shared_ptr<BaseSelector> reg_selector;
        std::vector<std::string> regions_;
        std::vector<std::string> hybridNames;
        std::vector<std::string> hybridNames2;
        std::string swTag;
};


#endif
