#ifndef TRIGGERPARAMETERSEXTRACTIONFEE_ANAHISTOS_H
#define TRIGGERPARAMETERSEXTRACTIONFEE_ANAHISTOS_H

// HPSTR
#include "HistoManager.h"

// C++
#include <string>
#include <vector>

class TriggerParametersExtractionFEEAnaHistos : public HistoManager {

    public:

	TriggerParametersExtractionFEEAnaHistos(const std::string& inputName) : HistoManager(inputName) { m_name = inputName; };

        virtual void Define3DHistos(){};
        virtual void Define2DHistos(){};
        virtual void Define1DHistos();
};

#endif //TRIGGERPARAMETERSEXTRACTIONFEE_ANAHISTOS_H
