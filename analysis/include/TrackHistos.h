#ifndef TRACKHISTOS_H
#define TRACKHISTOS_H

#include "HistoManager.h"
#include "Track.h"
#include "TrackerHit.h"
#include "Vertex.h"
#include "Particle.h"
#include <string>
#include <vector>

/**
 * @brief description
 * 
 * details
 */
class TrackHistos : public HistoManager {

    public:
        /**
         * @brief Constructor
         * 
         * @param inputName 
         */
        TrackHistos(const std::string& inputName) : HistoManager(inputName) {
            m_name = inputName;
            BuildAxes();
        }

        /**
         * @brief description
         * 
         */
        virtual void Define3DHistos(){};
        
        /**
         * @brief description
         * 
         */
        virtual void Define2DHistos();
        
        /**
         * @brief description
         * 
         */
        void BuildAxes();

        /**
         * @brief description
         * 
         */
        void DefineTrkHitHistos();

        /**
         * @brief Fill 1D track.
         * 
         * @param track 
         * @param weight 
         * @param trkname 
         */
        void Fill1DTrack(Track* track, float weight = 1., const std::string& trkname = "");
        
        /**
         * @brief Fill 2D track.
         * 
         * @param track 
         * @param weight 
         * @param trkname 
         */
        void Fill2DTrack(Track* track, float weight = 1., const std::string& trkname = "");

        /**
         * @brief Fill residual histograms.
         * 
         * \todo This should probably go somewhere else
         * 
         * @param track 
         * @param ly 
         * @param res 
         * @param sigma 
         */
        void FillResidualHistograms(Track* track, int ly, double res, double sigma);
        
        /**
         * @brief description
         * 
         * @param vtx 
         * @param weight 
         */
        void Fill1DVertex(Vertex* vtx, float weight = 1.);

        /**
         * @brief description
         * 
         * \todo change this
         * 
         * @param vtx 
         * @param ele 
         * @param pos 
         * @param ele_trk 
         * @param pos_trk 
         * @param weight 
         */
        void Fill1DVertex(Vertex* vtx, Particle* ele, Particle* pos, Track* ele_trk, Track* pos_trk, float weight = 1.);

        /**
         * @brief Fill 1D histograms.
         * 
         * @param track 
         * @param vtx 
         * @param weight 
         */
        void Fill1DHistograms(Track* track = nullptr, Vertex* vtx = nullptr, float weight = 1.);

        /**
         * @brief Fill 2D histograms.
         * 
         * @param vtx 
         * @param weight 
         */
        void Fill2DHistograms(Vertex* vtx = nullptr, float weight = 1.);

        /**
         * @brief Truth comparison.
         * 
         * @param track 
         * @param truth_track 
         * @param weight 
         */
        void Fill1DTrackTruth(Track* track, Track* truth_track, float weight=1., const std::string& = "");

        /**
         * @brief description
         * 
         * @param track_x x-axis
         * @param track_y y-axis
         * @param weight 
         */
        void FillTrackComparisonHistograms(Track* track_x, Track* track_y, float weight = 1.);

        /**
         * @brief Compare tracks.
         * 
         * @param doplots 
         */
        void doTrackComparisonPlots(bool doplots) { doTrkCompPlots = doplots; };

    private:
        /** Vertices */
        std::vector<std::string> vPs{"vtx_chi2", "vtx_X", "vtx_Y", "vtx_Z", "vtx_sigma_X","vtx_sigma_Y","vtx_sigma_Z","vtx_InvM","vtx_InvMErr"};

        /** description */
        bool doTrkCompPlots{false};

};

#endif //TRACKHISTOS_H
