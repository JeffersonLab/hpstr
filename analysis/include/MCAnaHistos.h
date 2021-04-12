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

class MCAnaHistos : public HistoManager {

    public:

        MCAnaHistos(const std::string& inputName) : HistoManager(inputName) { m_name = inputName; };

        virtual void Define3DHistos(){};
        virtual void Define2DHistos();
        virtual void Define1DHistos(){};

        void FillMCParticles(std::vector<MCParticle*> *mcParts,std::string analysis, float weight = 1.);
        void FillMCTrackerHits(std::vector<MCTrackerHit*> *mcTrkrHits, float weight = 1.);
        void FillMCEcalHits(std::vector<MCEcalHit*> *mcEcalHits, float weight = 1.);

};

#endif //MCANAHISTOS_H
