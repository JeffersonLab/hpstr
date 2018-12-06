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
         * Set the layer associated with this hit.
         *
         * @param layer The layer associated with this hit.
         */
        void setLayer(const int layer){ this->layer = layer; };

        /**
         * Set the hit position.
         *
         * @param position The hit position.
         */
        void setPosition(const double* position);

        /** @return The hit position. */
        std::vector<double> getPosition() const;

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
        void setTime(const double time) { this->time = time; };

        /** @return The layer associated with this hit. */
        double getLayer() const { return layer; };


        /** @return The hit time. */
        double getTime() const { return time; };

        ClassDef(TrackerHit, 1);	
    
    private:

        /** The x position of the hit. */
        double x{-999}; 
        
        /** The x position of the hit. */
        double y{-999}; 
        
        /** The x position of the hit. */
        double z{-999};

        /** Components of the covariance matrix. */
        double cxx{0};
        double cxy{0};
        double cxz{0};
        double cyy{0};
        double cyz{0};
        double czz{0};

        /** The hit time. */
        double time{-999};
        
        /** The hit layer. */
        int layer{0};

}; // TrackerHit

#endif // _TRACKER_HIT_H_
