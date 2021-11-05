#ifndef RECOTRACK_ANAHISTOS_H
#define RECOTRACK_ANAHISTOS_H

// HPSTR
#include "HistoManager.h"
#include "TrackHit.h"
#include "TrackCluster.h"
#include <string>
#include <vector>

class RecoTrackAnaHistos : public HistoManager {

    public:

		RecoTrackAnaHistos(const std::string& inputName) : HistoManager(inputName) { m_name = inputName; };

        virtual void Define3DHistos(){};
        virtual void Define2DHistos(){};
        virtual void Define1DHistos();

        void FillTrackHits(std::vector<TrackHit*> *hodoHits, float weight = 1.);
        void FillTrackClusters(std::vector<TrackCluster*> *hodoClusters, float weight = 1.);

};

#endif //RECOTRACK_ANAHISTOS_H
