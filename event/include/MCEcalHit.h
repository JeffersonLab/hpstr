/**
 * @file MCEcalHit.h
 * @brief Class used to encapsulate mc Ecal hit information
 * @author Cameron Bravo, SLAC National Accelerator Laboratory
 */

#ifndef _MC_ECAL_HIT_H_
#define _MC_ECAL_HIT_H_

//----------------//
//   C++ StdLib   //
//----------------//
#include <iostream>

//----------//
//   ROOT   //
//----------//
#include <TObject.h>
#include <TClonesArray.h>
#include <TRefArray.h>

class MCEcalHit : public TObject { 

    public: 

        /** Constructor */
        MCEcalHit();

        /** Destructor */
        virtual ~MCEcalHit();

        /** Reset the Hit object. */
        void Clear(Option_t *option="");

        /**
         * Set the hit position.
         *
         * @param position The hit position.
         */
        void setPosition(const double* position, bool rotate = false);

        /** @return The hit position. */
        std::vector<double> getPosition() const { return {x_, y_, z_}; };

        /** @return the global X coordinate of the hit */
        double getGlobalX() const {return x_;}

        /** @return the global Y coordinate of the hit */
        double getGlobalY() const {return y_;}

        /** @return the global Z coordinate of the hit */
        double getGlobalZ() const {return z_;}

        /**
         * Set the hit energy deposit.
         *
         * @param energy The hit energy.
         */
        void setEnergy(const double energy) { energy_ = energy; };

        /** @return The hit energy deposit. */
        double getEnergy() const { return energy_; };

        /**
         * Set the system of the hit.
         *
         * @param system The system of the hit.
         */
        void setSystem(const int system ) {system_ = system;} ;

        //** @return the Ecal hit system */
        int getSystem() { return system_;} ;

        /**
         * Set the layer of the hit.
         *
         * @param layer The layer of the hit.
         */
        void setLayer(const int layer) {layer_ = layer;};

        //** @return the Ecal hit layer from the raw hit content */
        int getLayer() const {return layer_;};

        /**
         * Set the x index of the hit.
         *
         * @param layer The x index of the hit.
         */
        void setIX(const int ix) {ix_ = ix;};

        //** @return the y index of the hit. */
        int getIX() const {return ix_;};

        /**
         * Set the y index of the hit.
         *
         * @param layer The y index of the hit.
         */
        void setIY(const int iy) {iy_ = iy;};

        //** @return the y index of the hit. */
        int getIY() const {return iy_;};

        ClassDef(MCEcalHit, 1);	

    private:

        /** The x position of the hit. */
        double x_{-999}; 

        /** The x position of the hit. */
        double y_{-999}; 

        /** The x position of the hit. */
        double z_{-999};

        /** System */
        int system_{-999};

        /** Layer */
        int layer_{-999};

        /** X index of crystal */
        int ix_{-999};

        /** Y index of crystal */
        int iy_{-999};

        /** Energy of hit */
        float energy_{-999};

}; // MCEcalHit

#endif // _MC_ECAL_HIT_H_
