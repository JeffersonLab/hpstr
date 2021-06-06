#ifndef TRIGGERPARAMETERSEXTRACTIONMOLLER_ANAHISTOS_H
#define TRIGGERPARAMETERSEXTRACTIONMOLLER_ANAHISTOS_H

// HPSTR
#include "HistoManager.h"

// C++
#include <string>
#include <vector>

class TriggerParametersExtractionMollerAnaHistos : public HistoManager {

    public:

	TriggerParametersExtractionMollerAnaHistos(const std::string& inputName) : HistoManager(inputName) { m_name = inputName; };

        virtual void Define3DHistos(){};
        virtual void Define2DHistos(){};
        virtual void Define1DHistos();
};

#endif //TRIGGERPARAMETERSEXTRACTIONMOLLER_ANAHISTOS_H
