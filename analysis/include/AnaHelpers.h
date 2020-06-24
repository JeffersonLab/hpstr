/** 
 * @file  AnaHelpers.h
 * @brief Helper class for hipster analysis
 * @author PF, SLAC
 */

#include <iostream>


#include "TMatrix.h"
#include "TVector3.h"
#include "TRotation.h"
#include "TMatrixDSym.h"
#include "TMatrixTSym.h"

//HPSTR
#include "Track.h"
#include "TrackerHit.h"
#include "Particle.h"
#include "Vertex.h"



class AnaHelpers {

public :

    AnaHelpers();
    
    /*
     * Rotate a vector from HPS to SVT Frame (i.e. for position/momentum rotation)
     */
    

    TVector3 rotateToSvtFrame(TVector3 v);

    /* 
     * Rotate a vector from HPS to SVT Frame via TRotation
     */
       
    TVector3 rotationToSvtFrame(const TVector3& v);
    
    /* 
     * Rotate a matrix from HPS to SVT Frame (i.e. for covariance matrix rotation)
     */
    
    TMatrixDSym  rotateToSvtFrame(TMatrixDSym cov) {
        return cov.Similarity(*rotSvt_sym);
    }

    TMatrixDSym rotateToHpsFrame(TMatrixDSym cov) {
        return cov.SimilarityT(*rotSvt_sym);
    }

    /** 
     * Checks if a track has a 3d hit on innermost layer and second innermost layer
     */
        
    void InnermostLayerCheck(Track* trk, bool& foundL1, bool& foundL2);
    
    bool GetParticlesFromVtx(Vertex* vtx, Particle*& ele, Particle*& pos);
    
    bool MatchToGBLTracks(int ele_id, int pos_id, Track* & ele_trk, Track* & pos_trk, std::vector<Track*>& trks);
    
    static std::string getFileName(std::string filePath, bool withExtension);    
    
private:
    
    //Angle between SVT system and HPS coordinates
    const double SVT_ANGLE = -30.5e-3;
    TRotation rotSvt;
    TMatrixDSym* rotSvt_sym;
    bool debug_{false};
};


