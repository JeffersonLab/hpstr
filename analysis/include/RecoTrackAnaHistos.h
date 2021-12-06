#ifndef RECOTRACKANAHISTOS_H
#define RECOTRACKANAHISTOS_H

#include "HistoManager.h"
#include "Track.h"
#include "TrackerHit.h"
#include "Vertex.h"
#include "Particle.h"
#include <string>
#include <vector>

class RecoTrackAnaHistos : public HistoManager {

    public:

	RecoTrackAnaHistos(const std::string& inputName) :
            HistoManager(inputName)
    {
        m_name = inputName;
        BuildAxes();
    }

        virtual void Define3DHistos(){};
        virtual void Define2DHistos();

        void BuildAxes();
        void DefineTrkHitHistos();

        void Fill1DTrack(Track* track, float weight = 1., const std::string& trkname = "");
        void Fill2DTrack(Track* track, float weight = 1., const std::string& trkname = "");

        //This should probably go somewhere else
        void FillResidualHistograms(Track* track, int ly, double res, double sigma);

        void Fill1DVertex(Vertex* vtx, float weight = 1.);

        //TODO Change this
        void Fill1DVertex(Vertex* vtx, Particle* ele, Particle* pos, Track* ele_trk, Track* pos_trk, float weight = 1.);

        void Fill1DHistograms(Track* track = nullptr, Vertex* vtx = nullptr, float weight = 1.);
        void Fill2DHistograms(Vertex* vtx = nullptr, float weight = 1.);

        //Truth comparison
        void Fill1DTrackTruth(Track* track, Track* truth_track, float weight=1.,const std::string& ="");

        //track_x goes for x axis, and y for y axis
        void FillTrackComparisonHistograms(Track* track_x, Track* track_y, float weight = 1.);
        void doTrackComparisonPlots(bool doplots) {doTrkCompPlots = doplots;};

    private:

        // Vertices
        std::vector<std::string> vPs{"vtx_chi2", "vtx_X", "vtx_Y", "vtx_Z", "vtx_sigma_X","vtx_sigma_Y","vtx_sigma_Z","vtx_InvM","vtx_InvMErr"};

        bool doTrkCompPlots{false};

};

#endif //RECOTRACKANAHISTOS_H
