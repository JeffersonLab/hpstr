/**
 * @file MCParticle.h
 * @brief Class used to encapsulate information about a mc particle. 
 * @author Cameron Bravo, SLAC National Accelerator Laboratory
 */

#ifndef _MCPARTICLE_H_
#define _MCPARTICLE_H_

//----------//
//   ROOT   //
//----------//
#include <TClonesArray.h>
#include <TObject.h>
#include <TRef.h>
#include <TRefArray.h>


class MCParticle : public TObject { 

    public:

        /** Default Constructor. */
        MCParticle(); 

        /** Destructor. */
        ~MCParticle();

        /** Reset this MCParticle object */
        void Clear(Option_t *option="");

        /**
         * Add a reference to an Particle object.  This will be used to
         * add daughter particles to this particle.
         *
         * @param particle Daughter particle composing this particle
         */
        void addDaughter(MCParticle* particle); 
       
        /**
         * Get the daughter particles composing this particle.
         *
         * @return An array of references to the daughter particles associated
         *         with this particle
         */
        TRefArray* getDaughters() const { return daughters_; }; 

        /**
         * Set the charge of the particle.
         *
         * @param charge_ MCParticle charge
         */
        void setCharge(const int charge) { charge_ = charge; };
       
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
         * Set the invariant mass of the particle in GeV.
         *
         * @param mass The invariant mass of the particle
         */
        void setTime(const double time) { time_ = time; }; 
       
        /**
         * Set the momentum of the particle in GeV.
         *
         * @param momentum An array containing the three momentum components 
         *                 of the particle.
         */
        void setMomentum(const double* momentum);

        /**
         * Set the vertex position of the particle.
         *
         * @param vtx_pos An array containing the three vertex position
         *                components of the particle
         */
        void setVertexPosition(const double* vtx_pos);

        /**
         * Set the end point of the particle.
         *
         * @param ep_pos An array containing the three endpoint
         *                components of the particle
         */
        void setEndPoint(const double* ep_pos);
 
        /** @return The particle charge. */
        int getCharge() const { return charge_; };
       
        /** @return The particle ID. */
        int getPDG() const { return pdg_; }; 
        
        /** @return The particle energy in GeV. */
        double getEnergy() const { return energy_; }; 
        
        /** @return The invariant mass of the particle in GeV. */
        double getMass() const { return mass_; }; 
        
        /** @return The time of the particle */
        double getTime() const { return time_; }; 
        
        /** @return The momentum of the particle. */
        std::vector<double> getMomentum() const;  
       
        /** @return The vertex position of the particle. */
        std::vector<double> getVertexPosition() const;

        /** @return The vertex position of the particle. */
        std::vector<double> getEndPoint() const;

        ClassDef(MCParticle, 1);

    private:

        /**
         *  An array of references to daughter particles associated with this
         *  particle
         */  
        TRefArray* daughters_{new TRefArray{}}; 
    
        /** The number of daughters associated with this particle */    
        int n_daughters_{0};

        /** The charge of this particle */
        int charge_{-9999}; 

        /** The PDG ID of this particle */
        int pdg_{-9999}; 

        /** The x component of the momentum of this particle in GeV */
        double px_{-9999}; 

        /** The y component of the momentum of this particle in GeV */
        double py_{-9999}; 

        /** The z component of the momentum of this particle in GeV */
        double pz_{-9999};
        
        /** The x component of the vertex of this particle in mm*/
        double vtx_x_{-9999};

        /** The y component of the vertex of this particle in mm */
        double vtx_y_{-9999};

        /** The z component of the vertex of this particle in mm */
        double vtx_z_{-9999};

        /** The x component of the end point of this particle in mm*/
        double ep_x_{-9999};

        /** The y component of the end point of this particle in mm */
        double ep_y_{-9999};

        /** The z component of the end point of this particle in mm */
        double ep_z_{-9999};

        /** The energy of the particle in GeV */
        double energy_{-9999}; 

        /** The invariant mass of the particle in GeV */ 
        double mass_{-9999}; 

        /** The time of the particle */ 
        double time_{-9999}; 

};  // MCParticle

#endif // _MCPARTICLE_H_
