#ifndef RECOPARTICLE_ANAHISTOS_H
#define RECOPARTICLE_ANAHISTOS_H

// HPSTR
#include "HistoManager.h"
#include "Particle.h"
#include <string>
#include <vector>

class RecoParticleAnaHistos : public HistoManager {

    public:

	RecoParticleAnaHistos(const std::string& inputName) : HistoManager(inputName) { m_name = inputName; };

        virtual void Define3DHistos(){};
        virtual void Define2DHistos(){};
        virtual void Define1DHistos();

        void FillFSPs(std::vector<Particle*> *fsp, float weight = 1.);

};

#endif //RECOPARTICLE_ANAHISTOS_H
