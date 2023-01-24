#ifndef ZBIHISTOS_H
#define ZBIHISTOS_H

#include "TFile.h"
#include "HistoManager.h"
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

        void addHistoClone1d(TH1F* parentHisto, std::string clone_histoname);

        void addHisto1d(std::string histoname, std::string xtitle, int nbinsX, float xmin, float xmax);

        void printHistos1d(){
            std::cout << "Printing 1d histos" << std::endl;
            for(it1d it = histos1d.begin(); it != histos1d.end(); it++)
                std::cout << it->first << std::endl;

        }

        /**
         * @brief Set debug
         * 
         * @param value 
         */
        void setDebug(bool value){debug_ = value;};

        //void iterativeSignalCuts(ZBiHistos *zbiHistos, IterativeCutSelector *cutSelector);
        double cutFractionOfIntegral(std::string histoname, bool isCutGreaterThan, double cutFraction, double initialIntegral);
        double getIntegral(std::string histoname);

        double fitZTail(std::string zVtxHistoname, double max_tail_events);
};

#endif
