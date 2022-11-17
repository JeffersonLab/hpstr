#ifndef MCANAHISTOS_H
#define MCANAHISTOS_H

// ROOT
#include "TLorentzVector.h"

// HPSTR
#include "HistoManager.h"
#include "MCParticle.h"
#include "MCTrackerHit.h"
#include "MCEcalHit.h"
#include <string>
#include <vector>

/**
 * @brief description
 * 
 * details
 */
class MCAnaHistos : public HistoManager {

    public:
        /**
         * @brief Constructor
         * 
         * @param inputName 
         */
        MCAnaHistos(const std::string& inputName) : HistoManager(inputName) { m_name = inputName; };

        /**
         * @brief description
         * 
         */
        virtual void Define3DHistos(){};

        /**
         * @brief description
         * 
         */
        virtual void Define2DHistos();

        /**
         * @brief description
         * 
         */
        virtual void Define1DHistos(){};

        /**
         * @brief description
         * 
         * @param mcParts 
         * @param analysis 
         * @param weight 
         */
        void FillMCParticles(std::vector<MCParticle*> *mcParts, std::string analysis, float weight = 1.);
        
        /**
         * @brief description
         * 
         * @param mcTrkrHits 
         * @param weight 
         */
        void FillMCTrackerHits(std::vector<MCTrackerHit*> *mcTrkrHits, float weight = 1.);

        /**
         * @brief description
         * 
         * @param mcEcalHits 
         * @param weight 
         */
        void FillMCEcalHits(std::vector<MCEcalHit*> *mcEcalHits, float weight = 1.);

};

#endif //MCANAHISTOS_H
