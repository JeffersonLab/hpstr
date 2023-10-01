#include "SimPartHistos.h"
#include <math.h>

void SimPartHistos::FillAcceptance(std::vector<MCParticle*> *MCParticles_, std::vector<Track*> *RecoTracks_, std::vector<TrackerHit*> *RecoTrackerClusters_, std::vector<CalCluster*> *RecoEcalClusters_, float weight) {
    int nParts = MCParticles_->size();
    Fill1DHisto("numMCparts_h", (float)nParts, weight);

    int nTracks = RecoTracks_->size();
    int nEcal_clusters = RecoEcalClusters_->size();

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
            if (nTracks == 0){
                Fill2DHisto("ele_pxpz_pypz_noTrack_hh", px/pz, py/pz, weight);
                if (nEcal_clusters != 0)
                    Fill2DHisto("ele_pxpz_pypz_noTrack_ECal_hh", px/pz, py/pz, weight);
            }
            if (nEcal_clusters == 0){
                Fill2DHisto("ele_pxpz_pypz_noEcal_hh", px/pz, py/pz, weight);
                if (nTracks != 0)
                    Fill2DHisto("ele_pxpz_pypz_Track_noECal_hh", px/pz, py/pz, weight);
            }
        }
    }

    for (int i=0; i<nTracks; i++) {
        Track* track = RecoTracks_->at(i);
        bool isTop = (track->getTanLambda() > 0.0);
        bool isPos = (track->getOmega() < 0.0);
        bool isKF = track->isKalmanTrack();
        int trkType = (int)isTop*2 + (int)isPos;
        //int n2dhits_onTrack = !isKF ? track->getTrackerHitCount() * 2 : track->getTrackerHitCount();
        int n_hits = track->getTrackerHitCount();
        double phi0 = track->getPhi();
        double tan_lambda = track->getTanLambda();

        if (n_hits >= 7):
            Fill2DHisto("track_phi0_tanlambda_minhits7_hh", phi0, tan_lambda, weight);
        elif (n_hits >= 8):
            Fill2DHisto("track_phi0_tanlambda_minhits8_hh", phi0, tan_lambda, weight);
        elif (n_hits >= 9):
            Fill2DHisto("track_phi0_tanlambda_minhits9_hh", phi0, tan_lambda, weight);
        elif (n_hits >= 10):
            Fill2DHisto("track_phi0_tanlambda_minhits10_hh", phi0, tan_lambda, weight);
        elif (n_hits >= 11):
            Fill2DHisto("track_phi0_tanlambda_minhits11_hh", phi0, tan_lambda, weight);
        elif (n_hits >= 12):
            Fill2DHisto("track_phi0_tanlambda_minhits12_hh", phi0, tan_lambda, weight);
        elif (n_hits >= 13):
            Fill2DHisto("track_phi0_tanlambda_minhits13_hh", phi0, tan_lambda, weight);
        elif (n_hits >= 14):
            Fill2DHisto("track_phi0_tanlambda_minhits14_hh", phi0, tan_lambda, weight);

    }

    for (int i=0; i<nEcal_clusters; i++) {
        CalCluster *ecal_cluster = RecoEcalClusters_->at(i);
        std::vector<double> position_V = ecal_cluster->getPosition();
        double cluster_x = position_V.at(0);
        double cluster_y = position_V.at(1);

        Fill2DHisto("ecal_x_y_hh", cluster_x, cluster_y, weight);
        if (nTracks == 0)
            Fill2DHisto("ecal_x_y_noTrack_hh", cluster_x, cluster_y, weight);
    }

}




