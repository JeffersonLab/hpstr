#include "FEEDataAnaHistos.h"
#include <math.h>

void FEEDataAnaHistos::Define1DHistos() {

}

void FEEDataAnaHistos::FillEcalClusters(std::vector<CalCluster*> *ecalClusters, float weight ) {
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

void FEEDataAnaHistos::FillVTPData(VTPData* vtpData, float weight) {
	std::vector<VTPData::hpsCluster> vtpClusters = vtpData->clusters;
    int nClusters = vtpClusters.size();
    Fill1DHisto("numVTPClusters_h", (float)nClusters, weight);
    for (int i=0; i < nClusters; i++)
    {
    	VTPData::hpsCluster cluster = vtpClusters.at(i);
        Fill1DHisto("VTPClusterEnergy_h", cluster.E/1000., weight);
        Fill1DHisto("VTPClusterTime_h", cluster.T * 4, weight); // ns
        Fill2DHisto("VTPClusterTimeVsEnergy_hh", cluster.E, cluster.T * 4, weight);
        Fill2DHisto("VTPClusterXYIndex_hh", cluster.X, cluster.Y, weight);
    }

}

void FEEDataAnaHistos::FillTracks(std::vector<Track*> *trks, float weight){
    int nTracks = trks->size();
    Fill1DHisto("numTracks_h", (float)nTracks, weight);
    for (int i=0; i < nTracks; i++){
    	Track* trk = trks->at(i);

        Fill1DHisto("chi2ndf_tracks_h", trk->getChi2Ndf(), weight);

    	int charge = trk->getCharge();
        Fill1DHisto("chargeTracks_h", charge, weight);

        std::vector<double> mom = trk->getMomentum();
        double p = sqrt(pow(mom[0], 2) + pow(mom[1], 2) + pow(mom[2], 2));

        std::vector<double> positionAtEcal = trk->getPositionAtEcal();
        Fill2DHisto("xy_positionAtEcal_tracks_hh", positionAtEcal[0], positionAtEcal[1], weight);

        if(charge == 1) {
        	Fill1DHisto("p_positive_tracks_h", p, weight);
        	Fill2DHisto("xy_positionAtEcal_positive_tracks_hh", positionAtEcal[0], positionAtEcal[1], weight);
        }
        else if(charge == -1) {
        	Fill1DHisto("p_negative_tracks_h", p, weight);
        	Fill2DHisto("xy_positionAtEcal_negative_tracks_hh", positionAtEcal[0], positionAtEcal[1], weight);
        }
    }
}
