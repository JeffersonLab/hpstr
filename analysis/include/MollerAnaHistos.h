#ifndef MOLLER_ANAHISTOS_H
#define MOLLER_ANAHISTOS_H

// HPSTR
#include "HistoManager.h"

// C++
#include <string>
#include <vector>

class MollerAnaHistos : public HistoManager {

    public:

	MollerAnaHistos(const std::string& inputName) : HistoManager(inputName) { m_name = inputName; };

        virtual void Define3DHistos(){};
        virtual void Define2DHistos(){};
        virtual void Define1DHistos();
};

#endif //MOLLER_ANAHISTOS_H
