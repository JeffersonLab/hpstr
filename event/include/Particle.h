/**
 * @file Particle.h
 * @brief Class used to encapsulate information about a particle. 
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef _PARTICLE_H_
#define _PARTICLE_H_

//----------//
//   ROOT   //
//----------//
#include <TClonesArray.h>
#include <TObject.h>
#include <TRef.h>
#include <TRefArray.h>


class Particle : public TObject { 

    public:

        /** Default Constructor. */
        Particle(); 

        /** Destructor. */
        ~Particle();

        /** Reset this Particle object */
        void Clear(Option_t *option="");

        /**
         * Add a reference to a Track object.  This will be used to 
         * associate a particle with the track that composes it.
         *
         * @param track Track whose reference will be added
         */
        void addTrack(TObject* track);
        
        /**
         * @return An array of references to the tracks associated with this
         *         particle
         */
        TRefArray* getTracks() const { return tracks_; } 

        /**
         * Add a reference to an CalCluster object.  This will be used
         * to associated a particle with the calorimeter cluster that composes it.
         *
         * @param cluster Cluster whose reference will be added
         */
        void addCluster(TObject* cluster);
       

        /**
         * @return An array of references to the calorimeter clusters associated
         *         with this particle
         */
        TRefArray* getClusters() const { return clusters_; };

        /**
         * Add a reference to an Particle object.  This will be used to
         * add daughter particles to this particle.
         *
         * @param particle Daughter particle composing this particle
         */
        void addParticle(Particle* particle); 
       
        /**
         * Get the daughter particles composing this particle.
         *
         * @return An array of references to the daughter particles associated
         *         with this particle
         */
        TRefArray* getParticles() const { return particles_; }; 

        /**
         * Set the charge of the particle.
         *
         * @param charge_ Particle charge
         */
        void setCharge(const int charge) { charge_ = charge; };
       
        /**
         * Set the overall goodness of the PID for this particle. 
         *
         * @param goodness_pid_ The goodness of the PID. 
         */
        void setGoodnessOfPID(const double goodness_pid) { goodness_pid_ = goodness_pid; }; 

        /**
         * Set the type of this particle.
         *
         * @param type The type of this particle
         */
        void setType(const int type) { type_ = type; }; 

        /**
         * Set the PDG ID of this particle.
         *
         * @param pdg The PDG ID of this particle
         */
        void setPDG(const int pdg) { pdg_ = pdg; }; 
       
        /**
         * Set the energy of the particle in GeV.
         *
         * @param energy The energy of this particle
         */
        void setEnergy(const double energy) { energy_ = energy; };
        
        /**
         * Set the invariant mass of the particle in GeV.
         *
         * @param mass The invariant mass of the particle
         */
        void setMass(const double mass) { mass_ = mass; }; 
       
        /**
         * Set the momentum of the particle in GeV.
         *
         * @param momentum An array containing the three momentum components 
         *                 of the particle.
         */
        void setMomentum(const double* momentum);

        /**
         * Set the corrected momentum of the paritcle in GeV.
         *
         * @param momentum An array containing the three momentum components
         *                 of the particle.
         */
        void setCorrMomentum(const double* momentum);  

        /**
         * Set the vertex position of the particle.
         *
         * @param vtx_pos An array containing the three vertex position
         *                components of the particle
         */
        void setVertexPosition(const float* vtx_pos);

        /**
         * Set the chi^2 of the vertex fit.
         *
         * @param vtx_fit_chi2 The chi^2 of the vertex fit
         */
        void setVertexFitChi2(const double vtx_fit_chi2) { vtx_fit_chi2_ = vtx_fit_chi2; };
 
        /** @return The particle charge. */
        int getCharge() const { return charge_; };
       
        /** @return The goodness of the PID. */
        double getGoodnessOfPID() const { return goodness_pid_; }; 
            
        /** @return The type of this particle. */
        int getType() const { return type_; }; 

        /** @return The particle ID. */
        int getPDG() const { return pdg_; }; 
        
        /** @return The particle energy in GeV. */
        double getEnergy() const { return energy_; }; 
        
        /** @return The invariant mass of the particle in GeV. */
        double getMass() const { return mass_; }; 
        
        /** @return The momentum of the particle. */
        std::vector<double> getMomentum() const;  
       
        /** @return The corrected momentum of the paritcle in GeV. */
        std::vector<double> getCorrMomentum() const;  

        /** @return The vertex position of the particle. */
        std::vector<double> getVertexPosition() const;

        /** @return The chi^2 of the vertex fit. */
        double getVertexFitChi2() const { return vtx_fit_chi2_; }; 

        ClassDef(Particle, 1);

    private:

        /** An array of references to tracks associated with this particle */
        TRefArray* tracks_{new TRefArray{}};

        /** 
         * An array of references to calorimeter clusters associated with this
         * particle 
         */
        TRefArray* clusters_{new TRefArray{}};
        
        /**
         *  An array of references to daughter particles associated with this
         *  particle
         */  
        TRefArray* particles_{new TRefArray{}}; 
    
        /** The number of daughters associated with this particle */    
        int n_daughters_{0};

        /** The charge of this particle */
        int charge_{-9999}; 

        /** The type of this particle */
        int type_{-9999}; 

        /** The PDG ID of this particle */
        int pdg_{-9999}; 

        /** The goodness of PID of this particle. */
        double goodness_pid_{-9999}; 

        /** The x component of the momentum of this particle in GeV */
        double px_{-9999}; 

        /** The x component of the corrected momentum of this particle in GeV */
        double px_corr_{-9999}; 

        /** The y component of the momentum of this particle in GeV */
        double py_{-9999}; 

        /** The y component of the corrected momentum of this particle in GeV */
        double py_corr_{-9999}; 
        
        /** The z component of the momentum of this particle in GeV */
        double pz_{-9999};
        
        /** The z component of the corrected momentum of this particle in GeV */
        double pz_corr_{-9999};

        /** The x component of the vertex of this particle in mm*/
        double vtx_x_{-9999};

        /** The y component of the vertex of this particle in mm */
        double vtx_y_{-9999};

        /** The z component of the vertex of this particle in mm */
        double vtx_z_{-9999};

        /** The chi^2 of the vertex fit */
        double vtx_fit_chi2_{-9999};

        /** The energy of the particle in GeV */
        double energy_{-9999}; 

        /** The invariant mass of the particle in GeV */ 
        double mass_{-9999}; 

};  // Particle

#endif // _PARTICLE_H_
