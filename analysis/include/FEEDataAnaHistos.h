#ifndef FEEDATA_ANAHISTOS_H
#define FEEDATA_ANAHISTOS_H

// HPSTR
#include "HistoManager.h"
#include "TSData.h"
#include "VTPData.h"
#include "CalCluster.h"
#include "Track.h"
#include "AnaHelpers.h"

// C++
#include <string>
#include <vector>

class FEEDataAnaHistos : public HistoManager {

    public:

	FEEDataAnaHistos(const std::string& inputName) : HistoManager(inputName) { m_name = inputName; };

        virtual void Define3DHistos(){};
        virtual void Define2DHistos(){};
        virtual void Define1DHistos();

        void FillEcalClusters(std::vector<CalCluster*> *ecalClusters, float weight = 1.);
        void FillVTPData(VTPData* vtpData, float weight = 1.);
        void FillTracks(std::vector<Track*> *trks, float weight = 1.);

};

#endif //FEEDATA_ANAHISTOS_H
