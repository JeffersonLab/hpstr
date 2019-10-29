#include "RecoHitAnaHistos.h"
#include <math.h>

void RecoHitAnaHistos::Define1DHistos() {

    // init TrackerHit Histos
    histos1d["numTrkrHits_h"] = new TH1F("numTrkrHits_h", ";Number of Reco Tracker Hits;Events",
            30, -0.5, 29.5);
    histos1d["trkrHitEdep_h"] = new TH1F("trkrHitEdep_h", 
            ";Tracker Hit Energy [arb. units];Tracker Hits / 1 arb. unit",
            100, 0, 100);

    // init Track Histos
    histos1d["numTracks_h"] = new TH1F("numTracks_h", ";Number of GBL Tracks;Events",
            10, -0.5, 9.5);

    // init Ecal Hit Histos
    histos1d["numEcalHits_h"] = new TH1F("numEcalHits_h", ";Number of Reco Ecal Hits;Events",
            10, -0.5, 9.5);
    histos1d["ecalHitEnergy_h"] = new TH1F("ecalHitEnergy_h",
            ";Reco Ecal Hit Energy [MeV];Ecal Hits / 2 MeV",
            200, 0, 400);

    // init Ecal Cluster Histos
    histos1d["numEcalClusters_h"] = new TH1F("numEcalClusters_h", ";Number of Reco Ecal Clusters;Events",
            10, -0.5, 9.5);
    histos1d["ecalClusterEnergy_h"] = new TH1F("ecalClusterEnergy_h",";Reco Ecal Cluster Energy [MeV];Clusters / 2 MeV",
            200, 0, 400);
    sumw2();
}

void RecoHitAnaHistos::FillTrackerHits(std::vector<TrackerHit*> *trkrHits, float weight ) {
    int nHits = trkrHits->size();
    histos1d["numTrkrHits_h"]->Fill((float)nHits, weight);
    for (int i=0; i < nHits; i++) 
    {
        TrackerHit *hit = trkrHits->at(i);
        histos1d["trkrHitEdep_h"]->Fill(hit->getCharge()*1000000.0, weight); // Scaled to MeV
    }
}

void RecoHitAnaHistos::FillTracks(std::vector<Track*> *tracks, float weight ) {
    int nTracks = tracks->size();
    histos1d["numTracks_h"]->Fill((float)nTracks, weight);
}

void RecoHitAnaHistos::FillEcalHits(std::vector<CalHit*> *ecalHits, float weight ) {
    int nHits = ecalHits->size();
    histos1d["numEcalHits_h"]->Fill((float)nHits, weight);
    for (int i=0; i < nHits; i++) 
    {
        CalHit *hit = ecalHits->at(i);
        histos1d["ecalHitEnergy_h"]->Fill(hit->getEnergy()*1000.0, weight); // Scaled to MeV
    }
}

void RecoHitAnaHistos::FillEcalClusters(std::vector<CalCluster*> *ecalClusters, float weight ) {
    int nClusters = ecalClusters->size();
    histos1d["numEcalClusters_h"]->Fill((float)nClusters, weight);
    for (int i=0; i < nClusters; i++) 
    {
        CalCluster *cluster = ecalClusters->at(i);
        histos1d["ecalClusterEnergy_h"]->Fill(cluster->getEnergy()*1000.0, weight); // Scaled to MeV
    }
}
