#ifndef CLUSTERHISTOS_H
#define CLUSTERHISTOS_H

#include "TFile.h"
#include "HistoManager.h"
#include "TGraphErrors.h"
#include "TKey.h"
#include "TList.h"

#include "TrackerHit.h"
#include "RawSvtHit.h"

#include "ModuleMapper.h"

#include <string>


class ClusterHistos : public HistoManager{
  
    public:
        /**
         * @brief Constructor
         * 
         * @param inputName 
         */
        ClusterHistos(const std::string& inputName);
        ~ClusterHistos();

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
        virtual void Define1DHistos();

        //virtual void GetHistosFromFile(TFile* inFile, const std::string& name,const std::string& folder="");

        /**
         * @brief description
         * 
         * @param hit 
         * @param weight 
         */
        void FillHistograms(TrackerHit* hit, float weight = 1.);
        //void BuildAxesMap();
        
        /**
         * @brief Set the baseline fits directories
         * 
         * @param baselineFits 
         */
        void setBaselineFitsDir(const std::string& baselineFits) { baselineFits_ = baselineFits; };

        /**
         * @brief Load baseline histograms
         * 
         * @param baselineRun 
         * @return true 
         * @return false 
         */
        bool LoadBaselineHistos(const std::string& baselineRun);

        /**
         * @brief Load offline baseline
         * 
         * @param baselineFits 
         * @return true 
         * @return false 
         */
        bool LoadOfflineBaselines(const std::string& baselineFits);
        
        //void setBaselineFits(const std::string& baselineFits){baselineFits_ = baselineFits;};
        //std::string getBaselineFits const () {return baselineFits_;};
  

    private:
        /** description */
        std::vector<std::string> variables{"charge", "cluSize"};
        
        /** description */
        std::vector<std::string> half_module_names{};

        /** description */
        std::map<std::string, int>    cluSizeMap;

        /** description */
        std::map<std::string, double> chargeMap;

        /** description */
        std::map<std::string, double> chargeCorrectedMap;

        /** description */
        std::map<std::string, double> cluPositionMap;
        
        /** description */
        std::string baselineFits_{"/nfs/hps3/svtTests/jlabSystem/baselines/fits/"};

        /** description */
        std::string baselineRun_{""};

        /** description */
        std::map<std::string, TGraphErrors*> baselineGraphs;

        /** 
         * description 
         * \todo clean this up
         */
        ModuleMapper *mmapper_{nullptr};
};


#endif
