#ifndef READOUTDIAGNOSTICS_ANAHISTOS_H
#define READOUTDIAGNOSTICS_ANAHISTOS_H

// HPSTR
#include "HistoManager.h"
#include "CalCluster.h"
#include "AnaHelpers.h"

// C++
#include <string>
#include <vector>

class ReadoutDiagnosticsAnaHistos : public HistoManager {

    public:

	ReadoutDiagnosticsAnaHistos(const std::string& inputName) : HistoManager(inputName) { m_name = inputName; };

        virtual void Define3DHistos(){};
        virtual void Define2DHistos(){};
        virtual void Define1DHistos();

        void FillEcalClusters(std::vector<CalCluster*> *ecalClusters, float weight = 1.);

};

#endif //READOUTDIAGNOSTICS_ANAHISTOS_H
