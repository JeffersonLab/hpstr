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
        double energy = part->getEnergy();
        double massMeV = 1000.0*part->getMass();
        double zPos = part->getVertexPosition().at(2);
        if(pdg == 622)
        {
            Fill1DHisto("mc622Mass_h", massMeV, weight);
            Fill1DHisto("mc622Z_h", zPos, weight);
        }
        if (fabs(pdg) == 13)
        {
            nMuons++;
            if(energy < minMuonE || minMuonE < 0.0)
            {
                minMuonE = energy;
            }
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

void MCAnaHistos::FillMCParticleHistos(MCParticle* mcpart, std::string label, double weight){
  double px=mcpart->getMomentum().at(0);
  double py=mcpart->getMomentum().at(1);
  double pz=mcpart->getMomentum().at(2);

  double p=sqrt(px*px+py*py+pz*pz);

  Fill1DHisto(label+"_px_h",px,weight);
  Fill1DHisto(label+"_py_h",py,weight);
  Fill1DHisto(label+"_pz_h",pz,weight);
  Fill1DHisto(label+"_p_h",p,weight);

  double thetaX=px/p;  
  double thetaY=py/p;
  Fill1DHisto(label+"_thetax_h",thetaX,weight);
  Fill1DHisto(label+"_thetay_h",thetaY,weight);

  return;
}

void MCAnaHistos::FillMCPairHistos(MCParticle* ele,MCParticle* pos,std::string label, double weight){
  double px1=ele->getMomentum().at(0);
  double py1=ele->getMomentum().at(1);
  double pz1=ele->getMomentum().at(2);
  double px2=pos->getMomentum().at(0);
  double py2=pos->getMomentum().at(1);
  double pz2=pos->getMomentum().at(2);

  double E1=sqrt(px1*px1+py1*py1+pz1*pz1);
  double E2=sqrt(px2*px2+py2*py2+pz2*pz2);

  double pxTot=px1+px2;
  double pyTot=py1+py2;
  double pzTot=pz1+pz2;
  double pTotSq=pxTot*pxTot+pyTot*pyTot+pzTot*pzTot;
  double Etot=E1+E2;

  double mass=sqrt(Etot*Etot-pTotSq);

  Fill1DHisto(label+"_pxV0_h",pxTot,weight);
  Fill1DHisto(label+"_pyV0_h",pyTot,weight);
  Fill1DHisto(label+"_pzV0_h",pzTot,weight);
  Fill1DHisto(label+"_pV0_h",sqrt(pTotSq),weight);
  Fill1DHisto(label+"_mass_h",mass,weight);
  
  double thetaX=pxTot/sqrt(pTotSq);  
  double thetaY=pyTot/sqrt(pTotSq);
  Fill1DHisto(label+"_thetaxV0_h",thetaX,weight);
  Fill1DHisto(label+"_thetayV0_h",thetaY,weight);

}
