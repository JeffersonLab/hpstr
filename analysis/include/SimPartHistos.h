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
#include <string>
#include <vector>

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
        void FillAcceptance(std::vector<MCParticle*> *MCParticles_, std::vector<Track*> *RecoTracks_, std::vector<TrackerHit*> *RecoTrackerClusters_, std::vector<CalCluster*> *RecoEcalClusters_, float weight = 1.);
  
};

#endif //SIMPARTHISTOS_H
