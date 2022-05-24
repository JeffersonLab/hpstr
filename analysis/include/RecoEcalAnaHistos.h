#ifndef RECOECAL_ANAHISTOS_H
#define RECOECAL_ANAHISTOS_H

// HPSTR
#include "HistoManager.h"
#include "CalHit.h"
#include "CalCluster.h"
#include <string>
#include <vector>

class RecoEcalAnaHistos : public HistoManager {

    public:

		RecoEcalAnaHistos(const std::string& inputName) : HistoManager(inputName) { m_name = inputName; };

        virtual void Define3DHistos(){};
        virtual void Define2DHistos(){};
        virtual void Define1DHistos();

        void FillEcalHits(std::vector<CalHit*> *ecalHits, float weight = 1.);
        void FillEcalClusters(std::vector<CalCluster*> *ecalClusters, float weight = 1.);

};

#endif //RECOECAL_ANAHISTOS_H
