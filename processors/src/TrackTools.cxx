#include "TrackTools.h"
#include <iostream>
#include <cmath>


float TrackTools::normalizeAngle(float angle) {
    const float PI = M_PI;
    while (angle > PI) angle -= 2.0 * PI;
    while (angle < -PI) angle += 2.0 * PI;
    return angle;
}


TMatrixD TrackTools::getTrackCovariance(Track* track) {
    TMatrixD cov(5, 5);
    std::vector<float> covArray = track->getCov();
    
    if (covArray.size() < 15) {
        std::cerr << "WARNING: Track covariance array size = " << covArray.size() 
                  << ", expected 15" << std::endl;
        return cov;
    }
    
    int idx = 0;
    for (int j = 0; j < 5; j++) {         
        for (int i = 0; i <= j; i++) {   
            cov(i, j) = covArray[idx];
            cov(j, i) = covArray[idx];  
            idx++;
        }
    }
    return cov;
}


TMatrixD TrackTools::getVertexCovariance(Vertex* vtx) {
    TMatrixD cov(3, 3);
    
    // Vertex covariance matrix
    std::vector<float> covArray = vtx->getCovariance();
    
    if (covArray.size() < 6) {
        std::cerr << "WARNING: Vertex covariance array size = " << covArray.size() 
                  << ", expected 6" << std::endl;
        return cov;
    }
    
    cov(0, 0) = covArray[0]; // Var(x)
    cov(0, 1) = covArray[1]; // Cov(x,y)
    cov(1, 0) = covArray[1];
    cov(1, 1) = covArray[2]; // Var(y)
    cov(0, 2) = covArray[3]; // Cov(x,z)
    cov(2, 0) = covArray[3];
    cov(1, 2) = covArray[4]; // Cov(y,z)
    cov(2, 1) = covArray[4];
    cov(2, 2) = covArray[5]; // Var(z)
    
    return cov;
}


TMatrixD TrackTools::momentumCovarianceFromTrack(
    double px, double py, double pz,
    double phi0, double omega, double tanLambda,
    const TMatrixD& covTrack) {
    
    // Calculate pT from momentum components
    double pT = std::sqrt(px*px + pz*pz);
    double pT2 = pT * pT;
    double pT3 = pT * pT2;
    
    // We need to infer bfield*MOM_PARAM from the relationship:
    // pt = |1/omega| * bfield * MOM_PARAM
    double bfield_times_mom = pT * std::fabs(omega);
    
    double sign_omega = (omega > 0) ? 1.0 : -1.0;
    
    double sin_phi0 = std::sin(phi0);
    double cos_phi0 = std::cos(phi0);
    
    // Derivatives
    double dpt_domega = -sign_omega / (omega * omega) * bfield_times_mom;
    
    // Jacobian matrix: d[px, py, pz]/d[d0, phi0, omega, z0, tanLambda]
    TMatrixD J(3, 5);
    
    // Row 0: px = pT * sin(phi0)
    J(0, 0) = 0.0;                    // d(px)/d(d0)
    J(0, 1) = pT * cos_phi0;          // d(px)/d(phi0)
    J(0, 2) = dpt_domega * sin_phi0;  // d(px)/d(omega)
    J(0, 3) = 0.0;                    // d(px)/d(z0)
    J(0, 4) = 0.0;                    // d(px)/d(tanLambda)
    
    // Row 1: py = pT * tanLambda
    J(1, 0) = 0.0;                      // d(py)/d(d0)
    J(1, 1) = 0.0;                      // d(py)/d(phi0)
    J(1, 2) = dpt_domega * tanLambda;   // d(py)/d(omega)
    J(1, 3) = 0.0;                      // d(py)/d(z0)
    J(1, 4) = pT;                       // d(py)/d(tanLambda)
    
    // Row 2: pz = pT * cos(phi0)
    J(2, 0) = 0.0;                    // d(pz)/d(d0)
    J(2, 1) = -pT * sin_phi0;         // d(pz)/d(phi0)
    J(2, 2) = dpt_domega * cos_phi0;  // d(pz)/d(omega)
    J(2, 3) = 0.0;                    // d(pz)/d(z0)
    J(2, 4) = 0.0;                    // d(pz)/d(tanLambda)
    
    // Propagate covariance: Cov_p = J * Cov_track * J^T
    TMatrixD J_T(TMatrixD::kTransposed, J);
    TMatrixD temp = J * covTrack;
    TMatrixD covMomentum = temp * J_T;
    
    return covMomentum;
}

TMatrixD TrackTools::momentumCovarianceFromTrack(Track* track) {
    TMatrixD covTrack = getTrackCovariance(track);
    
    return momentumCovarianceFromTrack(
        track->getMomentum()[0],
        track->getMomentum()[1],
        track->getMomentum()[2],
        track->getPhi(),
        track->getOmega(),
        track->getTanLambda(),
        covTrack);
}


