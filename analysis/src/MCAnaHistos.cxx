#include "MCAnaHistos.h"
#include <math.h>

void MCAnaHistos::Define1DHistos() {

    // init MCParticle Histos
    histos1d["numMCparts_h"] = new TH1F("numMCparts_h", ";Number of Sim Particles;Events",
            60, -0.5, 59.5);
    histos1d["MCpartsEnergy_h"] = new TH1F("MCpartsEnergy_h", ";Energy of Sim Particle [GeV];Sim Particles / 10 MeV",
            450, 0, 4.5);
    histos1d["MCpartsEnergyLow_h"] = new TH1F("MCpartsEnergyLow_h", ";Energy of Sim Particle [MeV];Sim Particles / 1 MeV",
            200, 0, 200);
    histos1d["numMuons_h"] = new TH1F("numMuons_h", ";Number of Sim Muons;Events",
            10, -0.5, 9.5);
    histos1d["minMuonE_h"] = new TH1F("minMuonE_h", "Minimum MC Muon Energy;Energy [GeV];Sim Particles / 10 MeV",
            450, 0, 4.5);
    histos1d["minMuonEhigh_h"] = new TH1F("minMuonEhigh_h","Minimum MC Muon Energy;Energy [GeV];Sim Particles/100 keV",
            3000, 2.2, 2.5);

    // init MCTrackerHit Histos
    histos1d["numMCTrkrHit_h"] = new TH1F("numMCTrkrHit_h", ";Number of Sim Tracker Hits;Events",
            30, -0.5, 29.5);
    histos1d["mcTrkrHitEdep_h"] = new TH1F("mcTrkrHitEdep_h", ";Tracker Hit Energy [MeV];Sim Particles / 1 MeV",
            10, 0, 10);
    histos1d["mcTrkrHitPdgId_h"] = new TH1F("mcTrkrHitPdgId_h", ";PDG ID;Sim Particles",
            101, -50.5, 50.5);
    
    // init MCEcalHit Histos
    histos1d["numMCEcalHit_h"] = new TH1F("numMCEcalHit_h", ";Number of Sim Ecal Hits;Events",
            60, -0.5, 59.5);
    histos1d["mcEcalHitEnergy_h"] = new TH1F("mcEcalHitEnergy_h", ";Ecal Hit Energy [MeV];Sim Particles / 2 MeV",
            200, 0, 400);
    sumw2();
}

void MCAnaHistos::FillMCParticles(TClonesArray* mcParts, float weight ) {
    int nParts = mcParts->GetEntriesFast();
    histos1d["numMCparts_h"]->Fill((float)nParts, weight);
    int nMuons = 0;
    double minMuonE = -99.9;
    for (int i=0; i < nParts; i++) 
    {
        MCParticle *part = (MCParticle*)mcParts->At(i);
        int pdg = part->getPDG();
        double energy = part->getEnergy();
        if (fabs(pdg) == 13)
        {
            nMuons++;
            if(energy < minMuonE || minMuonE < 0.0)
            {
                minMuonE = energy;
            }
        }
        histos1d["MCpartsEnergy_h"]->Fill(energy, weight);
        histos1d["MCpartsEnergyLow_h"]->Fill(energy*1000.0, weight);// Scaled to MeV
    }
    histos1d["numMuons_h"]->Fill(nMuons, weight);
    histos1d["minMuonE_h"]->Fill(minMuonE, weight);
    histos1d["minMuonEhigh_h"]->Fill(minMuonE, weight);
}

void MCAnaHistos::FillMCTrackerHits(TClonesArray* mcTrkrHits, float weight ) {
    int nHits = mcTrkrHits->GetEntriesFast();
    histos1d["numMCTrkrHit_h"]->Fill(nHits, weight);
    for (int i=0; i < nHits; i++) 
    {
        MCTrackerHit *hit = (MCTrackerHit*)mcTrkrHits->At(i);
        int pdg = hit->getPDG();
        histos1d["mcTrkrHitEdep_h"]->Fill(hit->getEdep()*1000.0, weight); // Scaled to MeV
        histos1d["mcTrkrHitPdgId_h"]->Fill((float)hit->getPDG(), weight);
    }
}

void MCAnaHistos::FillMCEcalHits(TClonesArray* mcEcalHits, float weight ) {
    int nHits = mcEcalHits->GetEntriesFast();
    histos1d["numMCEcalHit_h"]->Fill(nHits, weight);
    for (int i=0; i < nHits; i++) 
    {
        MCEcalHit *hit = (MCEcalHit*)mcEcalHits->At(i);
        histos1d["mcEcalHitEnergy_h"]->Fill(hit->getEnergy()*1000.0, weight); // Scaled to MeV
    }
}
