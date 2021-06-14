#ifndef __TRIGGERPARAMETERSEXTRACTION_ANAPROCESSOR_H__
#define __TRIGGERPARAMETERSEXTRACTION_ANAPROCESSOR_H__


//HPSTR
#include "HpsEvent.h"
#include "Collections.h"
#include "Track.h"
#include "CalCluster.h"
#include "CalHit.h"
#include "MCParticle.h"
#include "Particle.h"
#include "Processor.h"
#include "HistoManager.h"
#include "TriggerParametersExtractionAnaHistos.h"

#include "BaseSelector.h"

//ROOT
#include "TFile.h"
#include "TTree.h"
#include "TRefArray.h"
#include "TBranch.h"
#include "TVector3.h"
#include "TLorentzVector.h"

//C++
#include <memory>


struct char_cmp {
    bool operator () (const char *a,const char *b) const
    {
        return strcmp(a,b)<0;
    }
};


class TriggerParametersExtractionAnaProcessor : public Processor {

    public:
		TriggerParametersExtractionAnaProcessor(const std::string& name, Process& process);
        ~TriggerParametersExtractionAnaProcessor();
        virtual bool process(IEvent* ievent);

        virtual void initialize(TTree* tree);

        virtual void finalize();

        virtual void configure(const ParameterSet& parameters);

        /**
         * Gets the mapped position used by the VTP for a specific crystal. Warning:
         * This is the hardware position mapping, and does not always align perfectly
         * with the LCSim coordinate system. For instance, the electron side of the
         * detector is has positive x coordinates in the hardware system, and is
         * negative in LCSim.
         */
        std::vector<double> getCrystalPosition(CalCluster cluster);

        /**
         * Calculates the value used by the coplanarity cut.
         */
        double getValueCoplanarity(CalCluster clusterTop, CalCluster clusterBot);

        std::vector<double> getVariablesForEnergySlopeCut(CalCluster clusterTop, CalCluster clusterBot);

    private:
        //Containers to hold histogrammer info
        TriggerParametersExtractionAnaHistos* histos{nullptr};
        std::string  histCfgFilename_;

        TTree* tree_{nullptr};
        TBranch* btrks_{nullptr};
        TBranch* bgtpClusters_{nullptr};
        TBranch* bmcParts_{nullptr};

        std::vector<Track*>  * trks_{};
        std::vector<CalCluster*> * gtpClusters_{};
        std::vector<MCParticle*>  * mcParts_{};

        std::string anaName_{"vtxAna"};
        std::string trkColl_{"GBLTracks"};
        std::string gtpClusColl_{"RecoEcalClustersGTP"};
        std::string mcColl_{"MCParticle"};


        double beamE_{3.7};
        int isData_{0};
        std::string analysis_{"triggerParametersExtraction"};

        //Debug level
        int debug_{0};

        /*
         * Parameters for all cut functions depend on beam energy.
         * Here, the setup is for 3.7 GeV.
         */

        //Cut functions for X
        TF1 *func_pos_top_topCutX;
        TF1 *func_pos_top_botCutX;

        TF1 *func_neg_top_topCutX;
        TF1 *func_neg_top_botCutX;

        TF1 *func_pos_bot_topCutX;
        TF1 *func_pos_bot_botCutX;

        TF1 *func_neg_bot_topCutX;
        TF1 *func_neg_bot_botCutX;

        //Cut functions for Y
        TF1 *func_pos_top_topCutY;
        TF1 *func_pos_top_botCutY;

        TF1 *func_neg_top_topCutY;
        TF1 *func_neg_top_botCutY;

        TF1 *func_pos_bot_topCutY;
        TF1 *func_pos_bot_botCutY;

        TF1 *func_neg_bot_topCutY;
        TF1 *func_neg_bot_botCutY;

        //Cut function for PDE
        TF1 *func_pde;

        //Cut function for energy slope
        TF1 *func_energy_slope;

        //Parameters of cut functions for X
        double pos_top_topCutX[2] = {20.5128, 0.896808};
        double pos_top_botCutX[2] = {-13.4493, 0.845145};

        double neg_top_topCutX[2] = {21.9999, 0.866766};
        double neg_top_botCutX[2] = {-20.7497, 0.903365};

