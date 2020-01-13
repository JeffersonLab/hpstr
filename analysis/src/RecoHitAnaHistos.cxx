#include "RecoHitAnaHistos.h"
#include <math.h>

void RecoHitAnaHistos::Define1DHistos() {

}

void RecoHitAnaHistos::FillTrackerHits(std::vector<TrackerHit*> *trkrHits, float weight ) {
    int nHits = trkrHits->size();
    Fill1DHisto("numTrkrHits_h", (float)nHits, weight);
    for (int i=0; i < nHits; i++) 
    {
        TrackerHit *hit = trkrHits->at(i);
        Fill1DHisto("trkrHitEdep_h", hit->getCharge()*1000000.0, weight); // Scaled to MeV
    }
}

void RecoHitAnaHistos::FillTracks(std::vector<Track*> *tracks, float weight ) {
    int nTracks = tracks->size();
    Fill1DHisto("numTracks_h", (float)nTracks, weight);
}

void RecoHitAnaHistos::FillEcalHits(std::vector<CalHit*> *ecalHits, float weight ) {
    int nHits = ecalHits->size();
    Fill1DHisto("numEcalHits_h", (float)nHits, weight);
    for (int i=0; i < nHits; i++) 
    {
        CalHit *hit = ecalHits->at(i);
        Fill1DHisto("ecalHitEnergy_h", hit->getEnergy()*1000.0, weight); // Scaled to MeV
    }
}

void RecoHitAnaHistos::FillEcalClusters(std::vector<CalCluster*> *ecalClusters, float weight ) {
    int nClusters = ecalClusters->size();
    Fill1DHisto("numEcalClusters_h", (float)nClusters, weight);
    for (int i=0; i < nClusters; i++) 
    {
        CalCluster *cluster = ecalClusters->at(i);
        Fill1DHisto("ecalClusterEnergy_h", cluster->getEnergy()*1000.0, weight); // Scaled to MeV
    }
}
