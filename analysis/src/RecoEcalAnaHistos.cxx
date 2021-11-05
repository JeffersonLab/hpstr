#include "RecoEcalAnaHistos.h"
#include <math.h>

void RecoEcalAnaHistos::Define1DHistos() {

}

void RecoEcalAnaHistos::FillEcalHits(std::vector<CalHit*> *ecalHits, float weight ) {
    int nHits = ecalHits->size();
    Fill1DHisto("numEcalHits_h", (float)nHits, weight);
    for (int i=0; i < nHits; i++){
        CalHit *hit = ecalHits->at(i);
        Fill1DHisto("ecalHitEnergy_h", hit->getEnergy(), weight);

		std::vector<int> hitIndices = hit->getCrystalIndices();
		Fill2DHisto("xyIndicesEcalHits_hh",hitIndices[0], hitIndices[1], weight);
    }
}

void RecoEcalAnaHistos::FillEcalClusters(std::vector<CalCluster*> *ecalClusters, float weight ) {
    int nClusters = ecalClusters->size();
    Fill1DHisto("numEcalClusters_h", (float)nClusters, weight);
    for (int i=0; i < nClusters; i++){
        CalCluster *cluster = ecalClusters->at(i);

        Fill1DHisto("ecalClusterEnergy_h", cluster->getEnergy(), weight);
    	Fill1DHisto("ecalClusterNHits_h", cluster->getNHits(), weight);
    	Fill1DHisto("ecalClusterTime_h", cluster->getTime(), weight);

		std::vector<double> positionCluster = cluster->getPosition();
		double xCluster = positionCluster[0];
		double yCluster = positionCluster[1];
		Fill1DHisto("ecalClusterX_h", xCluster, weight);
		Fill1DHisto("ecalClusterY_h", yCluster, weight);
		Fill2DHisto("xyEcalClusters_hh",xCluster, yCluster, weight);

		CalHit* seed = (CalHit*)cluster->getSeed();
		Fill1DHisto("ecalSeeEnergy_h", seed->getEnergy(), weight);

		std::vector<int> seedIndices = seed->getCrystalIndices();
		Fill2DHisto("xyIndicesEcalSeeds_hh",seedIndices[0], seedIndices[1], weight);
    }
}
