#include "RecoTrackAnaHistos.h"
#include <math.h>

void RecoTrackAnaHistos::Define1DHistos() {

}

void RecoTrackAnaHistos::FillTrackHits(std::vector<TrackHit*> *hodoHits, float weight ) {
    int nHits = hodoHits->size();
    Fill1DHisto("numTrackHits_h", (float)nHits, weight);
    for (int i=0; i < nHits; i++){
    	TrackHit *hit = hodoHits->at(i);
        Fill1DHisto("hodoHitEnergy_h", hit->getEnergy(), weight);
        Fill1DHisto("hodoHitTime_h", hit->getTime(), weight);

		std::vector<int> hitIndices = hit->getIndices();

		Fill2DHisto("xyIndicesTrackHits_hh",hitIndices[0], hitIndices[1], weight);
    }
}

void RecoTrackAnaHistos::FillTrackClusters(std::vector<TrackCluster*> *hodoClusters, float weight ) {
    int nClusters = hodoClusters->size();
    Fill1DHisto("numTrackClusters_h", (float)nClusters, weight);
    for (int i=0; i < nClusters; i++){
    	TrackCluster *cluster = hodoClusters->at(i);

        Fill1DHisto("hodoClusterEnergy_h", cluster->getEnergy(), weight);
    	Fill1DHisto("hodoClusterTime_h", cluster->getTime(), weight);

		std::vector<int> clusterIndices = cluster->getIndices();
		Fill2DHisto("xyIndicesTrackClusters_hh",clusterIndices[0], clusterIndices[1], weight);
    }
}
