#ifndef _TRACKTOOLS_
#define _TRACKTOOLS_


#include <EVENT/LCCollection.h>
#include <EVENT/Track.h>
#include <EVENT/ReconstructedParticle.h>
#include <EVENT/Vertex.h>
#include <EVENT/TrackerHit.h>
#include <EVENT/TrackState.h>
#include <EVENT/TrackerRawData.h>
#include <EVENT/CalorimeterHit.h>
#include <EVENT/Cluster.h>
#include <IMPL/CalorimeterHitImpl.h>
#include <IMPL/LCGenericObjectImpl.h>
#include <IMPL/TrackerHitImpl.h>
#include <IMPL/ClusterImpl.h>
#include <UTIL/LCRelationNavigator.h>
#include <UTIL/BitField64.h>


#include <vector>
#include <cmath>
#include <array>
#include <iostream>
#include <fstream>

//-----------//
//   hpstr   //
//-----------//
#include "Collections.h"
#include "Processor.h"
#include "Particle.h"
#include "Track.h"
#include "Event.h"
#include "Vertex.h"
#include "TrackerHit.h"


//-----------//
//   ROOT    //
//-----------//
#include "TRefArray.h"
#include "TMatrixD.h"

namespace TrackTools {


    /**
     * @brief Normalize angle to [-π, π]
     * 
     * @param angle Input angle (radians)
     * @return double Normalized angle
     */
    float normalizeAngle(float angle);

    /**
     * @brief Extract track parameter covariance matrix from Track object
     * 
     * @param track Input track
     * @return TMatrixD 5x5 covariance matrix [d0, phi0, omega, z0, tanLambda]
     */
    TMatrixD getTrackCovariance(Track* track);

    /**
     * @brief Extract vertex position covariance from Vertex object
     * 
     * @param vtx Input vertex
     * @return TMatrixD 3x3 covariance matrix [x, y, z]
     */
    TMatrixD getVertexCovariance(Vertex* vtx);

    /**
     * @brief Calculate momentum covariance from track parameter covariance
     * 
     * Given track momentum components (px, py, pz) and track parameters,
     * propagate the track parameter covariance to momentum covariance.
     * 
     * @param px
     * @param py
     * @param pz
     * @param phi0
     * @param omega
     * @param tanLambda
     * @param covTrack 5x5 track parameter covariance matrix
     * @return TMatrixD 3x3 momentum covariance matrix
     */
    TMatrixD momentumCovarianceFromTrack(
        double px, double py, double pz,
        double phi0, double omega, double tanLambda,
        const TMatrixD& covTrack);

    /**
     * @brief Calculate momentum covariance from Track object
     * 
     * @param track
     * @return TMatrixD 3x3 momentum covariance matrix
     */
    TMatrixD momentumCovarianceFromTrack(Track* track);


    /**
     * @brief Calculate track parameters at target from position and momentum
     * 
     * Coordinate system:
     *   - x, z: transverse plane
     *   - y: beam direction
     * 
     * @param x_vtx
     * @param y_vtx
     * @param z_vtx
     * @param px
     * @param py
     * @param pz
     * @param Q
     * @param cB
     * @param targetPos y-coordinate of "new" pivot point (mm)
     * @return Track parameters [d0, phi0, omega, z0, tanLambda]
     */
    std::vector<float> calculateTrackParameters(
            float x_vtx, float y_vtx, float z_vtx,
            float px, float py, float pz, float Q,
            float cB, float targetPos);

    /**
     * @brief Analytical Jacobian: d[d0,phi0,omega,z0,tanLambda]/d[x,y,z,px,py,pz]
     * 
     * @param x_vtx
     * @param y_vtx
     * @param z_vtx
     * @param px
     * @param py
     * @param pz
     * @param Q
     * @param cB
     * @param targetPos y-coordinate of pivot point
     * @return TMatrixD 5x6 Jacobian matrix
     */
    TMatrixD trackParametersJacobian(
        float x_vtx, float y_vtx, float z_vtx,
        float px, float py, float pz, float Q,
        float cB, float targetPos);

    
    /**
    * @brief Calculate track parameter covariance from momentum, position
    * 
    * Given vertex position, momentum, and their covariances, propagate
    * to track parameter covariance using the Jacobian.
    * 
    * @param x_vtx, y_vtx, z_vtx
    * @param px, py, pz
    * @param covVertex
    * @param covMomentum
    * @param Q
    * @param cB
    * @param targetPos
    * @return TMatrixD 5x5 track parameter covariance matrix
    */
    TMatrixD calculateTrackCovariance(
        float x_vtx, float y_vtx, float z_vtx,
        float px, float py, float pz,
        const TMatrixD& covVertex,
        const TMatrixD& covMomentum,
        float Q, float cB, float targetPos);


        /**
        * @brief Build combined 6x6 covariance matrix [x,y,z,px,py,pz]
        * 
        * @param covVertex
        * @param covMomentum
        * @param covCross
        * @return TMatrixD 6x6 combined covariance
        */
        TMatrixD buildCombinedCovariance(
            const TMatrixD& covVertex,
            const TMatrixD& covMomentum,
            const TMatrixD* covCross = nullptr);

        TMatrixD numericalJacobian(
            float x_vtx, float y_vtx, float z_vtx,
            float px, float py, float pz,
            float Q, float cB, float targetPos);

}

#endif //TRACKTOOLS