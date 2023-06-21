#include "AnaHelpers.h"



std::string AnaHelpers::getFileName(std::string filePath, bool withExtension)
{
    char sep = '/';
    
    size_t i = filePath.rfind(sep, filePath.length());
    std::string baseFile="";

    if (i != std::string::npos) {
        baseFile = filePath.substr(i+1, filePath.length() - i);
    }
    
    if (withExtension)
        return baseFile;
    else {
        i = baseFile.find_last_of('.');
        return baseFile.substr(0,i);
    }
    
    return "";
}

void AnaHelpers::InnermostLayerCheck(Track* trk, bool& foundL1, bool& foundL2) {

    bool isKF = trk->isKalmanTrack();
    int innerCount = 0;
    bool hasL1 = false;
    bool hasL2 = false;
    bool hasL3 = false;
    for (int ihit=0; ihit<trk->getSvtHits().GetEntries();++ihit) {
        TrackerHit* hit3d = (TrackerHit*) trk->getSvtHits().At(ihit);
        if(isKF){
            if (hit3d->getLayer() == 0 ) {
                innerCount++;
            }
            if (hit3d->getLayer() == 1) {
                innerCount++;
            }
            if (hit3d->getLayer() == 2) {
                innerCount++;
                hasL2 = true;
            }
            if (hit3d->getLayer() == 3) {
                innerCount++;
                hasL3 = true;
            }
        }
        else{
            if (hit3d->getLayer() == 0 ) {
                innerCount++;
            }
            if (hit3d->getLayer() == 1) {
                innerCount++;
                hasL1 = true;
            }
        }
    }

    if(isKF){
        if (innerCount == 4) foundL1 = true;
        if (hasL2 && hasL3) foundL2 = true;
    }
    else{
        if (innerCount == 2) foundL1 = true;
        if (hasL1) foundL2 = true;
    }
}

bool AnaHelpers::MatchToGBLTracks(int ele_id, int pos_id, Track* & ele_trk, Track* & pos_trk, std::vector<Track*>& trks) {

    bool foundele = false;
    bool foundpos = false;

    for (auto trk : trks) {
        if (ele_id == trk->getID())  {
            ele_trk = trk;
            foundele = true;
        }
        if (pos_id == trk->getID()) {
            pos_trk = trk;
            foundpos = true;
        }
    }
    return foundele * foundpos;
}


//TODO clean bit up 
bool AnaHelpers::GetParticlesFromVtx(Vertex* vtx, Particle*& ele, Particle*& pos) {


    bool foundele = false;
    bool foundpos = false;

    for (int ipart = 0; ipart < vtx->getParticles().GetEntries(); ++ipart) {


        int pdg_id = ((Particle*)vtx->getParticles().At(ipart))->getPDG();
        if (debug_) std::cout<<"In Loop "<<pdg_id<< " "<< ipart<<std::endl;

        if (pdg_id == 11) {
            ele =  ((Particle*)vtx->getParticles().At(ipart));
            foundele=true;
            if (debug_) std::cout<<"found ele "<< (int)foundele<<std::endl;
        }
        else if (pdg_id == -11) {
            pos = (Particle*)vtx->getParticles().At(ipart);
            foundpos=true;
            if  (debug_) std::cout<<"found pos "<<(int)foundpos<<std::endl;

        }
    }

    if (!ele || !pos) {
        std::cout<<"Vertex formed without ele/pos. Skip."<<std::endl;
        return false;
    }
    if (debug_) std::cout<<"returning "<<(int) (foundele && foundpos) <<std::endl;
    return foundele && foundpos;
}

AnaHelpers::AnaHelpers() {
    rotSvt.RotateY(SVT_ANGLE);

    rotSvt_sym =  new TMatrixDSym(3);

    (*rotSvt_sym)(0,0) = cos(SVT_ANGLE);
    (*rotSvt_sym)(0,1) = 0.;
    (*rotSvt_sym)(0,2) = sin(SVT_ANGLE);

    (*rotSvt_sym)(1,0) = 0.;
    (*rotSvt_sym)(1,1) = 1.;
    (*rotSvt_sym)(1,2) = 0.;

    (*rotSvt_sym)(1,0) = -sin(SVT_ANGLE);
    (*rotSvt_sym)(1,1) = 0.;
    (*rotSvt_sym)(1,2) = cos(SVT_ANGLE);

    debug_ = false;

}

TVector3 AnaHelpers::rotateToSvtFrame(TVector3 v) {

    v.RotateY(SVT_ANGLE);
    return v;   
}

TVector3 AnaHelpers::rotationToSvtFrame(const TVector3&  v) {
    return rotSvt*v;
}
