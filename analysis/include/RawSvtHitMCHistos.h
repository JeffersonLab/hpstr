#ifndef RAWSVTHITMCHISTOS_H
#define RAWSVTHITMCHISTOS_H

#include "TFile.h"
#include "HistoManager.h"
#include "TGraphErrors.h"
#include "TKey.h"
#include "TList.h"
#include "TH1.h"
#include "RawSvtHit.h"

#include "ModuleMapper.h"

#include <string>


class RawSvtHitMCHistos : public HistoManager{

    public:
        RawSvtHitMCHistos(const std::string& inputName);
        ~RawSvtHitMCHistos();

        virtual void Define3DHistos(){};
        virtual void Define2DHistos(){};
        virtual void Define1DHistos(){};

        void FillHistograms(std::vector<RawSvtHit*> *rawSvtHits_,float weight = 1.);

    private:

        int Event_number=0;

};


#endif
