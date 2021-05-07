#include "EcalTimingAnaHistos.h"
#include <math.h>

void EcalTimingAnaHistos::Define1DHistos() {

}

void EcalTimingAnaHistos::FillTSData(TSData* tsData, float weight ){
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


void EcalTimingAnaHistos::FillEcalClusters(std::vector<CalCluster*> *ecalClusters, float weight ) {
    int nClusters = ecalClusters->size();
    Fill1DHisto("numEcalClusters_h", (float)nClusters, weight);
    for (int i=0; i < nClusters; i++)
    {
        CalCluster *cluster = ecalClusters->at(i);
        Fill1DHisto("ecalClusterEnergy_h", cluster->getEnergy()*1000.0, weight); // Scaled to MeV
        Fill1DHisto("ecalClusterTime_h", cluster->getTime(), weight); // ns
        Fill2DHisto("ecalClusterTimeVsEnergy_hh", cluster->getEnergy()*1000.0, cluster->getTime(), weight);
        Fill2DHisto("ecalClusterXVsEnergy_hh", cluster->getEnergy()*1000.0, cluster->getPosition()[0], weight);

        Fill1DHisto("ecalClusterX_h", cluster->getPosition()[0], weight); // mm
        Fill1DHisto("ecalClusterY_h", cluster->getPosition()[1], weight); // mm
        Fill2DHisto("ecalClusterXY_hh", cluster->getPosition()[0], cluster->getPosition()[1], weight);

        if(cluster->getPosition()[0] < 0) Fill2DHisto("ecalClusterTimeVsEnergyNegX_hh", cluster->getEnergy()*1000.0, cluster->getTime(), weight);
        else Fill2DHisto("ecalClusterTimeVsEnergyPosX_hh", cluster->getEnergy()*1000.0, cluster->getTime(), weight);

    }
}

void EcalTimingAnaHistos::FillTargetConstrainedV0s(std::vector<Vertex*> *vtxs, float weight ){
	std::shared_ptr<AnaHelpers> _ah =  std::make_shared<AnaHelpers>();

	int nVtxs = vtxs->size();
    Fill1DHisto("numTargetConstrainedVertices_h", (float)nVtxs, weight);
    if(nVtxs == 1){
        Vertex* vtx = vtxs->at(0);
        Particle* ele = nullptr;
        Particle* pos = nullptr;

        bool foundParts = _ah->GetParticlesFromVtx(vtx,ele,pos);
        if (!foundParts) {
            std::cout<<"VertexAnaProcessor::WARNING::Found vtx without ele/pos. Skip."<<std::endl;
        }

        CalCluster eleClus = ele->getCluster();
        CalCluster posClus = pos->getCluster();

        //std::cout << eleClus.getPosition()[2]<<std::endl;
        //std::cout << posClus.getPosition()[2]<<std::endl;

        Fill1DHisto("eleClusterEnergyTargetConstrainedVertices_h", eleClus.getEnergy()*1000.0, weight); // Scaled to MeV
        Fill1DHisto("eleClusterTimeTargetConstrainedVertices_h", eleClus.getTime(), weight); // ns
        Fill2DHisto("eleClusterTimeVsEnergyTargetConstrainedVertices_hh", eleClus.getEnergy()*1000.0, eleClus.getTime(), weight);

        Fill1DHisto("posClusterEnergyTargetConstrainedVertices_h", posClus.getEnergy()*1000.0, weight); // Scaled to MeV
        Fill1DHisto("posClusterTimeTargetConstrainedVertices_h", posClus.getTime(), weight); // ns
        Fill2DHisto("posClusterTimeVsEnergyTargetConstrainedVertices_hh", posClus.getEnergy()*1000.0, eleClus.getTime(), weight);

        Fill1DHisto("eleClusterXTargetConstrainedVertices_h", eleClus.getPosition()[0], weight);
        Fill1DHisto("posClusterXTargetConstrainedVertices_h", posClus.getPosition()[0], weight);

        Fill1DHisto("eleClusterYTargetConstrainedVertices_h", eleClus.getPosition()[1], weight);
        Fill1DHisto("posClusterYTargetConstrainedVertices_h", posClus.getPosition()[1], weight);

        Fill1DHisto("clusterTimeDifferenceTargetConstrainedVertices_h", eleClus.getTime() - posClus.getTime(), weight);

    }
}
