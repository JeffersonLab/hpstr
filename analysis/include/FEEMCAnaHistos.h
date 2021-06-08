#ifndef FEEMC_ANAHISTOS_H
#define FEEMC_ANAHISTOS_H

// HPSTR
#include "HistoManager.h"
#include "CalCluster.h"
#include "Track.h"
#include "CalHit.h"
#include "AnaHelpers.h"

// C++
#include <string>
#include <vector>

class FEEMCAnaHistos : public HistoManager {

    public:

		FEEMCAnaHistos(const std::string& inputName) : HistoManager(inputName) { m_name = inputName; };

        virtual void Define3DHistos(){};
        virtual void Define2DHistos(){};
        virtual void Define1DHistos();

        void FillEcalClusters(std::vector<CalCluster*> *ecalClusters, float weight = 1.);
        void FillGTPClusters(std::vector<CalCluster*> *gtpClusters, float weight = 1.);
        void FillTracks(std::vector<Track*> *trks, float weight = 1.);

};

#endif //FEEMC_ANAHISTOS_H
