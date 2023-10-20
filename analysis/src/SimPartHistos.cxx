#include "SimPartHistos.h"
#include <math.h>

void SimPartHistos::FillMCParticle(MCParticle* part, FlatTupleMaker* tuples, float weight){
   int pdg = part->getPDG();
    std::vector<double> momentum_V = part->getMomentum();
    double px = momentum_V.at(0);
    double py = momentum_V.at(1);
    double pz = momentum_V.at(2);
    double p = sqrt(px*px + py*py + pz*pz);
    double energy = part->getEnergy();

    Fill1DHisto("particle_pdgid_h", pdg, weight);
    tuples->addToVector("particle_pdgid", pdg);
    if (pdg == 11) {
        Fill1DHisto("ele_px_h", px, weight);
        Fill1DHisto("ele_py_h", py, weight);
        Fill1DHisto("ele_pz_h", pz, weight);
        Fill1DHisto("ele_p_h", p, weight);
        Fill1DHisto("ele_energy_h", energy, weight);
        Fill2DHisto("ele_pxpz_pypz_hh", px/pz, py/pz, weight);
        tuples->addToVector("ele_px", px);
        tuples->addToVector("ele_py", py);
        tuples->addToVector("ele_pz", pz);
        tuples->addToVector("ele_p", p);
        tuples->addToVector("ele_energy", energy);
        tuples->addToVector("ele_pxpy", px/pz);
        tuples->addToVector("ele_pypz", py/pz);
    }
}

void SimPartHistos::FillRecoTrack(Track* track, FlatTupleMaker* tuples, float weight){
    bool isTop = (track->getTanLambda() > 0.0);
    bool isPos = (track->getOmega() < 0.0);
    bool isKF = track->isKalmanTrack();
    int trkType = (int)isTop*2 + (int)isPos;
    //int n2dhits_onTrack = !isKF ? track->getTrackerHitCount() * 2 : track->getTrackerHitCount();
    int n_hits = track->getTrackerHitCount();
    double phi0 = track->getPhi();
    double tan_lambda = track->getTanLambda();

    std::vector<double> position_V = track->getPosition();
    double track_x = position_V.at(0);
    double track_y = position_V.at(1);
    std::vector<double> position_ecal_V = track->getPositionAtEcal();
    double track_ecal_x = position_ecal_V.at(0);
    double track_ecal_y = position_ecal_V.at(1);
    
    std::vector<double> momentum_V = track->getMomentum();
    double px = momentum_V.at(0);
    double py = momentum_V.at(1);
    double pz = momentum_V.at(2);
    double p = track->getP();

    Fill1DHisto("track_n_hits_h", n_hits, weight);
    Fill1DHisto("track_px_h", px, weight);
    Fill1DHisto("track_py_h", py, weight);
    Fill1DHisto("track_pz_h", pz, weight);
    Fill1DHisto("track_p_h", p, weight);
    Fill2DHisto("track_phi0_tanlambda_hh", phi0, tan_lambda, weight);
    Fill2DHisto("track_x_y_hh", track_x, track_y, weight);
    Fill2DHisto("track_ecal_x_y_hh", track_ecal_x, track_ecal_y, weight);

    tuples->addToVector("track_n_hits", n_hits);
    tuples->addToVector("track_px", px);
    tuples->addToVector("track_py", py);
    tuples->addToVector("track_pz", pz);
    tuples->addToVector("track_p", p);
    tuples->addToVector("track_phi0", phi0);
    tuples->addToVector("track_tanlambda", tan_lambda);
    tuples->addToVector("track_x", track_x);
    tuples->addToVector("track_y", track_y);
    tuples->addToVector("track_ecal_x", track_ecal_x);
    tuples->addToVector("track_ecal_y", track_ecal_y);
}

void SimPartHistos::FillRecoEcalCuster(CalCluster* ecal_cluster, FlatTupleMaker* tuples, float weight){
    std::vector<double> position_V = ecal_cluster->getPosition();
    double cluster_x = position_V.at(0);
    double cluster_y = position_V.at(1);
    int n_hits = ecal_cluster->getNHits();
    double energy = ecal_cluster->getEnergy();

    Fill1DHisto("ecal_n_hits_h", n_hits, weight);
    Fill1DHisto("ecal_energy_h", energy, weight);
    Fill2DHisto("ecal_x_y_hh", cluster_x, cluster_y, weight);
    tuples->addToVector("ecal_n_hits", n_hits);
    tuples->addToVector("ecal_energy", energy);
    tuples->addToVector("ecal_x", cluster_x);
    tuples->addToVector("ecal_y", cluster_y);
}





