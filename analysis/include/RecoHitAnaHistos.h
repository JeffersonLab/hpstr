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

/**
 * @brief description
 * 
 * details
 */
class RecoHitAnaHistos : public HistoManager {

    public:
        /**
         * @brief constructor
         * 
         * @param inputName 
         */
        RecoHitAnaHistos(const std::string& inputName) : HistoManager(inputName) { m_name = inputName; };

        /**
         * @brief description
         * 
         */
        virtual void Define3DHistos(){};

        /**
         * @brief description
         * 
         */
        virtual void Define2DHistos(){};

        /**
         * @brief description
         * 
         */
        virtual void Define1DHistos();

        /**
         * @brief description
         * 
         * @param trkrHits 
         * @param weight 
         */
        void FillTrackerHits(std::vector<TrackerHit*> *trkrHits, float weight = 1.);

        /**
         * @brief description
         * 
         * @param tracks 
         * @param weight 
         */
        void FillTracks(std::vector<Track*> *tracks, float weight = 1.);

        /**
         * @brief description
         * 
         * @param ecalHits 
         * @param weight 
         */
        void FillEcalHits(std::vector<CalHit*> *ecalHits, float weight = 1.);

        /**
         * @brief description
         * 
         * @param ecalClusters 
         * @param weight 
         */
        void FillEcalClusters(std::vector<CalCluster*> *ecalClusters, float weight = 1.);

};

#endif //RECOHIT_ANAHISTOS_H
