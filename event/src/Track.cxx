/**
 * @file Track.h
 * @brief Class used to encapsulate track information.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "Track.h"

ClassImp(Track)

Track::Track()
    : TObject() { 
    }

//TODO Fix particle->track->tracker hits relation. If Track is object of Particle then tracker_hits in on the stack and delete make it crash?
Track::~Track() {
    Clear();
    //delete tracker_hits_;
}


//TODO:: Why is tracker_hist->Delete() crashes.
void Track::Clear(Option_t* /* option */) {
    TObject::Clear();
    //if (tracker_hits_) 
    //   tracker_hits_->Delete();
    memset(isolation_, 0, sizeof(isolation_)); 
    n_hits_ = 0; 
}

void Track::setTrackParameters(double d0, double phi0, double omega,
        double tan_lambda, double z0) {
    d0_         = d0;
    phi0_       = phi0;
    omega_      = omega;
    tan_lambda_ = tan_lambda;
    z0_         = z0;
    if (omega_ < 0)
        charge_   = -1;
    else
        charge_   =  1;

}

std::vector<double> Track::getTrackParameters() { return { d0_, phi0_, omega_, tan_lambda_, z0_ }; }

void Track::setPositionAtEcal(const double* position) { 
    x_at_ecal_ = position[0]; 
    y_at_ecal_ = position[1];
    z_at_ecal_ = position[2];
}

std::vector<double> Track::getPositionAtEcal() { return { x_at_ecal_, y_at_ecal_, z_at_ecal_ }; }


void Track::setMomentum(double bfield) {
    
    double mom_param = 2.99792458e-04;
    double pt = fabs(1. / omega_) * bfield * mom_param;
    py_ = pt*tan_lambda_;
    px_ = pt*sin(phi0_);
    pz_ = pt*cos(phi0_);
    //pterr_ = pow(1./omega,2)*sqrt(cov_[5])*bfield*mom_param;
}


void Track::setMomentum(std::vector<double> momentum) { 
    px_ = momentum[0]; 
    py_ = momentum[1]; 
    pz_ = momentum[2]; 
}

void Track::setMomentum(double px, double py, double pz) {
    px_ = px;
    py_ = py;
    pz_ = pz;
}

void Track::addHit(TObject* hit) {
    ++n_hits_; 
    tracker_hits_->Add(hit); 
}

void Track::Print (Option_t *option) const {
    printf("d0     Phi     Omega     TanLambda     Z0     time     chi2\n");
    printf("% 6.4f  % 6.4f  % 6.4f  % 6.4f  % 6.4f  % 6.4f  % 6.4f\n",d0_,phi0_,omega_,tan_lambda_,z0_,track_time_,chi2_);
    printf("type: %d\n", type_);
}

std::vector<float> Track::getCovEigenvalues(std::vector<float> cov) {
    //std::vector<double> V{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14};
    int dim = 5;
    TMatrixFSym covM(dim);
    std::vector<std::vector<float>> row; 
    int s=0;
    for(int i=0; i < dim; i++)
    {   row.push_back({});
        if(i==0){row[i].push_back(cov.at(i));}
        else
        {
            s=s+i;
            for(int j=0; j < i+1; j++)
            {
                row[i].push_back(cov.at(s+j));

            }
        }
            
    }
    
    for(int i=0; i < row.size(); i++)
    {
        for(int j=0; j < row[i].size(); j++)
        {
            covM(i,j) = row[i][j];
            covM(j,i) = row[i][j];
        }

    }
    
    TMatrixDSymEigen eigen(covM);
    TVectorD eigenVal = eigen.GetEigenValues();
    std::vector<float> eigenvals;
    for (int i = 0; i < dim; i++)
    {
        eigenvals.push_back(eigenVal[i]);
    }
    
    return eigenvals;
    


}
