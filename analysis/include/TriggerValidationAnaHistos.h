#ifndef TRIGGERVALIDATION_ANAHISTOS_H
#define TRIGGERVALIDATION_ANAHISTOS_H

// HPSTR
#include "HistoManager.h"
#include "VTPData.h"
#include "CalCluster.h"
#include "CalHit.h"
#include "TSData.h"
#include "AnaHelpers.h"

// C++
#include <string>
#include <vector>

class TriggerValidationAnaHistos : public HistoManager {

    public:

	TriggerValidationAnaHistos(const std::string& inputName) : HistoManager(inputName) { m_name = inputName; };

        virtual void Define3DHistos(){};
        virtual void Define2DHistos(){};
        virtual void Define1DHistos();

        void FillTSData(TSData* tsData, float weight = 1.);
        void FillEcalClusters(std::vector<CalCluster*> *ecalClusters, float weight = 1.);
        void FillVTPData(VTPData *vtpData, float weight = 1.);

};

#endif //TRIGGERVALIDATION_ANAHISTOS_H
