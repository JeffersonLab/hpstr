/**
 * @file MCTrackerHit.h
 * @brief Class used to encapsulate mc tracker hit information
 * @author Cameron Bravo, SLAC National Accelerator Laboratory
 */

#ifndef _MCTRACKER_HIT_H_
#define _MCTRACKER_HIT_H_

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

class MCTrackerHit : public TObject { 

    public: 

        /** Constructor */
        MCTrackerHit();

        /** Destructor */
        virtual ~MCTrackerHit();

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

        /** @return the global X coordinate of the hit */
        double getGlobalY() const {return y_;}

        /** @return the global X coordinate of the hit */
        double getGlobalZ() const {return z_;}

        /**
         * Set the hit time.
         *
         * @param time The hit time.
         */
        void setTime(const double time) { time_ = time; };

        /** @return The hit time. */
        double getTime() const { return time_; };

        /**
         * Set the hit energy deposit.
         *
         * @param charge The hit energy.
         */
        void setEdep(const double edep) { edep_ = edep; };

        /** @return The hit energy deposit. */
        double getEdep() const { return edep_; };

        void setModule(const int module ) {module_ = module;} ;

        //** @return the tracker hit volume from the raw hit content */
        int getModule() { return module_;} ;

        //** set the tracker hit layer from the raw hit content */
        void setLayer(const int layer) {layer_ = layer;};

        //** @return the tracker hit layer from the raw hit content */
        int getLayer() const {return layer_;};

        //** set the pdg id of particle that made the hit */
        void setPDG(const int pdg) {pdg_ = pdg;};

        //** @return the pdg id of particle that made the hit */
        int getPDG() const {return pdg_;};

        ClassDef(MCTrackerHit, 1);	

    private:

        /** The x position of the hit. */
        double x_{-999}; 

        /** The x position of the hit. */
        double y_{-999}; 

        /** The x position of the hit. */
        double z_{-999};

        /** The hit time. */
        double time_{-999};

        /** Layer (Axial + Stereo). 1-12 in 2015/2016 geometry, 1-14 in 2019 geometry */
        int layer_{-999};

        /** Module */
        int module_{-999};

        /** Energy deposit of hit */
        float edep_{-999};

        /** pdg id of particle that made the hit */
        int pdg_{-999};

}; // MCTrackerHit

#endif // _MCTRACKER_HIT_H_
