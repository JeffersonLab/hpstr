#ifndef RECOHODO_ANAHISTOS_H
#define RECOHODO_ANAHISTOS_H

// HPSTR
#include "HistoManager.h"
#include "HodoHit.h"
#include "HodoCluster.h"
#include <string>
#include <vector>

class RecoHodoAnaHistos : public HistoManager {

    public:

		RecoHodoAnaHistos(const std::string& inputName) : HistoManager(inputName) { m_name = inputName; };

        virtual void Define3DHistos(){};
        virtual void Define2DHistos(){};
        virtual void Define1DHistos();

        void FillHodoHits(std::vector<HodoHit*> *hodoHits, float weight = 1.);
        void FillHodoClusters(std::vector<HodoCluster*> *hodoClusters, float weight = 1.);

};

#endif //RECOHODO_ANAHISTOS_H