std::vector<float> TrackTools::calculateTrackParameters(
    float x_vtx, float y_vtx, float z_vtx,
    float px, float py, float pz, float Q, float cB,
    float targetPos) {
    
    // Transverse momentum
    float pT = std::sqrt(px*px + pz*pz);
    float omega = -Q * cB / pT;
    
    // Basic track parameters
    float tan_lambda = py / pT;
    float phi0 = std::atan2(px, pz);
    
    // Transform to helix convention
    float tan_L_helix = -tan_lambda;
    float phi0_helix = -phi0;
    float alpha_over_kappa = -1.0 / omega;
    
    // Helix center in transverse (x-z) plane
    float cos_phi0_h = std::cos(phi0_helix);
    float sin_phi0_h = std::sin(phi0_helix);
    
    float x_center = x_vtx + alpha_over_kappa * cos_phi0_h;
    float z_center = z_vtx + alpha_over_kappa * sin_phi0_h;
    

    // Angle to pivot point
    float phi0_new_helix = std::atan2(targetPos - z_center, -x_center);
    float cos_phi_new = std::cos(phi0_new_helix);
    float sin_phi_new = std::sin(phi0_new_helix);
    
    // Impact parameter d0
    float d0_helix = (x_center * cos_phi_new + 
                       (z_center - targetPos) * sin_phi_new - 
                       alpha_over_kappa);
    
    // z0 calculation
    float phi_diff = phi0_new_helix - phi0_helix;
    float dz_helix = -y_vtx - alpha_over_kappa * phi_diff * tan_L_helix;
    
    // Transform back 
    std::vector<float> params;
    params.push_back(d0_helix);           // d0
    params.push_back(-phi0_new_helix);    // phi0
    params.push_back(omega);              // omega
    params.push_back(-dz_helix);          // z0
    params.push_back(tan_lambda);         // tan_lambda
    
    // Normalize phi0
    params[1] = normalizeAngle(params[1]);
    
    return params;
}


