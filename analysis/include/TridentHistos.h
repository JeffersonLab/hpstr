#ifndef TRIDENTHISTOS_H
#define TRIDENTHISTOS_H

#include "HistoManager.h"
#include "Track.h"
#include "TrackerHit.h"
#include "RawSvtHit.h"
#include "CalCluster.h"
#include "Vertex.h"
#include "Particle.h"
#include <string>
#include <vector>

class TridentHistos : public HistoManager {

    public:

        TridentHistos(const std::string& inputName) : 
            HistoManager(inputName) 
    {
        m_name = inputName;
        BuildAxes();
    }
            
        virtual void Define3DHistos(){};
        virtual void Define2DHistos();
        
        void BuildAxes();

        void Fill1DTrack(Track* track, double trkTimeOffset,float weight = 1., const std::string& trkname = "");
        void Fill2DTrack(Track* track,float weight = 1., const std::string& trkname = "");

        //This should probably go somewhere else
        void FillResidualHistograms(Track* track, int ly, double res, double sigma);
        
        void Fill1DVertex(Vertex* vtx, float weight = 1.);
        
        //TODO Change this
        void Fill1DVertex(Vertex* vtx, Particle* ele, Particle* pos, Track* ele_trk, Track* pos_trk, double trkTimeOffset,float weight = 1.);

        void Fill1DHistograms(Track* track = nullptr, Vertex* vtx = nullptr, float weight = 1.);
        void Fill2DHistograms(Vertex* vtx = nullptr, float weight = 1.);

        //Truth comparison
        void Fill1DTrackTruth(Track* track, Track* truth_track, float weight=1.,const std::string& ="");
	std::pair<CalCluster*, Track*> getTrackClusterPair(Track* trk,std::vector<CalCluster*>& clusters, float weight);
        //track_x goes for x axis, and y for y axis
        void FillTrackComparisonHistograms(Track* track_x, Track* track_y, float weight = 1.);
        void doTrackComparisonPlots(bool doplots) {doTrkCompPlots = doplots;};
	void FillTrackClusterHistos(std::pair<CalCluster, Track> ele, std::pair<CalCluster, Track> posOrGamma, double calTimeOffset, double trkTimeOffset,std::vector<CalCluster*>  * clusterList, double weight); 
	void FillWABHistos(std::pair<CalCluster*, Track*> ele, CalCluster* gamma,  double weight);
    private:

        // Vertices
        std::vector<std::string> vPs{"vtx_chi2", "vtx_X", "vtx_Y", "vtx_Z", "vtx_sigma_X","vtx_sigma_Y","vtx_sigma_Z","vtx_InvM","vtx_InvMErr"};
	

        bool doTrkCompPlots{false};

};

#endif //TRACKHISTOS_H
