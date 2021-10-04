#include "TriggerValidationAnaHistos.h"
#include <math.h>

void TriggerValidationAnaHistos::Define1DHistos() {

}

void TriggerValidationAnaHistos::FillTSData(TSData* tsData, float weight ){
	if(tsData->prescaled.Single_0_Top == true) Fill1DHisto("ts_h", 0, weight);
	if(tsData->prescaled.Single_1_Top == true) Fill1DHisto("ts_h", 1, weight);
	if(tsData->prescaled.Single_2_Top == true) Fill1DHisto("ts_h", 2, weight);
	if(tsData->prescaled.Single_3_Top == true) Fill1DHisto("ts_h", 3, weight);

	if(tsData->prescaled.Single_0_Bot == true) Fill1DHisto("ts_h", 4, weight);
	if(tsData->prescaled.Single_1_Bot == true) Fill1DHisto("ts_h", 5, weight);
	if(tsData->prescaled.Single_2_Bot == true) Fill1DHisto("ts_h", 6, weight);
	if(tsData->prescaled.Single_3_Bot == true) Fill1DHisto("ts_h", 7, weight);

	if(tsData->prescaled.Pair_0 == true) Fill1DHisto("ts_h", 8, weight);
	if(tsData->prescaled.Pair_1 == true) Fill1DHisto("ts_h", 9, weight);
	if(tsData->prescaled.Pair_2 == true) Fill1DHisto("ts_h", 10, weight);
	if(tsData->prescaled.Pair_3 == true) Fill1DHisto("ts_h", 11, weight);

	if(tsData->prescaled.LED == true) Fill1DHisto("ts_h", 12, weight);
	if(tsData->prescaled.Cosmic == true) Fill1DHisto("ts_h", 13, weight);
	if(tsData->prescaled.Hodoscope == true) Fill1DHisto("ts_h", 14, weight);
	if(tsData->prescaled.Pulser == true) Fill1DHisto("ts_h", 15, weight);

	if(tsData->prescaled.Mult_0 == true) Fill1DHisto("ts_h", 16, weight);
	if(tsData->prescaled.Mult_1 == true) Fill1DHisto("ts_h", 17, weight);

	if(tsData->prescaled.FEE_Top == true) Fill1DHisto("ts_h", 18, weight);
	if(tsData->prescaled.FEE_Bot == true) Fill1DHisto("ts_h", 19, weight);
}


void TriggerValidationAnaHistos::FillEcalClusters(std::vector<CalCluster*> *ecalClusters, float weight ) {
    int nClusters = ecalClusters->size();
    Fill1DHisto("numEcalClusters_h", (float)nClusters, weight);
    for (int i=0; i < nClusters; i++)
    {
        CalCluster *cluster = ecalClusters->at(i);
        Fill1DHisto("ecalClusterEnergy_h", cluster->getEnergy(), weight);
        Fill1DHisto("ecalClusterNHits_h", cluster->getNHits(), weight);
        Fill1DHisto("ecalClusTime_h", cluster->getTime(), weight);

		CalHit* seed = (CalHit*)cluster->getSeed();

		int ix = seed -> getCrystalIndices()[0];
		int iy = seed -> getCrystalIndices()[1];

        Fill2DHisto("x_y_ecalCluster_hh", ix, iy, weight);
    }
}

void TriggerValidationAnaHistos::FillVTPData(VTPData *vtpData, float weight) {
	std::vector<VTPData::hpsCluster> vtpClusters =  vtpData->clusters;
    int nClusters = vtpClusters.size();
    Fill1DHisto("numVTPClusters_h", (float)nClusters, weight);
	for(int j = 0; j < nClusters; j++){
		VTPData::hpsCluster vtpCluster = vtpClusters.at(j);

        Fill1DHisto("vtpClusterEnergy_h", vtpCluster.E / 1000., weight);
        Fill1DHisto("vtpClusterNHits_h", vtpCluster.N, weight);
        Fill1DHisto("vtpClusterTime_h", vtpCluster.T * 4, weight);

		int ix = vtpCluster.X;
		int iy = vtpCluster.Y;
		if(ix <= 0) ix--;
		Fill2DHisto("x_y_vtpCluster_hh", ix, iy, weight);
	}
}
