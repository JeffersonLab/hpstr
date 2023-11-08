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

    Fill1DHisto("sim_pdgid_h", pdg, weight);
    Fill1DHisto("sim_px_h", px, weight);
    Fill1DHisto("sim_py_h", py, weight);
    Fill1DHisto("sim_pz_h", pz, weight);
    Fill1DHisto("sim_p_h", p, weight);
    Fill1DHisto("sim_pxpz_h", px/pz, weight);
    Fill1DHisto("sim_pypz_h", py/pz, weight);
    Fill1DHisto("sim_energy_h", energy, weight);
    Fill2DHisto("sim_pxpz_pypz_hh", px/pz, py/pz, weight);
    Fill2DHisto("sim_pxpz_p_hh", px/pz, p, weight);
    Fill2DHisto("sim_pxpz_energy_hh", px/pz, energy, weight);
    tuples->addToVector("particle_pdgid", pdg);
    tuples->addToVector("sim_px", px);
    tuples->addToVector("sim_py", py);
    tuples->addToVector("sim_pz", pz);
    tuples->addToVector("sim_p", p);
    tuples->addToVector("sim_energy", energy);
    tuples->addToVector("sim_pxpz", px/pz);
    tuples->addToVector("sim_pypz", py/pz);
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
    double d0 = track->getD0();
    double z0 = track->getZ0();
    double omega = track->getOmega();

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

    std::vector<int> hit_layers;
    for (int ihit = 0; ihit<track->getSvtHits().GetEntries(); ++ihit) {
        TrackerHit* hit = (TrackerHit*) track->getSvtHits().At(ihit);
        int layer = hit->getLayer();
        hit_layers.push_back(layer);
    }

    Fill1DHisto("track_n_hits_h", n_hits, weight);
    Fill1DHisto("track_px_h", px, weight);
    Fill1DHisto("track_py_h", py, weight);
    Fill1DHisto("track_pz_h", pz, weight);
    Fill1DHisto("track_p_h", p, weight);
    Fill1DHisto("track_phi0", phi0, weight);
    Fill1DHisto("track_tanlambda", tan_lambda, weight);
    Fill1DHisto("track_d0", d0, weight);
    Fill1DHisto("track_z0", z0, weight);
    Fill1DHisto("track_omega", omega, weight);
    Fill1DHisto("track_ecal_x", track_ecal_x, weight);
    Fill1DHisto("track_ecal_y", track_ecal_y, weight);
    Fill2DHisto("track_phi0_p_hh", phi0, p, weight);
    Fill2DHisto("track_phi0_tanlambda_hh", phi0, tan_lambda, weight);
    Fill2DHisto("track_z0_tanlambda_hh", z0, tan_lambda, weight);
    //Fill2DHisto("track_x_y_hh", track_x, track_y, weight);
    Fill2DHisto("track_ecal_x_y_hh", track_ecal_x, track_ecal_y, weight);

    tuples->addToVector("track_n_hits", n_hits);
    tuples->addToVector("track_px", px);
    tuples->addToVector("track_py", py);
    tuples->addToVector("track_pz", pz);
    tuples->addToVector("track_p", p);
    tuples->addToVector("track_phi0", phi0);
    tuples->addToVector("track_tanlambda", tan_lambda);
    tuples->addToVector("track_d0", d0);
    tuples->addToVector("track_z0", z0);
    tuples->addToVector("track_omega", omega);
    //tuples->addToVector("track_x", track_x);
    //tuples->addToVector("track_y", track_y);
    tuples->addToVector("track_ecal_x", track_ecal_x);
    tuples->addToVector("track_ecal_y", track_ecal_y);

    for (int i=0; i<hit_layers.size(); i++){
        Fill1DHisto("track_hit_layer_h", hit_layers[i], weight);
        tuples->addToVector("track_hit_layrer", hit_layers[i]);
    }
}

void SimPartHistos::FillRecoEcalCuster(CalCluster* ecal_cluster, FlatTupleMaker* tuples, float weight){
    std::vector<double> position_V = ecal_cluster->getPosition();
    double cluster_x = position_V.at(0);
    double cluster_y = position_V.at(1);
    int n_hits = ecal_cluster->getNHits();
    double energy = ecal_cluster->getEnergy();

    Fill1DHisto("ecal_n_hits_h", n_hits, weight);
    Fill1DHisto("ecal_energy_h", energy, weight);
    Fill1DHisto("ecal_x", cluster_x, weight);
    Fill1DHisto("ecal_y", cluster_y, weight);
    Fill2DHisto("ecal_x_y_hh", cluster_x, cluster_y, weight);
    tuples->addToVector("ecal_n_hits", n_hits);
    tuples->addToVector("ecal_energy", energy);
    tuples->addToVector("ecal_x", cluster_x);
    tuples->addToVector("ecal_y", cluster_y);
}




