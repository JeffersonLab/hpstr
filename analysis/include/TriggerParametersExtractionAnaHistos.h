#ifndef TRIGGERPARAMETERSEXTRACTION_ANAHISTOS_H
#define TRIGGERPARAMETERSEXTRACTION_ANAHISTOS_H

// HPSTR
#include "HistoManager.h"

// C++
#include <string>
#include <vector>

class TriggerParametersExtractionAnaHistos : public HistoManager {

    public:

	TriggerParametersExtractionAnaHistos(const std::string& inputName) : HistoManager(inputName) { m_name = inputName; };

        virtual void Define3DHistos(){};
        virtual void Define2DHistos(){};
        virtual void Define1DHistos();
};

#endif //TRIGGERPARAMETERSEXTRACTION_ANAHISTOS_H
