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

// HPSTR
#include "Track.h"
#include "TrackerHit.h"
#include "Particle.h"
#include "Vertex.h"

/**
 * @brief brief description
 * more details
 */
class AnaHelpers {

    public :

        AnaHelpers();
 
        /**
         * @brief Rotate a vector from HPS to SVT Frame (i.e. for position/momentum rotation)
         * 
         * @param v 
         * @return TVector3 
         */
        TVector3 rotateToSvtFrame(TVector3 v);

        /**
         * @brief Rotate a vector from HPS to SVT Frame via TRotation
         * 
         * @param v 
         * @return TVector3 
         */
        TVector3 rotationToSvtFrame(const TVector3& v);

        /**
         * @brief Rotate a matrix from HPS to SVT Frame (i.e. for covariance matrix rotation)
         * 
         * @param cov 
         * @return TMatrixDSym 
         */
        TMatrixDSym rotateToSvtFrame(TMatrixDSym cov) {
            return cov.Similarity(*rotSvt_sym);
        }
        
        /**
         * @brief brief description
         * 
         * @param cov 
         * @return TMatrixDSym 
         */
        TMatrixDSym rotateToHpsFrame(TMatrixDSym cov) {
            return cov.SimilarityT(*rotSvt_sym);
        }

        /**
         * @brief Checks if a track has a 3d hit on innermost layer and second innermost layer
         * 
         * @param trk 
         * @param foundL1 
         * @param foundL2 
         */
        void InnermostLayerCheck(Track* trk, bool& foundL1, bool& foundL2);
        
        /**
         * @brief Get the Particles From Vtx object
         * 
         * @param vtx 
         * @param ele 
         * @param pos 
         * @return true 
         * @return false 
         */
        bool GetParticlesFromVtx(Vertex* vtx, Particle*& ele, Particle*& pos);

        /**
         * @brief Get the Particles From Vtx object
         * 
         * @param vtx 
         * @param ele1 
         * @param ele2
         * @return true 
         * @return false 
         */
        bool GetSameParticlesFromVtx(Vertex* vtx, Particle*& ele1, Particle*& ele2);
        
        /**
         * @brief brief description
         * 
         * @param ele_id 
         * @param pos_id 
         * @param ele_trk 
         * @param pos_trk 
         * @param trks 
         * @return true 
         * @return false 
         */
        bool MatchToGBLTracks(int ele_id, int pos_id, Track* & ele_trk, Track* & pos_trk, std::vector<Track*>& trks);
        
        static std::string getFileName(std::string filePath, bool withExtension);    
        
    private:
        
        const double SVT_ANGLE = -30.5e-3; //!< Angle between SVT system and HPS coordinates
        TRotation rotSvt; //!< description
        TMatrixDSym* rotSvt_sym; //!< description
        bool debug_{false}; //!< description
};


