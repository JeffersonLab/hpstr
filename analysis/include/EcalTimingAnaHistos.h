#ifndef ECALTIMING_ANAHISTOS_H
#define ECALTIMING_ANAHISTOS_H

// HPSTR
#include "HistoManager.h"
#include "TSData.h"
#include "VTPData.h"
#include "CalCluster.h"
#include "Vertex.h"
#include "AnaHelpers.h"

// C++
#include <string>
#include <vector>

class EcalTimingAnaHistos : public HistoManager {

    public:

	EcalTimingAnaHistos(const std::string& inputName) : HistoManager(inputName) { m_name = inputName; };

        virtual void Define3DHistos(){};
        virtual void Define2DHistos(){};
        virtual void Define1DHistos();

        void FillTSData(TSData* tsData, float weight = 1.);

        void FillEcalClusters(std::vector<CalCluster*> *ecalClusters, float weight = 1.);

        void FillTargetConstrainedV0s(std::vector<Vertex*> *vtxs, float weight = 1.);

};

#endif //ECALTIMING_ANAHISTOS_H
