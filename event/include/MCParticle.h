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
         * Get number of the daughter particles composing this particle.
         *
         * @return number of the daughter particles composing this particle.
         */
        int getNumDaughters() const {return n_daughters_; };

        /**
         * Set number of the daughter particles composing this particle.
         *
         * @param number of the daughter particles composing this particle.
         */
        void setNumDaughters(const int num) { n_daughters_ = num; };

        /**
         * Add ID of a daughter composing this particle.
         *
         * @param ID of a daughter composing this particle.
         */
        void addDaughterID(int id){ vect_id_daughters.push_back(id); };

        /**
         * Get vector of IDs of daughter particles composing this particle.
         *
         * @return vector of IDs of daughter particles composing this particle.
         */
        std::vector<int> getIDDaugheters() const { return vect_id_daughters; };
       
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
         * Set the LCIO ID of this particle.
         *
         * @param id The LCIO ID of this particle
         */
        void setID(const int id) { id_ = id; }; 

        /**
         * Set the PDG ID of the mother of this particle.
         *
         * @param momPDG The PDG ID of the mother of this particle
         */
        void setMomPDG(const int momPDG) { momPDG_ = momPDG; }; 

        /**
         * Set the ID of the mother of this particle.
         *
         * @param ID The ID of the mother of this particle
         */
        void setMomID(const int id) { momID_ = id; };

        /**
         * Set the generator status of the particle.
         *
         * @param gen_ MCParticle generator status
         */
        void setGenStatus(const int gen) { gen_ = gen; };

        /**
         * Set the generator status of the particle.
         *
         * @param sim_ MCParticle generator status
         */
        void setSimStatus(const int sim) { sim_ = sim; };
       
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
       
        /** @return The particle PDG ID. */
        int getPDG() const { return pdg_; }; 
        
        /** @return The particle LCIO ID. */
        int getID() const { return id_; }; 
        
        /** @return The particle ID of the mother. */
        int getMomPDG() const { return momPDG_; }; 
        
        /** @return The particle ID of the mother. */
        int getMomID() const { return momID_; };

        /** @return The particle generator status. */
        int getGenStatus() const { return gen_; }; 
        
        /** @return The particle simulator status. */
        int getSimStatus() const { return sim_; }; 
        
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
    
        /** The LCIO ID of this particle */
        int id_{-9999}; 

        /** The number of daughters associated with this particle */    
        int n_daughters_{0};

        /** The number of daughters associated with this particle */
        std::vector<int> vect_id_daughters;

        /** The charge of this particle */
        int charge_{-9999}; 

        /** The PDG ID of this particle */
        int pdg_{-9999}; 

        /** The PDG ID of mother of this particle */
        int momPDG_{-9999}; 

        /** The ID of mother of this particle */
        int momID_{-9999};

        /** The generator status of the particle */ 
        int gen_{-9999}; 

        /** The generator status of the particle */ 
        int sim_{-9999}; 

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
