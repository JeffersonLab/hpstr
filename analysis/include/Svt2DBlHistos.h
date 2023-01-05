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

/**
 * @brief description
 *
 * details
 */
class Svt2DBlHistos : public HistoManager{

    public:
        /**
         * @brief Constructor
         *
         * @param inputName
         * @param mmapper_
         */
        Svt2DBlHistos(const std::string& inputName, ModuleMapper* mmapper_);
        ~Svt2DBlHistos();

        /** 
         * @brief description
         *
         */
        void DefineHistos();

        /**
         * @brief description
         *
         * @param rawSvtHits_
         * @param weight
         */
        void FillHistograms(std::vector<RawSvtHit*> *rawSvtHits_,float weight = 1.);


    private:

        int Event_number=0; //!< description
        int debug_ = 1; //!< description

        TH1F* svtCondHisto{nullptr}; //!< description 
        //ModuleMapper
        ModuleMapper* mmapper_; //!< description
};
#endif
