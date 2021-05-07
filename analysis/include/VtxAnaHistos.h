#ifndef VTX_ANAHISTOS_H
#define VTX_ANAHISTOS_H

// HPSTR
#include "HistoManager.h"
#include "TSData.h"
#include "Vertex.h"
#include "AnaHelpers.h"

// C++
#include <string>
#include <vector>

class VtxAnaHistos : public HistoManager {

    public:

	VtxAnaHistos(const std::string& inputName) : HistoManager(inputName) { m_name = inputName; };

        virtual void Define3DHistos(){};
        virtual void Define2DHistos(){};
        virtual void Define1DHistos();

        void FillTSData(TSData* tsData, float weight = 1.);

        void FillUnconstrainedV0s(std::vector<Vertex*> *vtxs, float weight = 1.);

};

#endif //VTX_ANAHISTOS_H