TMatrixD TrackTools::trackParametersJacobian(
    float x_vtx, float y_vtx, float z_vtx,
    float px, float py, float pz,
    float Q, float cB,
    float targetPos) {
    
    // ---- Intermediate quantities ----
    float pT2 = px*px + pz*pz;
    float pT = std::sqrt(pT2);
    float pT3 = pT * pT2;

    float omega = -Q * cB / pT;
    
    float tan_lambda = py / pT;
    float phi0 = std::atan2(px, pz);
    
    // Helix frame
    float tan_L_helix = -tan_lambda;
    float phi0_helix = -phi0;
    float alpha_over_kappa = -1.0 / omega;
    
    float cos_phi0_h = std::cos(phi0_helix);
    float sin_phi0_h = std::sin(phi0_helix);
    
    // Center position
    float x_center = x_vtx + alpha_over_kappa * cos_phi0_h;
    float z_center = z_vtx + alpha_over_kappa * sin_phi0_h;
    
    // To pivot
    double dx_c = -x_center;
    double dz_c = targetPos - z_center;
    float r_c2 = dx_c*dx_c + dz_c*dz_c;

    float phi0_new_helix = std::atan2(dz_c, dx_c);
    float cos_phi_new = std::cos(phi0_new_helix);
    float sin_phi_new = std::sin(phi0_new_helix);
    
    float phi_diff = phi0_new_helix - phi0_helix;
    
    // ---- Derivatives of basic quantities ----
    
    // d(pT)/d(px), d(pT)/d(pz)
    float dpT_dpx = px / pT;
    float dpT_dpz = pz / pT;
    
    // d(phi0)/d(px), d(phi0)/d(pz)
    // phi0 = atan2(px, pz)
    float dphi0_dpx = pz / pT2;
    float dphi0_dpz = -px / pT2;
    
    // d(phi0_helix)/d(px), d(phi0_helix)/d(pz)
    float dphi0h_dpx = -dphi0_dpx;
    float dphi0h_dpz = -dphi0_dpz;
    
    // d(tan_lambda)/d(px), d(tan_lambda)/d(py), d(tan_lambda)/d(pz)
    float dtanL_dpx = -py * px / pT3;
    float dtanL_dpy = 1.0 / pT;
    float dtanL_dpz = -py * pz / pT3;
    
    //d(omega)/d(pT)
    float domega_dpT = Q * cB / pT2;

    //d(alpha_over_kappa)/d(px)
    float dalpha_over_kappa_dpx = (1 / (omega*omega)) * domega_dpT * dpT_dpx;
    float dalpha_over_kappa_dpy = 0;
    float dalpha_over_kappa_dpz = (1 / (omega*omega)) * domega_dpT * dpT_dpz;


    
    // d(x_center)/d(x), d(x_center)/d(px), etc.
    float dxc_dx = 1.0;
    float dxc_dpx = -alpha_over_kappa * sin_phi0_h * dphi0h_dpx + cos_phi0_h * dalpha_over_kappa_dpx;
    float dxc_dpz = -alpha_over_kappa * sin_phi0_h * dphi0h_dpz + cos_phi0_h * dalpha_over_kappa_dpz;
    
    // d(z_center)/d(z), d(z_center)/d(px), etc.
    float dzc_dz = 1.0;
    float dzc_dpx = alpha_over_kappa * cos_phi0_h * dphi0h_dpx + sin_phi0_h * dalpha_over_kappa_dpx;
    float dzc_dpz = alpha_over_kappa * cos_phi0_h * dphi0h_dpz + sin_phi0_h * dalpha_over_kappa_dpz;
    
    // d(phi0_new_helix)/d(x_center), d(phi0_new_helix)/d(z_center)
    float dphi0new_dxc = dz_c / r_c2;
    float dphi0new_dzc = -dx_c / r_c2;
    
    // d(phi0_new_helix)/d(x), d(phi0_new_helix)/d(z), etc.
    float dphi0new_dx = dphi0new_dxc * (dxc_dx);
    float dphi0new_dz = dphi0new_dzc * (dzc_dz);
    float dphi0new_dpx = dphi0new_dxc * (dxc_dpx) + dphi0new_dzc * (dzc_dpx);
    float dphi0new_dpz = dphi0new_dxc * (dxc_dpz) + dphi0new_dzc * (dzc_dpz);
    
    // ---- Build Jacobian ----
    TMatrixD J(5, 6);
    
    // ========================================
    // Row 0: d0
    // ========================================
    // d0 = x_center * cos(phi0_new) + (z_center - target) * sin(phi0_new) - alpha_over_kappa
    

    double phi_term = -(x_center * sin_phi_new) + (z_center - targetPos) * cos_phi_new;

    double dd0_dxc = cos_phi_new + phi_term * dphi0new_dxc;
    double dd0_dzc = sin_phi_new + phi_term * dphi0new_dzc;
    
    J(0, 0) = dd0_dxc * dxc_dx;  // d(d0)/d(x)
    J(0, 1) = 0.0;                // d(d0)/d(y)
    J(0, 2) = dd0_dzc * dzc_dz;  // d(d0)/d(z)
    J(0, 3) = dd0_dxc * dxc_dpx + dd0_dzc * dzc_dpx - dalpha_over_kappa_dpx;  // d(d0)/d(px)
    J(0, 4) = 0.0;                // d(d0)/d(py)
    J(0, 5) = dd0_dxc * dxc_dpz + dd0_dzc * dzc_dpz - dalpha_over_kappa_dpz;  // d(d0)/d(pz)
    
    // ========================================
    // Row 1: phi0_new = -phi0_new_helix
    // ========================================
    
    J(1, 0) = -dphi0new_dx;   // d(phi0)/d(x)
    J(1, 1) = 0.0;            // d(phi0)/d(y)
    J(1, 2) = -dphi0new_dz;   // d(phi0)/d(z)
    J(1, 3) = -dphi0new_dpx;  // d(phi0)/d(px)
    J(1, 4) = 0.0;            // d(phi0)/d(py)
    J(1, 5) = -dphi0new_dpz;  // d(phi0)/d(pz)
    
    // ========================================
    // Row 2: omega (constant - given from original track)
    // ========================================
    
    J(2, 0) = 0.0;  // d(omega)/d(x)
    J(2, 1) = 0.0;  // d(omega)/d(y)
    J(2, 2) = 0.0;  // d(omega)/d(z)
    J(2, 3) = domega_dpT * dpT_dpx;  // d(omega)/d(px)
    J(2, 4) = 0.0;  // d(omega)/d(py)
    J(2, 5) = domega_dpT * dpT_dpz;  // d(omega)/d(pz)
    
    // ========================================
    // Row 3: z0 = -dz_helix = y + alpha * phi_diff * tan_lambda
    // ========================================
    
    // d(phi_diff)/d(phi0_new_helix) = 1
    // d(phi_diff)/d(phi0_helix) = -1
    
    J(3, 0) = alpha_over_kappa * tan_L_helix * dphi0new_dx;  // d(z0)/d(x)
    J(3, 1) = 1.0;                                            // d(z0)/d(y)
    J(3, 2) = alpha_over_kappa * tan_L_helix * dphi0new_dz;  // d(z0)/d(z)
    
    // d(z0)/d(px) = d(y + alpha*phi_diff*tan_L)/d(px)
    //             = alpha * (dphi_diff/dpx) * tan_L + alpha * phi_diff * (dtan_L/dpx) +
    //               phi_diff*tan_L*(dalpha_dpx)
    // where dphi_diff/dpx = dphi0new/dpx - dphi0h/dpx
    double dphidiff_dpx = dphi0new_dpx - dphi0h_dpx;
    double dphidiff_dpz = dphi0new_dpz - dphi0h_dpz;
    
    J(3, 3) = alpha_over_kappa * (dphidiff_dpx * tan_L_helix - phi_diff * dtanL_dpx) + phi_diff * tan_L_helix * dalpha_over_kappa_dpx;
    J(3, 4) = -alpha_over_kappa * phi_diff * dtanL_dpy;
    J(3, 5) = alpha_over_kappa * (dphidiff_dpz * tan_L_helix - phi_diff * dtanL_dpz) + phi_diff * tan_L_helix * dalpha_over_kappa_dpz;
    
    // ========================================
    // Row 4: tan_lambda (depends only on momentum)
    // ========================================
    
    J(4, 0) = 0.0;         // d(tanL)/d(x)
    J(4, 1) = 0.0;         // d(tanL)/d(y)
    J(4, 2) = 0.0;         // d(tanL)/d(z)
    J(4, 3) = dtanL_dpx;   // d(tanL)/d(px)
    J(4, 4) = dtanL_dpy;   // d(tanL)/d(py)
    J(4, 5) = dtanL_dpz;   // d(tanL)/d(pz)
    
    return J;
}



