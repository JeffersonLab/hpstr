#include "RecoHodoAnaHistos.h"
#include <math.h>

void RecoHodoAnaHistos::Define1DHistos() {

}

void RecoHodoAnaHistos::FillHodoHits(std::vector<HodoHit*> *hodoHits, float weight ) {
    int nHits = hodoHits->size();
    Fill1DHisto("numHodoHits_h", (float)nHits, weight);
    for (int i=0; i < nHits; i++){
    	HodoHit *hit = hodoHits->at(i);
        Fill1DHisto("hodoHitEnergy_h", hit->getEnergy(), weight);
        Fill1DHisto("hodoHitTime_h", hit->getTime(), weight);

		std::vector<int> hitIndices = hit->getIndices();

		Fill2DHisto("xyIndicesHodoHits_hh",hitIndices[0], hitIndices[1], weight);
    }
}

void RecoHodoAnaHistos::FillHodoClusters(std::vector<HodoCluster*> *hodoClusters, float weight ) {
    int nClusters = hodoClusters->size();
    Fill1DHisto("numHodoClusters_h", (float)nClusters, weight);
    for (int i=0; i < nClusters; i++){
    	HodoCluster *cluster = hodoClusters->at(i);

        Fill1DHisto("hodoClusterEnergy_h", cluster->getEnergy(), weight);
    	Fill1DHisto("hodoClusterTime_h", cluster->getTime(), weight);

		std::vector<int> clusterIndices = cluster->getIndices();
		Fill2DHisto("xyIndicesHodoClusters_hh",clusterIndices[0], clusterIndices[1], weight);
    }
}
