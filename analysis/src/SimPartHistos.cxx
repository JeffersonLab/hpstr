#include "SimPartHistos.h"
#include <math.h>

void SimPartHistos::FillAcceptance(std::vector<MCParticle*> *MCParticles_, std::vector<Track*> *RecoTracks_, std::vector<TrackerHit*> *RecoTrackerClusters_, std::vector<CalCluster*> *RecoEcalClusters_, float weight) {
    int nParts = MCParticles_->size();
    Fill1DHisto("numMCparts_h", (float)nParts, weight);

    for (int i=0; i<nParts; i++) {
        MCParticle *part = MCParticles_->at(i);
        int pdg = part->getPDG();
        int gen = part->getGenStatus();
        std::vector<double> momentum_V = part->getMomentum();
        double px = momentum_V.at(0);
        double py = momentum_V.at(1);
        double pz = momentum_V.at(2);

        if (gen != 1)
            continue;

        Fill1DHisto("particle_pdgid_h", pdg, weight);
        if (pdg == 11) {
            Fill2DHisto("ele_pxpz_pypz_hh", px/pz, py/pz, weight);



        }

    }
}




