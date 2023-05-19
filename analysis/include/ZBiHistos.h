#ifndef ZBIHISTOS_H
#define ZBIHISTOS_H

#include "TFile.h"
#include "HistoManager.h"
#include "TGraph.h"
#include "TKey.h"
#include "TH1.h"
#include "TH1F.h"
#include "TrackerHit.h"
#include "TF1.h"
#include <string>
#include "FlatTupleMaker.h"
#include <math.h>
#include "TCanvas.h"
#include <vector>
#include "TFitResult.h"
#include "TFitResultPtr.h"
#include "TF1.h"
#include <fstream>
#include "TRandom3.h"
#include "IterativeCutSelector.h"
#include "MutableTTree.h"

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
        std::map<std::string, TH1F*> get1dHistos() {
            return histos1d;
        }

        void setHistogramTitle1D(std::string histoname, std::string title);
    
        void writeHistos(TFile* outF, std::string folder);

        void resetHistograms1d();

        void resetHistograms2d();

        void addHisto1d(std::string histoname, std::string xtitle, int nbinsX, float xmin, float xmax);

        void addHisto2d(std::string histoname, std::string xtitle, int nbinsX, float xmin, float xmax, std::string ytitle, int nbinsY, float ymin, float ymax);

        void set2DHistoYlabel(std::string histoName, int ybin, std::string ylabel);

        std::vector<double> defineImpactParameterCut(double alpha = 0.15);

        void defineTestCutHistograms(IterativeCutSelector* testCutsSelector);

        void defineZBiCutflowProcessorHistograms();

        void fillEventVariableHistograms(MutableTTree* MTT);

        void printHistos1d(){
            std::cout << "Printing 1d histos" << std::endl;
            for(it1d it = histos1d.begin(); it != histos1d.end(); it++)
                std::cout << it->first << std::endl;
        }

        void printHistos2d(){
            std::cout << "Printing 2d histos" << std::endl;
            for(it2d it = histos2d.begin(); it != histos2d.end(); it++)
                std::cout << it->first << std::endl;
        }

        void defineIterHistos();

        /**
         * @brief Set debug
         * 
         * @param value 
         */
        void setDebug(bool value){debug_ = value;};

        //void iterativeSignalCuts(ZBiHistos *zbiHistos, IterativeCutSelector *cutSelector);
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
