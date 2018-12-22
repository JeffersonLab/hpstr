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

class TrackerHit : public TObject { 

    public: 

        /** Constructor */
		TrackerHit();

        /** Destructor */
		virtual ~TrackerHit();

        /** Reset the Hit object. */
        void Clear(Option_t *option="");
        
        /**
         * Set the hit position.
         *
         * @param position The hit position.
         */
        void setPosition(const double* position);

        /** @return The hit position. */
        std::vector<double> getPosition() const { return {x_, y_, z_}; };

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

        ClassDef(TrackerHit, 1);	
    
    private:

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
        

}; // TrackerHit

#endif // _TRACKER_HIT_H_
