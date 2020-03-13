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
        Svt2DBlHistos(const std::string& inputName);
        ~Svt2DBlHistos();

        virtual void Define3DHistos(){};
        virtual void Define2DHistos(){};
        virtual void Define1DHistos(){};


        void FillHistograms(std::vector<RawSvtHit*> *rawSvtHits_,float weight = 1.);
        void get2DHistoOccupancy(std::vector<std::string> histos2dNames);
        void setBaselineFitsDir(const std::string& baselineFits) {baselineFits_ = baselineFits;};
        bool LoadBaselineHistos(const std::string& baselineRun);


    private:

        int Event_number=0;
        int debug_ = 1;

        TH1F* svtCondHisto{nullptr};  

        std::string baselineFits_{"/nfs/hps3/svtTests/jlabSystem/baselines/fits/"};
        std::string baselineRun_{""};

        std::map<std::string, TGraphErrors*> baselineGraphs;

};


#endif
