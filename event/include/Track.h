/**
 * @file Track.h
 * @brief Class used to encapsulate track information.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef _TRACK_H_
#define _TRACK_H_

//----------------//
//   C++ StdLib   //
//----------------//
#include <cstdio>
#include <vector>

//----------//
//   ROOT   //
//----------//
#include <TObject.h>
#include <TClonesArray.h>
#include <TRefArray.h>
#include <TRef.h>

class Track : public TObject {

    public:

        /** Constructor */
        Track();

        /** Destructor */
        ~Track();

        /** Reset the Track object */ 
        void Clear(Option_t *option="");

        /**
         * Add a reference to an TrackerHit.
         *
         * @param hit : A TrackerHit object
         */
        void addHit(TObject* hit); 

        /** @return A reference to the hits associated with this track. */
        TRefArray* getSvtHits() const { return tracker_hits_; };

        /**
         * Set the track parameters.
         *
         * @param d0 Distance of closest approach to the reference point.
         * @param phi0 The azimuthal angle of the momentum at the distance of
         *             closest approach. 
         * @param omega The curvature of the track.
         * @param tan_lambda The slope of the track in the SY plane.
         * @param z0 The y position of the track at the distance of closest 
         *           approach.
         */
        void setTrackParameters(const double d0, 
                const double phi0, 
                const double omega, 
                const double tan_lambda,
                const double z0);
      
        /** @return The track parameters. */ 
        std::vector<double> getTrackParameters(); 

        /**
         * Set the chi^2 of the fit to the track.
         *
         * @param chi2 The chi^2 of the fit to the track.
         */
        void setChi2(const double chi2) { chi2_ = chi2; };
      
        /** @return the chi^2 of the fit to the track. */
        double getChi2() const { return chi2_; };

        /**
         * Set the isolation variable of the given layer.
         *
         * @param layer Layer number associated with the given isolation value.
         * @param isolation The isolation variable. 
         */ 
        void setIsolation(const int layer, const double isolation) { isolation_[layer] = isolation; };


        /**
         * Get the isolation value of the given layer.
         *
         * @param layer The SVT layer of interest.
         * @return The isolation value of the given layer.
         */
        double getIsolation(const int layer) const { return isolation_[layer]; }; 


        /** @param track_time The track time. */
        void setTrackTime(const double track_time) { track_time_ = track_time; };

        /**
         * Get the time of the track.
         *
         * @return The track time.
         */
        double getTrackTime() const { return track_time_; }; 

        /**
         * The the volume (Top/Bottom) that the track is located in.
         *
         * @param track_volume The track volume.
         */
        void setTrackVolume(const int track_volume) { track_volume_ = track_volume; };

        /**
         * Set the Particle associated with this track.  This can be used to
         * retrieve additional track properties such as the momentum and charge.
         *
         * @param particle : Final state particle associated with this track
         */
        void setParticle(TObject* particle) { particle_ = particle; };

        /** @return The {@link Particle} associated with this track. */
        TObject* getParticle() const { return static_cast<TObject*>(particle_.GetObject()); }; 

        /**
         * Set the extrapolated track position at the Ecal face. The 
         * extrapolation is assumed to use the full 3D field map.
         *
         * @parm position The extrapolated track position at the Ecal
         */
        void setPositionAtEcal(const double* position);

        /** @return Extrapolated track position at Ecal face. */
        std::vector<double> getPositionAtEcal(); 

        /**
         * Set the track type.  For more details, see {@link StrategyType} and
         * {@link TrackType}.
         *
         * @param type The track type. 
         */
        void setType(const int type) { type_ = type; }; 

        /** @return The track type. */
        int getType() const { return type_; }; 


        /** 
         * Set the track charge. 
         *
         * @param charge Track charge.
         */
        void setCharge(const int charge) { charge_ = charge; };

        /**
         * Get the charge of a the track.
         *
         * @return The charge associated of the track.
         */
        int getCharge() const { return charge_; };  

        /** 
         * Set the momentum of the track.  The momentum is extracted from
         * the corresponding ReconstructedParticle.
         *
         * @param momentum The momentum of the track.
         */
        void setMomentum(std::vector<double> momentum); 

         /** @return The track momentum. */
        std::vector<double> getMomentum() { return {px_, py_, pz_}; }; 
       
        /**
         * Set the lambda kink of the given layer.
         *
         * @param layer Layer number associated with the given lambda kink.
         * @param lambda_kink The lambda kink value.
         */
        void setLambdaKink(const int layer, const double lambda_kink) { lambda_kinks_[layer] = lambda_kink; }
       
        /**
         * Get the lambda kink value of the given layer.
         *
         * @param layer The SVT layer of interest.
         * @return The lambda kink value of the given layer.
         */
        double getLambdaKink(const int layer) const { return lambda_kinks_[layer]; }
        
        /**
         * Set the phi kink of the given layer.
         *
         * @param layer Layer number associated with the given phi kink.
         * @param phi_kink The phi kink value.
         */
        void setPhiKink(const int layer, const double phi_kink) { phi_kinks_[layer] = phi_kink; }

        /**
         * Get the phi kink value of the given layer.
         *
         * @param layer The SVT layer of interest.
         * @return The phi kink value of the given layer.
         */
        double getPhiKink(const int layer) const { return phi_kinks_[layer]; }

        /**
         * @returns True if the track is in the top SVT volume, false otherwise.
         */
        bool isTopTrack() const { return track_volume_ ? false : true; };

        /**
         * @return True if the track is in the bottom SVT volume, false otherwise.
         */
        bool isBottomTrack() const { return track_volume_ ? true : false; };

        /**
         * @return Number of tracker hits associated with this track.
         */
        int getTrackerHitCount() const { return n_hits_; };

    private:

        /** Reference to the 3D hits associated with this track. */
        TRefArray* tracker_hits_{new TRefArray{}}; 

        /** Reference to the reconstructed particle associated with this track. */
        TRef particle_;

        /** Array used to store the isolation variables for each of the sensor layers. */
        double isolation_[12];

        /** The number of 3D hits associated with this track. */
        int n_hits_{0}; 

        /** The volume to which this track belongs to. */
        int track_volume_{-999}; 

        /** The track type. */
        int type_{-999}; 

        /** The distance of closest approach to the reference point. */
        double d0_{-999}; 
       
        /**
         * The azimuthal angle of the momentum at the position of closest
         * approach to the reference point. 
        */
        double phi0_{-999};
        
        /**
         * The track curvature. The curvature is positive (negative) if the particle has a
         * positive (negative) charge.
         */
        double omega_{-999}; 

        /**
         * The slope of the track in the SY plane where S is the arc length of 
         * the helix in the xz plane.
         */ 
        double tan_lambda_{-999};
        
        /** 
         * The y position of the track at the distance of closest approach 
         * in the xz plane.
         */
        double z0_{-999}; 
        
        /** The chi^2 of the track fit. */ 
        double chi2_{-999};

        /** 
         * The time of the track.  This is currently the average time of all
         * hits composing the track.
         */
        double track_time_{-999};
        
        /** The x position of the extrapolated track at the Ecal face. */ 
        double x_at_ecal_{-999};
        
        /** The y position of the extrapolated track at the Ecal face. */ 
        double y_at_ecal_{-999};
        
        /** The z position of the extrapolated track at the Ecal face. */ 
        double z_at_ecal_{-999};
        
        /** Array used to store the lambda kinks for each of the sensor layers. */
        double lambda_kinks_[12];  
        
        /** Array used to store the phi kinks for each of the sensor layers. */
        double phi_kinks_[12];  

        /** Track momentum. */
        double px_{-9999}; 
        double py_{-9999}; 
        double pz_{-9999};

        /** Track charge. */
        int charge_{0}; 

        ClassDef(Track, 1);
}; // Track

#endif // __TRACK_H__
