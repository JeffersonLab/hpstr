#include "MCAnaHistos.h"
#include <math.h>

void MCAnaHistos::Define1DHistos() {

}

void MCAnaHistos::FillMCParticles(std::vector<MCParticle*> *mcParts, float weight ) {
    int nParts = mcParts->size();
    Fill1DHisto("numMCparts_h", (float)nParts, weight);
    int nMuons = 0;
    double minMuonE = -99.9;
    for (int i=0; i < nParts; i++) 
    {
        MCParticle *part = mcParts->at(i);
        int pdg = part->getPDG();
        int momPdg = part->getMomPDG();
	//	if ( momPdg == 622 )
	//  std::cout<<"Found particle with momPDG = 622, part = " << pdg << std::endl;
 
        double energy = part->getEnergy();
        double massMeV = 1000.0*part->getMass();
        double zPos = part->getVertexPosition().at(2);
        if(pdg == 622)
        {
            Fill1DHisto("mc622Mass_h", massMeV, weight);
            Fill1DHisto("mc622Z_h", zPos, weight);
        }

        if(pdg == 625)
        {
            Fill1DHisto("mc625Mass_h", massMeV, weight);
            Fill1DHisto("mc625Z_h", zPos, weight);
        }

        if(pdg == 624)
        {
            Fill1DHisto("mc624Mass_h", massMeV, weight);
            Fill1DHisto("mc624Z_h", zPos, weight);
        }


        if (fabs(pdg) == 13)
        {
            nMuons++;
            if(energy < minMuonE || minMuonE < 0.0)
            {
                minMuonE = energy;
            }
        }

	//	if (momPdg > 600)
	// std::cout<< i <<" : mom is "<<momPdg<<", mine is "<<pdg<<std::endl;

	if (pdg == 11 && momPdg == 625){
	  std::vector<double> p = part->getMomentum();
	  Fill1DHisto("truthRadElecE_h",energy,weight);
	  Fill1DHisto("truthRadEleczPos_h",zPos,weight);
	  Fill1DHisto("truthRadElecPt_h",sqrt(p[0]*p[0] + p[1]*p[1]),weight);
	  Fill1DHisto("truthRadElecPz_h",p[2],weight);
	}

	if (pdg == -11 && momPdg == 625){
	  std::vector<double> p = part->getMomentum();
	  Fill1DHisto("truthRadPosE_h",energy,weight);
	  Fill1DHisto("truthRadPoszPos_h",zPos,weight);
	  Fill1DHisto("truthRadPosPt_h",sqrt(p[0]*p[0] + p[1]*p[1]),weight);
	  Fill1DHisto("truthRadPosPz_h",p[2],weight);
	}
 

        Fill1DHisto("MCpartsEnergy_h", energy, weight);
        Fill1DHisto("MCpartsEnergyLow_h", energy*1000.0, weight);// Scaled to MeV
    }
    Fill1DHisto("numMuons_h", nMuons, weight);
    Fill1DHisto("minMuonE_h", minMuonE, weight);
    Fill1DHisto("minMuonEhigh_h", minMuonE, weight);
}

void MCAnaHistos::FillMCTrackerHits(std::vector<MCTrackerHit*> *mcTrkrHits, float weight ) {
    int nHits = mcTrkrHits->size();
    Fill1DHisto("numMCTrkrHit_h", nHits, weight);
    for (int i=0; i < nHits; i++) 
    {
        MCTrackerHit *hit = mcTrkrHits->at(i);
        int pdg = hit->getPDG();
        Fill1DHisto("mcTrkrHitEdep_h", hit->getEdep()*1000.0, weight); // Scaled to MeV
        Fill1DHisto("mcTrkrHitPdgId_h", (float)hit->getPDG(), weight);
    }
}

void MCAnaHistos::FillMCEcalHits(std::vector<MCEcalHit*> *mcEcalHits, float weight ) {
    int nHits = mcEcalHits->size();
    Fill1DHisto("numMCEcalHit_h", nHits, weight);
    for (int i=0; i < nHits; i++) 
    {
        MCEcalHit *hit = mcEcalHits->at(i);
        Fill1DHisto("mcEcalHitEnergy_h", hit->getEnergy()*1000.0, weight); // Scaled to MeV
    }
}
