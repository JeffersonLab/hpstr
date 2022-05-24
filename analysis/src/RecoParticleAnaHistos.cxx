#include "RecoParticleAnaHistos.h"
#include <math.h>
#include <iostream>

#define ROTATIONANGLEAROUNDY 0.0305 // rad
#define EBEAM 4.55 // GeV

void RecoParticleAnaHistos::Define1DHistos() {

}

void RecoParticleAnaHistos::FillFSPs(std::vector<Particle*> *fsps, float weight ){

    int n_fsps = fsps->size();
    Fill1DHisto("n_fsps_h",n_fsps, weight);

    for(int i = 0; i < n_fsps; i++){

    	Particle *fsp = fsps->at(i);

    	int charge = fsp->getCharge();
    	Fill1DHisto("charge_h", charge, weight);

    	std::vector<double> mom = fsp->getCorrMomentum();
    	double momMag = sqrt(mom[0] * mom[0] + mom[1] * mom[1] + mom[2] * mom[2]);

		double momX_before_beam_rotation = mom[0] * cos(ROTATIONANGLEAROUNDY) - mom[2] * sin(ROTATIONANGLEAROUNDY);
		double momZ_before_beam_rotation = mom[0] * sin(ROTATIONANGLEAROUNDY) + mom[2] * cos(ROTATIONANGLEAROUNDY);
		double momY_before_beam_rotation = mom[1];

		double dirY = momY_before_beam_rotation/momMag;


    	CalCluster cluster = fsp->getCluster();
    	std::vector<double> posCluster = cluster.getPosition();

    	Track track = fsp->getTrack();
    	std::vector<double> momTrack = track.getMomentum();
    	double momTrackMag = sqrt(momTrack[0] * momTrack[0] + momTrack[1] * momTrack[1] + momTrack[2] * momTrack[2]);

    	if(charge == -1){
    		if(momMag < 0.005 * EBEAM) continue;
    		if(momMag > 0.6 * EBEAM && fabs(dirY) < 0.005) continue;

    		Fill1DHisto("ecalClusterEnergy_ele_h", cluster.getEnergy(), weight);
    		Fill2DHisto("xyEcalClusters_ele_hh", posCluster[0], posCluster[1], weight);

    		Fill1DHisto("p_ele_h", momTrackMag, weight);
    	}

    	if(charge == 0){
    		if(fabs(dirY) < 0.004) continue;
    		if(momMag > 0.8 && fabs(dirY) > 0.005) continue;

    		Fill1DHisto("ecalClusterEnergy_photon_h", cluster.getEnergy(), weight);
    		Fill2DHisto("xyEcalClusters_photon_hh", posCluster[0], posCluster[1], weight);
    	}


    	if(charge == 1){
    		Fill1DHisto("ecalClusterEnergy_pos_h", cluster.getEnergy(), weight);
    		Fill2DHisto("xyEcalClusters_pos_hh", posCluster[0], posCluster[1], weight);

    		Fill1DHisto("p_pos_h", momTrackMag, weight);
    	}



    }

}
