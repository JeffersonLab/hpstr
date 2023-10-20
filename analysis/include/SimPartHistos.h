#ifndef SIMPARTHISTOS_H
#define SIMPARTHISTOS_H

// ROOT
#include "TLorentzVector.h"

// HPSTR
#include "HistoManager.h"
#include "MCParticle.h"
#include "MCTrackerHit.h"
#include "MCEcalHit.h"
#include "Track.h"
#include "TrackerHit.h"
#include "CalCluster.h"
#include "FlatTupleMaker.h"
#include <string>
#include <vector>
#include <cmath>

/**
 * @brief description
 * 
 * details
 */
class SimPartHistos : public HistoManager {

    public:
        /**
         * @brief Constructor
         * 
         * @param inputName 
         */
        SimPartHistos(const std::string& inputName) : HistoManager(inputName) { m_name = inputName; };

        /**
         * @brief description
         * 
         * @param MCParticles 
         * @param RecoTracks 
         * @param RecoTrackerClusters 
         * @param RecoEcalClusters 
         * @param weight 
         */
        void FillMCParticle(MCParticle* part, FlatTupleMaker* tuples, float weight = 1.);
        void FillRecoTrack(Track* track, FlatTupleMaker* tuples, float weight = 1.);
        void FillRecoEcalCuster(CalCluster* cluster, FlatTupleMaker* tuples, float weight = 1.);
};

#endif //SIMPARTHISTOS_H
