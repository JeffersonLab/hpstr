/**
 * @file TrackerHit.h
 * @brief Class used to encapsulate tracker hit information
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef _TRACKER_HIT_H_
#define _TRACKER_HIT_H_

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

class TrackerHit : public TObject { 

    public: 

        /** Constructor */
		TrackerHit();

        /** Destructor */
		virtual ~TrackerHit();

        /** Reset the Hit object. */
        void Clear(Option_t *option="");

        /** Get the references to the raw hits associated with this tracker hit */
        TRefArray* getRawHits() const {return raw_hits_;};
        
        /**
         * Set the hit position.
         *
         * @param position The hit position.
         */
        void setPosition(const double* position, bool rotate = false);

        //TODO: avoid returning a vector, rather pass by ref.
        /** @return The hit position. */
        std::vector<double> getPosition() const { return {x_, y_, z_}; };

        /** @return the global X coordinate of the hit */
        double getGlobalX() const {return x_;}

        /** @return the global X coordinate of the hit */
        double getGlobalY() const {return y_;}

        /** @return the global X coordinate of the hit */
        double getGlobalZ() const {return z_;}

        /**
         * Set the covariance matrix.
         *
         * @param cov The covariance matrix.
         */
        void setCovarianceMatrix(std::vector<float> cov);

        /** @return The convariance matrix. */
        std::vector<double> getCovarianceMatrix() const;

        /**
         * Set the hit time.
         *
         * @param time The hit time.
         */
        void setTime(const double time) { time_ = time; };

        /** @return The hit time. */
        double getTime() const { return time_; };

        /**
         * Set the hit charge deposit.
         *
         * @param charge The hit charge.
         */
        void setCharge(const double charge) { charge_ = charge; };

        /** @return The hit time. */
        double getCharge() const { return charge_; };


        /** Add raw hit to the raw hit reference array */
        void addRawHit(TObject* rawhit) {
	  ++n_rawhits_;
	  raw_hits_->Add(rawhit);
	}

        ClassDef(TrackerHit, 1);	
    
    private:

        /** Number of raw hits forming the Tracker hit */
        int n_rawhits_{0};

        /** The x position of the hit. */
        double x_{-999}; 
        
        /** The x position of the hit. */
        double y_{-999}; 
        
        /** The x position of the hit. */
        double z_{-999};

        /** Components of the covariance matrix. */
        double cxx_{0};
        double cxy_{0};
        double cxz_{0};
        double cyy_{0};
        double cyz_{0};
        double czz_{0};

        /** The hit time. */
        double time_{-999};
        
        /** The hit charge deposit. */
        double charge_{-999};

        /** The raw hits */
        TRefArray* raw_hits_{new TRefArray{}};
        

}; // TrackerHit

#endif // _TRACKER_HIT_H_
