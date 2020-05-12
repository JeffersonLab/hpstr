#ifndef MCTRACKER_HIT_H
#define MCTRACKER_HIT_H

/*~~~~~~~~~~~~~~~~*/
/*   C++ StdLib   */
/*~~~~~~~~~~~~~~~~*/
#include <iostream>

/*~~~~~~~~~~*/
/*   ROOT   */
/*~~~~~~~~~~*/
#include "TObject.h"
#include "TRef.h" 

/*~~~~~~~~~~~*/
/*   event   */
/*~~~~~~~~~~~*/
#include "MCParticle.h" 

/**
 * Class which encapsulates information from a hit in a simulated tracking 
 * detector.
 */
class MCTrackerHit : public TObject { 

    public: 

        /// Constructor 
        MCTrackerHit();

        /// Destructor
        ~MCTrackerHit();

        /// Reset this MCTrackHit object
        void Clear(Option_t *option="") final override;

        /// Print a string representation of this object
        void Print(Option_t *option="") const final override; 

        /**
         * Set the hit position in mm. 
         *
         * If the rotate flag is set to true, the hits will be rotated by 
         * 30 mrad.
         * TODO: Make this more generic by passing the rotation angle instead.
         *
         * @param[in] position The hit position.
         */
        void setPosition(const double* position, bool rotate = false);

        /**
         * Get a vector with the x, y, z coordinates of the hit in mm.
         *
         * @return A vector containing the hit position. 
         */
        std::vector<double> getPosition() const { return {x_, y_, z_}; }

        /**
         * Get a vector with the global hit position in mm.
         *
         * @return A vector containing the global position of the hit.
         * TODO: What is this meant for? Doesn't seem to be any different than
         *       just getting the position. 
         */
        std::vector< double > getGlobalPosition() const { return { x_, y_, z_ }; } 

        /**
         * Set the hit time in ns.
         *
         * @param[in] time The hit time in ns.
         */
        void setTime(const double time) { time_ = time; }

        /**
         * Get the global hit time in ns.
         *
         * @return The global hit time in ns.
         */
        double getTime() const { return time_; }

        /**
         * Set the energy deposited by the hit MeV.
         *
         * @param charge The energy deposited by the hit.
         */
        void setEdep(const double edep) { edep_ = edep; }

        /**
         * Get the energy deposited by the hit in MeV.
         *
         * @return The energy deposited by the hit.  
         */
        double getEdep() const { return edep_; }

        /**
         * Set the LCIO ID of the particle that created this hit.
         *
         * @param[in] lcio_id the LCIO ID.
         */
        void setLcioID(const int lcio_id) { lcio_id_ = lcio_id; }

        /**
         * Get the LCIO ID of the particle that created this hit.
         *
         * @return The LCIO ID.
         */
        int getLcioID() const { return lcio_id_; }

        /**
         * Set the module ID associated with this hit.  This is used to 
         * uniquely identify a sensor within a layer. 
         *
         * @param[in] module the module ID.
         */
        void setModule(const int module ) { module_ = module; }
        
        /**
         * Get the module ID associated with this hit. This isued to uniquely
         * identify a sensor within a layer.
         *
         * @return the module ID.  
         */
        int getModule() const { return module_; }

        /**
         * Set the geometric layer ID of the hit.
         *
         * @param[in] layer The geometric layer ID of the hit.
         */
        void setLayer(const int layer) { layer_ = layer; }

        /**
         * Get the geometric layer ID of the hit.
         *
         * @return The geometric layer ID of the hit. 
         */
        int getLayer() const { return layer_; }

        /** 
         * Set the PDG id of particle that made the hit. 
         *
         * param[in] pdg The PDG ID of the hit.
         */
        void setPDG(const int pdg) { pdg_ = pdg; }

        /**
         * Get the PDG id of the particle that made this hit.
         * 
         * @return The PDG ID of particle that made this hit. 
         */
        int getPDG() const { return pdg_; }

        /**
         * Set the Monte Carlo particle that created this hit in the simulation.
         * 
         * @param mc_particle The particle that created the hit.
         */
        void setMCParticle(MCParticle* mc_particle) { mc_particle_ = static_cast< TObject* >(mc_particle); }

        /**
         * Set the Monte Carlo particle that created this hit in the simulation.
         *
         * @return A pointer to the MC particle that created this hit in the 
         *      simulation.
         */
        MCParticle* getMCParticle() const { return static_cast< MCParticle* >(mc_particle_.GetObject()); }

        ClassDef(MCTrackerHit, 2);	

    private:


        /// The angle of rotation of the SVT.
        // TODO: Move this to a constants header.
        const float svt_angle_{30.5e-3}; 

        /// The x position of the hit in mm.
        double x_{-999}; 

        /// The y position of the hit in mm.
        double y_{-999}; 

        /// The z position of the hit in mm. 
        double z_{-999};

        /// The hit time in ns. 
        double time_{-999};

        /**
         *  Layer (Axial + Stereo). 1-12 in 2015/2016 geometry, 
         *  1-14 in 2019 geometry 
         */
        int layer_{-999};

        /// Module ID (0 - 2)
        int module_{-999};

        /// Energy deposit of hit MeV
        float edep_{-999};

        /// PDG ID of particle the particle that made the hit.
        int pdg_{-999};

        /// The LCIO ID of the MC particle that created this hit
        int lcio_id_{-9999}; 

        /// The MC particle that created this hit.
        TRef mc_particle_{nullptr};

}; // MCTrackerHit

#endif // MCTRACKER_HIT_H