        double pos_bot_topCutX[2] = {20.1246, 0.898751};
        double pos_bot_botCutX[2] = {-13.0183, 0.842857};

        double neg_bot_topCutX[2] = {23.4249, 0.867771};
        double neg_bot_botCutX[2] = {-21.4538, 0.905457};

        //Parameters of cut functions for Y
        double pos_top_topCutY[2] = {7.16273, 0.929672};
        double pos_top_botCutY[2] = {-5.67945, 0.878445};

        double neg_top_topCutY[2] = {7.71179, 0.902755};
        double neg_top_botCutY[2] = {-5.92585, 0.905328};

        double pos_bot_topCutY[2] = {4.62628, 0.864853};
        double pos_bot_botCutY[2] = {-6.37296, 0.938625};

        double neg_bot_topCutY[2] = {4.7885, 0.889888};
        double neg_bot_botCutY[2] = {-6.75031, 0.913876};

        //Parameters of cut function for PDE
        double pars_pde[4] = {0.751987, -0.0527523, 0.00133877, -6.13361e-06}; // 99%

        //Parameters of cut function for energy slope
        double pars_energy_slope[2] = {1.02439, -0.00245455}; // 3sigma

        /**
         * An array of the form <code>position[iy][ix]</code> that contains the hardware
         * VTP position mappings for each crystal. Note that ix in the array goes from
         * -22 (representing ix = 23) up to 25 (representing ix = 23) and uses array
         * index x = 0 as a valid parameter, while ix skips zero.<br/>
         * <br/>
         * Note that in this table, position[][] = { x, z, y } by in the coordinate
         * system employed by the rest of the class.
         */

