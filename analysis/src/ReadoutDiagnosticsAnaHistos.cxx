#include "ReadoutDiagnosticsAnaHistos.h"
#include <math.h>

void ReadoutDiagnosticsAnaHistos::Define1DHistos() {

}

void ReadoutDiagnosticsAnaHistos::FillEcalClusters(std::vector<CalCluster*> *ecalClusters, float weight ) {
    int nClusters = ecalClusters->size();
    Fill1DHisto("numEcalClusters_h", (float)nClusters, weight);
    for (int i=0; i < nClusters; i++)
    {
        CalCluster *cluster = ecalClusters->at(i);
        Fill1DHisto("ecalClusterEnergy_h", cluster->getEnergy(), weight);
        Fill1DHisto("ecalClusterTime_h", cluster->getTime(), weight); // ns
        Fill2DHisto("ecalClusterTimeVsEnergy_hh", cluster->getEnergy(), cluster->getTime(), weight);
        Fill2DHisto("ecalClusterXY_hh", cluster->getPosition()[0], cluster->getPosition()[1], weight);
    }
}
