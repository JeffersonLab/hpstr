#ifndef ZBIHISTOS_H
#define ZBIHISTOS_H

#include "TFile.h"
#include "HistoManager.h"
#include "TGraph.h"
#include "TKey.h"
#include "TH1.h"
#include "TH1F.h"
#include "TF1.h"
#include "TCanvas.h"
#include "TFitResult.h"
#include "TFitResultPtr.h"
#include "TF1.h"
#include "TRandom3.h"
#include "MutableTTree.h"
#include <vector>
#include <fstream>
#include <string>
#include <math.h>

/**
 * @brief description
 * 
 * more details
 */
class ZBiHistos : public HistoManager{

    public:
        ZBiHistos();
        ~ZBiHistos();

        ZBiHistos(const std::string& inputName);
        /**
         * @brief description
         * 
         */
        virtual void Define3DHistos(){};

        /**
         * @brief description
         * 
         */
        virtual void Define2DHistos(){};

        /**
         * @brief description
         * 
         */

        /**
         * @brief description
         * 
         * @return std::map<std::string,TH1F*> 
         */

        void writeHistos(TFile* outF, std::string folder);

        void resetHistograms1d();

        void resetHistograms2d();

        void addHisto1d(std::string histoname, std::string xtitle, int nbinsX, float xmin, float xmax);

        void addHisto2d(std::string histoname, std::string xtitle, int nbinsX, float xmin, float xmax, std::string ytitle, int nbinsY, float ymin, float ymax);

        void set2DHistoYlabel(std::string histoName, int ybin, std::string ylabel);

        std::vector<double> defineImpactParameterCut(double alpha = 0.15);

        void defineZBiCutflowProcessorHistograms();


        /**
         * @brief Set debug
         * 
         * @param value 
         */
        void setDebug(bool value){debug_ = value;};

        double cutFractionOfSignalVariable(std::string cutvariable, bool isCutGreaterThan, double cutFraction, double initialIntegral);

        double integrateHistogram1D(std::string histoname);

        TF1* fitExponentialTail(std::string histogramName, double start_nevents);

        double fitZTail(std::string zVtxHistoname, double max_tail_events);

        double shosFitZTail(std::string cutname, double max_tail_events);

        TF1* getZTailFit(std::string cutname);

        void writeGraphs(TFile* outF, std::string folder);

    private:
        std::map<std::string, TGraph*> graphs_;
};

#endif
