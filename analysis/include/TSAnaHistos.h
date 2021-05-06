#ifndef TS_ANAHISTOS_H
#define TS_ANAHISTOS_H

// HPSTR
#include "HistoManager.h"
#include "TSData.h"
#include "Vertex.h"
#include "AnaHelpers.h"

// C++
#include <string>
#include <vector>

class TSAnaHistos : public HistoManager {

    public:

	TSAnaHistos(const std::string& inputName) : HistoManager(inputName) { m_name = inputName; };

        virtual void Define3DHistos(){};
        virtual void Define2DHistos(){};
        virtual void Define1DHistos();

        void FillTSData(TSData* tsData, float weight = 1.);

};

#endif //TS_ANAHISTOS_H