TMatrixD TrackTools::buildCombinedCovariance(
    const TMatrixD& covVertex,
    const TMatrixD& covMomentum,
    const TMatrixD* covCross) {
    
    TMatrixD cov(6, 6);
    
    // Upper-left 3x3: position covariance
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            cov(i, j) = covVertex(i, j);
        }
    }
    
    // Lower-right 3x3: momentum covariance
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            cov(i+3, j+3) = covMomentum(i, j);
        }
    }
    
    // Off-diagonal blocks: position-momentum correlation
    if (covCross != nullptr) {
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                cov(i, j+3) = (*covCross)(i, j);
                cov(j+3, i) = (*covCross)(i, j); 
            }
        }
    } else {
        // No correlation between position and momentum
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                cov(i, j+3) = 0.0;
                cov(j+3, i) = 0.0;
            }
        }
    }
    
    return cov;
}


TMatrixD TrackTools::calculateTrackCovariance(
    float x_vtx, float y_vtx, float z_vtx,
    float px, float py, float pz,
    const TMatrixD& covVertex,
    const TMatrixD& covMomentum,
    float Q, float cB, float targetPos) {
    
    // Get Jacobian: d[track params]/d[x,y,z,px,py,pz]
    TMatrixD J = trackParametersJacobian(
        x_vtx, y_vtx, z_vtx,
        px, py, pz, Q, cB, targetPos);
    
    // Build 6x6 combined covariance matrix
    TMatrixD cov6x6 = buildCombinedCovariance(covVertex, covMomentum);
    
    // Propagate covariance: Cov_track = J * Cov_6x6 * J^T
    TMatrixD J_T(TMatrixD::kTransposed, J);
    TMatrixD temp = J * cov6x6;
    TMatrixD covTrack = temp * J_T;
    
    return covTrack;
}


TMatrixD TrackTools::numericalJacobian(
    float x_vtx, float y_vtx, float z_vtx,
    float px, float py, float pz,
    float Q, float cB, float targetPos) {
    
    const double epsilon = 1e-4;
    TMatrixD J_num(5, 6);
    
    // Get nominal track parameters
    std::vector<float> params_nominal = TrackTools::calculateTrackParameters(
        x_vtx, y_vtx, z_vtx, px, py, pz, Q, cB, targetPos);
    
    // Vary each input parameter
    std::vector<double> inputs = {x_vtx, y_vtx, z_vtx, px, py, pz};
    
    for (int j = 0; j < 6; j++) {
        std::vector<double> inputs_plus = inputs;
        inputs_plus[j] += epsilon;
        
        std::vector<float> params_plus = TrackTools::calculateTrackParameters(
            inputs_plus[0], inputs_plus[1], inputs_plus[2],
            inputs_plus[3], inputs_plus[4], inputs_plus[5],
            Q, cB, targetPos);
        
        // Compute derivatives
        for (int i = 0; i < 5; i++) {
            J_num(i, j) = (params_plus[i] - params_nominal[i]) / epsilon;
            
            // Handle angle wrapping for phi0
            if (i == 1) {
                double diff = params_plus[i] - params_nominal[i];
                if (diff > M_PI) diff -= 2*M_PI;
                if (diff < -M_PI) diff += 2*M_PI;
                J_num(i, j) = diff / epsilon;
            }
        }
    }
    
    return J_num;
}




