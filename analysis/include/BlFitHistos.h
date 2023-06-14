#ifndef BLFITHISTOS_H
#define BLFITHISTOS_H

#include "TFile.h"
#include "HistoManager.h"
#include "TGraphErrors.h"
#include "TKey.h"
#include "TList.h"
#include "TH1.h"
#include "TrackerHit.h"
#include "RawSvtHit.h"
#include "HistoManager.h"
#include "ModuleMapper.h"
#include "TF1.h"
#include <string>
#include "FlatTupleMaker.h"
#include <math.h>
#include "TCanvas.h"
#include <vector>
#include "ModuleMapper.h"
#include "TFitResult.h"
#include "TF1.h"
#include <fstream>

/**
 * @brief description
 *
 * more details
 */
class BlFitHistos : public HistoManager{

    public:
        BlFitHistos(int year);
        ~BlFitHistos();

        /**
         * @brief description
         *
         * @return std::map<std::string,TH1F*>
         */
        std::map<std::string,TH1F*> get1dHistos() {
            return histos1d;
        }

        /**
         * @brief description
         *
         * @return std::map<std::string,TH2F*>
         */
        std::map<std::string,TH2F*> get2dHistos() {
            return histos2d;
        }

        /**
         * @brief Get the Histos From File object
         *
         * @param inFile
         * @param layer
         */
        void getHistosFromFile(TFile* inFile, std::string layer = "");

        /**
         * @brief description
         * 
         * @param histos2d 
         * @param rebin_ 
         * @param minStats_ 
         * @param deadRMS_ 
         * @param thresholdsFileIn_ 
         * @param flat_tuple_ 
         */
        void fit2DHistoChannelBaselines(std::map<std::string, TH2F*> histos2d,int rebin_, int minStats_,int deadRMS_, std::string thresholdsFileIn_, FlatTupleMaker* flat_tuple_);

        /**
         * @brief description
         * 
         * @param hist 
         * @param min 
         * @param max 
         * @param sigmaRange 
         * @param hardminimum 
         * @param hardmaximum 
         */
        void iterativeGausFit(TH1D* hist, double min, double max, double sigmaRange, double hardminimum, double hardmaximum);

        /**
         * @brief Set debug
         * 
         * @param value 
         */
        void setDebug(bool value){debug_ = value;};

         /**
         * @brief description
         * 
         * @param hist 
         * @param xmin 
         * @param xmax 
         */
        void backwardsIterChi2Fit(TH1D* hist, double xmin, double xmax);

    private:

        TH1F* fitHistos{nullptr}; //!< description

    protected:
        std::map<std::string, TH2F*> histos2d; //!< description
        std::map<std::string, TH1F*> histos1d; //!< description
        std::map<std::string,std::map<int,int>> svtIDMap; //!< description
        std::map<std::string,std::map<std::string,std::vector<int>>> threshMap_; //!< description
        ModuleMapper * mmapper_; //!< description
        bool debug_{false}; //!< description
};

#endif
