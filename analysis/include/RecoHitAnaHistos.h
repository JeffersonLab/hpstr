#ifndef RECOHIT_ANAHISTOS_H
#define RECOHIT_ANAHISTOS_H

// HPSTR
#include "HistoManager.h"
#include "TrackerHit.h"
#include "Track.h"
#include "CalHit.h"
#include "CalCluster.h"
#include <string>
#include <vector>

class RecoHitAnaHistos : public HistoManager {

    public:

        RecoHitAnaHistos(const std::string& inputName) : HistoManager(inputName) { m_name = inputName; };

        virtual void Define3DHistos(){};
        virtual void Define2DHistos(){};
        virtual void Define1DHistos();

        void FillTrackerHits(std::vector<TrackerHit*> *trkrHits, float weight = 1.);
        void FillTracks(std::vector<Track*> *tracks, float weight = 1.);
        void FillEcalHits(std::vector<CalHit*> *ecalHits, float weight = 1.);
        void FillEcalClusters(std::vector<CalCluster*> *ecalClusters, float weight = 1.);

};

#endif //RECOHIT_ANAHISTOS_H
