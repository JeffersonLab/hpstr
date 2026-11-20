#pragma once

//------------------//
//     C++          //
//------------------//
#include <map>
#include <memory>
#include <string>
#include <vector>

//------------------//
//     JSON         //
//------------------//
#include "json.hpp"

//------------------//
//    hpstr         //
//------------------//
#include "MCParticle.h"
#include "RunScaleCorrectionTool.h"
#include "Track.h"
#include "TrackBiasingTool.h"
#include "TrackSmearingTool.h"
#include "Vertex.h"

/**
 * One entry point for the track corrections a processor applies: it owns the
 * individual tools and runs them in a fixed order, configured by a single JSON.
 *
 * JSON keys (all optional; a missing or empty key switches that step off). Relative
 * paths are resolved against $HPSTR_BASE.
 *
 *   "trackOffsets":  file with constant offsets {"track_z0": x, "track_time": t}
 *   "z0Offset":      {"data": file, "mc": file}  per-run v0-projection z0 means
 *                    (elez0_mean / posz0_mean); the closest run at or below the
 *                    event's run is used
 *   "z0Calib":       {"data": file, "mc": file}  linear z0 calibration, z0 -= slope*tanL + intercept
 *   "momentumScale": {"runScale": file, "meanCorr": file, "meanCorrVariable": ""}
 *                    data only; see RunScaleCorrectionTool
 *   "smearing":      {"cfg": file, "smearingFactor": 1.0, "smearOmega": true,
 *                     "variable": "tanLambda", "requireTruthMatch": false,
 *                     "smearZ0": true, "trackCollection": "KalmanFullTracks"}
 *                    MC only, and only when Options::doSmearing is set
 *
 * Order, each step applied to all tracks before the next: trackOffsets, z0Offset,
 * z0Calib, momentumScale (data), z0 smearing (MC), momentum smearing (MC).
 *
 * Per-job settings (data/MC, seed, whether to smear, truth-match override) are not
 * in the JSON; the processor passes them in Options.
 */
class TrackCorrector {
  public:
    struct Options {
        bool isData{false};
        int seed{42};
        bool doSmearing{false};     //!< MC Gaussian smearing on/off for this job
        int requireTruthMatch{-1};  //!< -1: use the JSON value; 0/1: override it
        bool debug{false};
    };

    /** Momentum scale factors applied to one track. */
    struct Result {
        double pScale{1.0};  //!< momentumScale step (data)
        double pSmear{1.0};  //!< smearing step (MC)
        double pRatio() const { return pScale * pSmear; }
    };

    TrackCorrector(const std::string& cfgFile, const Options& opts);

    /** Call once per event, before correct(). */
    void beginEvent(int run, const std::vector<MCParticle*>* mcParticles = nullptr);

    /**
     * Correct the tracks in place. charges gives each track's charge sign, used to
     * pick the electron or positron z0 offset; if empty, Track::getCharge() is used.
     */
    std::vector<Result> correct(const std::vector<Track*>& tracks, const std::vector<int>& charges = {});

    /** True if correctVertex() should be called after correct() for vertex tracks. */
    bool updatesVertex() const { return updatesVertex_; }

    /**
     * Scale the vertex momenta p1, p2 by the tracks' momentum ratios and the
     * invariant mass by sqrt(r1 * r2), as TrackSmearingTool::updateVertexWithSmearP.
     */
    static void correctVertex(Vertex* vtx, double r1, double r2);

    bool hasSmearing() const { return smearing_ != nullptr; }
    bool smearsOmega() const { return smearOmega_; }
    bool hasMomentumScale() const { return momentumScale_ != nullptr; }

    void printConfig() const;

  private:
    std::string resolvePath(const std::string& path) const;
    std::string dataOrMc(const nlohmann::json& block) const;

    std::string cfgFile_;
    Options opts_;
    int run_{-1};

    std::map<std::string, double> trackOffsets_;

    std::string z0OffsetFile_;
    nlohmann::json z0OffsetFits_;
    int z0OffsetRun_{-1};
    double eleZ0Offset_{0.}, posZ0Offset_{0.};

    std::string z0CalibFile_;
    std::shared_ptr<TrackBiasingTool> z0Calib_;

    std::shared_ptr<RunScaleCorrectionTool> momentumScale_;

    std::shared_ptr<TrackSmearingTool> smearing_;
    bool smearOmega_{false};
    bool smearZ0_{true};
    bool applySmearing_{false};  //!< smearing configured, MC, and Options::doSmearing

    bool updatesVertex_{false};
};