        double positionMap[11][46][3] = {
                { { -340.003, 97.065, 87.845 }, { -324.283, 97.450, 87.875 }, { -308.648, 97.810, 87.900 },
                        { -293.093, 98.150, 87.920 }, { -277.618, 98.470, 87.940 }, { -262.213, 98.765, 87.965 },
                        { -246.878, 99.040, 87.980 }, { -231.603, 99.290, 87.995 }, { -216.393, 99.520, 88.010 },
                        { -201.228, 99.725, 88.030 }, { -186.118, 99.905, 88.040 }, { -171.058, 100.070, 88.050 },
                        { -156.038, 100.205, 88.055 }, { -141.058, 100.325, 88.070 }, { -126.113, 100.415, 88.075 },
                        { -111.198, 100.485, 88.075 }, { -96.313, 100.530, 88.080 }, { -81.453, 100.555, 88.085 },
                        { -66.608, 100.560, 88.085 }, { -51.788, 100.540, 88.080 }, { -36.983, 100.490, 88.075 },
                        { -22.183, 100.425, 88.075 }, { -7.393, 100.335, 88.070 }, { 7.393, 100.335, 88.070 },
                        { 22.183, 100.425, 88.075 }, { 36.983, 100.490, 88.075 }, { 51.793, 100.540, 88.080 },
                        { 66.613, 100.560, 88.085 }, { 81.453, 100.555, 88.085 }, { 96.313, 100.530, 88.080 },
                        { 111.198, 100.485, 88.075 }, { 126.113, 100.415, 88.075 }, { 141.053, 100.325, 88.070 },
                        { 156.038, 100.205, 88.055 }, { 171.053, 100.070, 88.050 }, { 186.118, 99.905, 88.040 },
                        { 201.228, 99.725, 88.030 }, { 216.388, 99.520, 88.010 }, { 231.608, 99.290, 87.995 },
                        { 246.878, 99.040, 87.980 }, { 262.218, 98.765, 87.965 }, { 277.623, 98.470, 87.940 },
                        { 293.098, 98.150, 87.920 }, { 308.653, 97.810, 87.900 }, { 324.288, 97.450, 87.875 },
                        { 340.008, 97.065, 87.845 } },
                { { -340.003, 97.040, 72.715 }, { -324.283, 97.420, 72.735 }, { -308.648, 97.785, 72.750 },
                        { -293.093, 98.125, 72.765 }, { -277.618, 98.450, 72.785 }, { -262.213, 98.745, 72.800 },
                        { -246.878, 99.015, 72.815 }, { -231.603, 99.265, 72.825 }, { -216.388, 99.495, 72.840 },
                        { -201.228, 99.700, 72.850 }, { -186.118, 99.885, 72.860 }, { -171.058, 100.045, 72.865 },
                        { -156.033, 100.185, 72.875 }, { -141.053, 100.300, 72.880 }, { -126.108, 100.395, 72.880 },
                        { -111.193, 100.460, 72.890 }, { -96.308, 100.510, 72.890 }, { -81.448, 100.535, 72.895 },
                        { -66.608, 100.535, 72.890 }, { -51.788, 100.510, 72.890 }, { -36.978, 100.470, 72.890 },
                        { -22.183, 100.405, 72.880 }, { -7.388, 100.310, 72.880 }, { 7.393, 100.310, 72.880 },
                        { 22.188, 100.405, 72.885 }, { 36.983, 100.470, 72.890 }, { 51.793, 100.510, 72.890 },
                        { 66.613, 100.535, 72.890 }, { 81.453, 100.535, 72.895 }, { 96.313, 100.510, 72.890 },
                        { 111.198, 100.460, 72.890 }, { 126.113, 100.395, 72.880 }, { 141.063, 100.300, 72.880 },
                        { 156.043, 100.185, 72.875 }, { 171.063, 100.045, 72.865 }, { 186.123, 99.885, 72.860 },
                        { 201.233, 99.700, 72.850 }, { 216.393, 99.495, 72.840 }, { 231.608, 99.265, 72.825 },
                        { 246.883, 99.015, 72.815 }, { 262.218, 98.745, 72.800 }, { 277.623, 98.450, 72.785 },
                        { 293.098, 98.125, 72.765 }, { 308.653, 97.785, 72.750 }, { 324.288, 97.420, 72.735 },
                        { 340.008, 97.040, 72.715 } },
                { { -340.003, 96.990, 57.600 }, { -324.283, 97.375, 57.610 }, { -308.648, 97.740, 57.625 },
                        { -293.093, 98.080, 57.630 }, { -277.618, 98.395, 57.645 }, { -262.213, 98.700, 57.655 },
                        { -246.873, 98.970, 57.660 }, { -231.603, 99.220, 57.670 }, { -216.383, 99.450, 57.680 },
                        { -201.228, 99.660, 57.685 }, { -186.113, 99.840, 57.695 }, { -171.053, 100.005, 57.695 },
                        { -156.033, 100.140, 57.700 }, { -141.053, 100.255, 57.710 }, { -126.108, 100.345, 57.710 },
                        { -111.193, 100.420, 57.710 }, { -96.308, 100.465, 57.715 }, { -81.448, 100.490, 57.715 },
                        { -66.608, 100.490, 57.715 }, { -51.788, 100.470, 57.710 }, { -36.978, 100.425, 57.710 },
                        { -22.178, 100.355, 57.710 }, { -7.388, 100.265, 57.705 }, { 7.398, 100.265, 57.705 },
                        { 22.188, 100.355, 57.710 }, { 36.988, 100.425, 57.710 }, { 51.793, 100.470, 57.710 },
                        { 66.613, 100.490, 57.715 }, { 81.458, 100.490, 57.715 }, { 96.318, 100.465, 57.715 },
                        { 111.198, 100.420, 57.710 }, { 126.118, 100.345, 57.710 }, { 141.063, 100.255, 57.710 },
                        { 156.043, 100.140, 57.700 }, { 171.063, 100.005, 57.695 }, { 186.123, 99.840, 57.695 },
                        { 201.233, 99.660, 57.685 }, { 216.393, 99.450, 57.680 }, { 231.608, 99.220, 57.670 },
                        { 246.883, 98.970, 57.660 }, { 262.218, 98.700, 57.655 }, { 277.623, 98.395, 57.645 },
                        { 293.098, 98.080, 57.630 }, { 308.653, 97.740, 57.625 }, { 324.288, 97.375, 57.610 },
                        { 340.008, 96.990, 57.600 } },
                { { -340.003, 96.925, 42.490 }, { -324.283, 97.305, 42.495 }, { -308.648, 97.675, 42.505 },
                        { -293.093, 98.010, 42.510 }, { -277.618, 98.330, 42.510 }, { -262.213, 98.625, 42.515 },
                        { -246.873, 98.900, 42.525 }, { -231.603, 99.155, 42.530 }, { -216.383, 99.385, 42.535 },
                        { -201.223, 99.590, 42.530 }, { -186.113, 99.775, 42.535 }, { -171.048, 99.930, 42.540 },
                        { -156.033, 100.070, 42.545 }, { -141.048, 100.185, 42.545 }, { -126.108, 100.280, 42.550 },
                        { -111.193, 100.350, 42.545 }, { -96.308, 100.400, 42.545 }, { -81.448, 100.420, 42.550 },
                        { -66.608, 100.425, 42.550 }, { -51.788, 100.405, 42.550 }, { -36.978, 100.355, 42.545 },
                        { -22.178, 100.290, 42.545 }, { -7.388, 100.200, 42.545 }, { 7.398, 100.200, 42.545 },
                        { 22.188, 100.290, 42.545 }, { 36.988, 100.355, 42.545 }, { 51.793, 100.405, 42.550 },
                        { 66.613, 100.425, 42.550 }, { 81.458, 100.420, 42.550 }, { 96.318, 100.400, 42.545 },
                        { 111.198, 100.350, 42.545 }, { 126.118, 100.280, 42.550 }, { 141.063, 100.185, 42.545 },
                        { 156.043, 100.070, 42.545 }, { 171.063, 99.930, 42.540 }, { 186.123, 99.775, 42.535 },
                        { 201.233, 99.590, 42.530 }, { 216.393, 99.385, 42.535 }, { 231.608, 99.155, 42.530 },
                        { 246.883, 98.900, 42.525 }, { 262.218, 98.625, 42.515 }, { 277.628, 98.330, 42.510 },
                        { 293.098, 98.010, 42.510 }, { 308.653, 97.675, 42.505 }, { 324.288, 97.305, 42.495 },
                        { 340.008, 96.925, 42.490 } },
                { { -340.003, 96.830, 27.385 }, { -324.278, 97.215, 27.385 }, { -308.648, 97.575, 27.385 },
                        { -293.093, 97.915, 27.385 }, { -277.613, 98.240, 27.385 }, { -262.213, 98.535, 27.385 },
                        { -246.878, 98.810, 27.385 }, { -231.603, 99.060, 27.385 }, { -216.383, 99.290, 27.385 },
                        { -201.223, 99.495, 27.385 }, { -186.113, 99.680, 27.385 }, { -171.048, 99.840, 27.385 },
                        { -156.033, 99.980, 27.385 }, { -141.048, 100.095, 27.385 }, { -126.103, 100.185, 27.385 },
                        { -111.193, 100.255, 27.385 }, { -96.303, 100.305, 27.385 }, { -81.448, 100.330, 27.385 },
                        { -66.608, 100.330, 27.385 }, { -51.783, 100.310, 27.385 }, { -36.973, 100.265, 27.385 },
                        { -22.178, 100.200, 27.385 }, { -7.388, 100.105, 27.385 }, { 7.403, 100.105, 27.385 },
                        { 0.000, 0.000, 0.000 }, { 0.000, 0.000, 0.000 }, { 0.000, 0.000, 0.000 }, { 0.000, 0.000, 0.000 },
                        { 0.000, 0.000, 0.000 }, { 0.000, 0.000, 0.000 }, { 0.000, 0.000, 0.000 }, { 0.000, 0.000, 0.000 },
                        { 0.000, 0.000, 0.000 }, { 156.078, 99.980, 27.385 }, { 171.103, 99.840, 27.385 },
                        { 186.168, 99.680, 27.385 }, { 201.268, 99.495, 27.385 }, { 216.423, 99.290, 27.385 },
                        { 231.638, 99.060, 27.385 }, { 246.913, 98.810, 27.385 }, { 262.248, 98.535, 27.385 },
                        { 277.658, 98.240, 27.385 }, { 293.133, 97.920, 27.385 }, { 308.688, 97.575, 27.385 },
                        { 324.323, 97.215, 27.385 }, { 340.043, 96.830, 27.385 } },
                { { 0.000, 0.000, 0.000 }, { 0.000, 0.000, 0.000 }, { 0.000, 0.000, 0.000 }, { 0.000, 0.000, 0.000 },
                        { 0.000, 0.000, 0.000 }, { 0.000, 0.000, 0.000 }, { 0.000, 0.000, 0.000 }, { 0.000, 0.000, 0.000 },
                        { 0.000, 0.000, 0.000 }, { 0.000, 0.000, 0.000 }, { 0.000, 0.000, 0.000 }, { 0.000, 0.000, 0.000 },
                        { 0.000, 0.000, 0.000 }, { 0.000, 0.000, 0.000 }, { 0.000, 0.000, 0.000 }, { 0.000, 0.000, 0.000 },
                        { 0.000, 0.000, 0.000 }, { 0.000, 0.000, 0.000 }, { 0.000, 0.000, 0.000 }, { 0.000, 0.000, 0.000 },
                        { 0.000, 0.000, 0.000 }, { 0.000, 0.000, 0.000 }, { 0.000, 0.000, 0.000 }, { 0.000, 0.000, 0.000 },
                        { 0.000, 0.000, 0.000 }, { 0.000, 0.000, 0.000 }, { 0.000, 0.000, 0.000 }, { 0.000, 0.000, 0.000 },
                        { 0.000, 0.000, 0.000 }, { 0.000, 0.000, 0.000 }, { 0.000, 0.000, 0.000 }, { 0.000, 0.000, 0.000 },
                        { 0.000, 0.000, 0.000 }, { 0.000, 0.000, 0.000 }, { 0.000, 0.000, 0.000 }, { 0.000, 0.000, 0.000 },
                        { 0.000, 0.000, 0.000 }, { 0.000, 0.000, 0.000 }, { 0.000, 0.000, 0.000 }, { 0.000, 0.000, 0.000 },
                        { 0.000, 0.000, 0.000 }, { 0.000, 0.000, 0.000 }, { 0.000, 0.000, 0.000 }, { 0.000, 0.000, 0.000 },
                        { 0.000, 0.000, 0.000 }, { 0.000, 0.000, 0.000 } },
                { { -339.998, 96.840, -27.330 }, { -324.278, 97.225, -27.340 }, { -308.643, 97.585, -27.345 },
                        { -293.093, 97.925, -27.350 }, { -277.613, 98.245, -27.360 }, { -262.213, 98.545, -27.365 },
                        { -246.868, 98.820, -27.365 }, { -231.598, 99.070, -27.370 }, { -216.383, 99.300, -27.375 },
                        { -201.223, 99.505, -27.380 }, { -186.113, 99.690, -27.385 }, { -171.048, 99.850, -27.380 },
                        { -156.028, 99.990, -27.385 }, { -141.048, 100.100, -27.390 }, { -126.103, 100.195, -27.390 },
                        { -111.193, 100.265, -27.395 }, { -96.303, 100.315, -27.395 }, { -81.443, 100.340, -27.390 },
                        { -66.603, 100.335, -27.390 }, { -51.783, 100.315, -27.390 }, { -36.973, 100.275, -27.395 },
                        { -22.173, 100.205, -27.390 }, { -7.383, 100.115, -27.385 }, { 7.403, 100.115, -27.385 },
                        { 0.000, 0.000, 0.000 }, { 0.000, 0.000, 0.000 }, { 0.000, 0.000, 0.000 }, { 0.000, 0.000, 0.000 },
                        { 0.000, 0.000, 0.000 }, { 0.000, 0.000, 0.000 }, { 0.000, 0.000, 0.000 }, { 0.000, 0.000, 0.000 },
                        { 0.000, 0.000, 0.000 }, { 156.088, 99.985, -27.385 }, { 171.103, 99.845, -27.380 },
                        { 186.168, 99.680, -27.385 }, { 201.268, 99.495, -27.380 }, { 216.428, 99.290, -27.375 },
                        { 231.643, 99.060, -27.370 }, { 246.913, 98.810, -27.365 }, { 262.258, 98.535, -27.365 },
                        { 277.658, 98.240, -27.360 }, { 293.138, 97.925, -27.350 }, { 308.688, 97.580, -27.345 },
                        { 324.323, 97.215, -27.340 }, { 340.043, 96.835, -27.330 } },
                { { -339.998, 96.930, -42.435 }, { -324.278, 97.315, -42.445 }, { -308.648, 97.680, -42.455 },
                        { -293.093, 98.015, -42.470 }, { -277.613, 98.340, -42.480 }, { -262.208, 98.635, -42.490 },
                        { -246.873, 98.910, -42.500 }, { -231.593, 99.160, -42.510 }, { -216.383, 99.390, -42.515 },
                        { -201.223, 99.595, -42.525 }, { -186.113, 99.780, -42.525 }, { -171.048, 99.940, -42.535 },
                        { -156.028, 100.080, -42.540 }, { -141.048, 100.195, -42.540 }, { -126.103, 100.290, -42.545 },
                        { -111.193, 100.355, -42.550 }, { -96.303, 100.405, -42.550 }, { -81.443, 100.430, -42.550 },
                        { -66.608, 100.430, -42.550 }, { -51.783, 100.405, -42.550 }, { -36.973, 100.365, -42.550 },
                        { -22.178, 100.295, -42.545 }, { -7.388, 100.205, -42.545 }, { 7.403, 100.205, -42.545 },
                        { 22.193, 100.295, -42.545 }, { 36.988, 100.365, -42.550 }, { 51.798, 100.405, -42.550 },
                        { 66.623, 100.430, -42.550 }, { 81.458, 100.430, -42.550 }, { 96.318, 100.405, -42.550 },
                        { 111.208, 100.355, -42.550 }, { 126.118, 100.290, -42.545 }, { 141.063, 100.195, -42.540 },
                        { 156.043, 100.080, -42.540 }, { 171.063, 99.940, -42.535 }, { 186.128, 99.780, -42.525 },
                        { 201.238, 99.595, -42.525 }, { 216.398, 99.390, -42.515 }, { 231.613, 99.160, -42.510 },
                        { 246.888, 98.910, -42.500 }, { 262.223, 98.635, -42.490 }, { 277.628, 98.340, -42.480 },
                        { 293.108, 98.015, -42.470 }, { 308.663, 97.680, -42.455 }, { 324.293, 97.315, -42.445 },
                        { 340.013, 96.930, -42.435 } },
                { { -339.998, 97.000, -57.540 }, { -324.278, 97.385, -57.560 }, { -308.648, 97.745, -57.575 },
                        { -293.093, 98.090, -57.595 }, { -277.613, 98.410, -57.610 }, { -262.208, 98.705, -57.625 },
                        { -246.873, 98.975, -57.640 }, { -231.593, 99.225, -57.655 }, { -216.383, 99.455, -57.665 },
                        { -201.223, 99.665, -57.675 }, { -186.113, 99.845, -57.685 }, { -171.048, 100.010, -57.690 },
                        { -156.028, 100.145, -57.700 }, { -141.048, 100.265, -57.705 }, { -126.103, 100.355, -57.710 },
                        { -111.193, 100.425, -57.710 }, { -96.303, 100.475, -57.720 }, { -81.443, 100.495, -57.715 },
                        { -66.608, 100.500, -57.720 }, { -51.783, 100.480, -57.715 }, { -36.973, 100.430, -57.710 },
                        { -22.178, 100.365, -57.710 }, { -7.388, 100.275, -57.705 }, { 7.403, 100.275, -57.705 },
                        { 22.193, 100.365, -57.710 }, { 36.988, 100.430, -57.710 }, { 51.798, 100.480, -57.715 },
                        { 66.623, 100.500, -57.720 }, { 81.458, 100.495, -57.715 }, { 96.318, 100.475, -57.720 },
                        { 111.208, 100.425, -57.710 }, { 126.118, 100.355, -57.710 }, { 141.063, 100.265, -57.705 },
                        { 156.043, 100.145, -57.700 }, { 171.063, 100.010, -57.690 }, { 186.128, 99.845, -57.685 },
                        { 201.238, 99.665, -57.675 }, { 216.398, 99.455, -57.665 }, { 231.613, 99.225, -57.655 },
                        { 246.888, 98.975, -57.640 }, { 262.223, 98.705, -57.625 }, { 277.628, 98.410, -57.610 },
                        { 293.108, 98.090, -57.595 }, { 308.663, 97.745, -57.575 }, { 324.293, 97.385, -57.560 },
                        { 340.013, 97.000, -57.540 } },
                { { -339.998, 97.045, -72.655 }, { -324.278, 97.435, -72.680 }, { -308.648, 97.795, -72.710 },
                        { -293.093, 98.135, -72.730 }, { -277.613, 98.455, -72.750 }, { -262.208, 98.750, -72.775 },
                        { -246.873, 99.020, -72.795 }, { -231.593, 99.280, -72.810 }, { -216.383, 99.505, -72.820 },
                        { -201.223, 99.710, -72.840 }, { -186.113, 99.895, -72.850 }, { -171.048, 100.055, -72.860 },
                        { -156.028, 100.190, -72.870 }, { -141.048, 100.305, -72.880 }, { -126.103, 100.400, -72.885 },
                        { -111.193, 100.470, -72.890 }, { -96.303, 100.520, -72.890 }, { -81.443, 100.540, -72.895 },
                        { -66.608, 100.540, -72.895 }, { -51.783, 100.520, -72.895 }, { -36.973, 100.480, -72.890 },
                        { -22.178, 100.405, -72.885 }, { -7.388, 100.320, -72.880 }, { 7.403, 100.320, -72.880 },
                        { 22.193, 100.405, -72.885 }, { 36.988, 100.480, -72.890 }, { 51.798, 100.520, -72.895 },
                        { 66.623, 100.540, -72.895 }, { 81.458, 100.540, -72.895 }, { 96.318, 100.520, -72.890 },
                        { 111.208, 100.470, -72.890 }, { 126.118, 100.400, -72.885 }, { 141.063, 100.305, -72.880 },
                        { 156.043, 100.190, -72.870 }, { 171.063, 100.055, -72.860 }, { 186.128, 99.895, -72.850 },
                        { 201.238, 99.710, -72.840 }, { 216.398, 99.505, -72.820 }, { 231.613, 99.280, -72.810 },
                        { 246.888, 99.020, -72.795 }, { 262.223, 98.750, -72.775 }, { 277.628, 98.455, -72.750 },
                        { 293.108, 98.135, -72.730 }, { 308.663, 97.795, -72.710 }, { 324.293, 97.435, -72.680 },
                        { 340.013, 97.045, -72.655 } },
                { { -339.998, 97.070, -87.790 }, { -324.278, 97.460, -87.820 }, { -308.648, 97.820, -87.850 },
                        { -293.093, 98.160, -87.885 }, { -277.613, 98.480, -87.910 }, { -262.208, 98.775, -87.935 },
                        { -246.873, 99.050, -87.960 }, { -231.593, 99.300, -87.980 }, { -216.383, 99.530, -88.000 },
                        { -201.223, 99.735, -88.015 }, { -186.113, 99.920, -88.030 }, { -171.048, 100.080, -88.045 },
                        { -156.028, 100.215, -88.055 }, { -141.048, 100.335, -88.065 }, { -126.103, 100.420, -88.070 },
                        { -111.193, 100.490, -88.075 }, { -96.303, 100.540, -88.085 }, { -81.443, 100.565, -88.085 },
                        { -66.608, 100.560, -88.085 }, { -51.783, 100.540, -88.085 }, { -36.973, 100.500, -88.080 },
                        { -22.178, 100.430, -88.075 }, { -7.388, 100.340, -88.065 }, { 7.403, 100.340, -88.070 },
                        { 22.193, 100.430, -88.075 }, { 36.988, 100.500, -88.080 }, { 51.798, 100.540, -88.085 },
                        { 66.623, 100.560, -88.085 }, { 81.458, 100.565, -88.085 }, { 96.318, 100.540, -88.085 },
                        { 111.208, 100.490, -88.075 }, { 126.118, 100.420, -88.070 }, { 141.063, 100.335, -88.065 },
                        { 156.043, 100.215, -88.055 }, { 171.063, 100.080, -88.045 }, { 186.128, 99.915, -88.030 },
                        { 201.238, 99.735, -88.015 }, { 216.398, 99.530, -88.000 }, { 231.613, 99.300, -87.980 },
                        { 246.888, 99.050, -87.960 }, { 262.223, 98.775, -87.935 }, { 277.628, 98.480, -87.910 },
                        { 293.108, 98.160, -87.885 }, { 308.663, 97.820, -87.850 }, { 324.293, 97.460, -87.820 },
                        { 340.013, 97.070, -87.790 } } };

};

#endif
