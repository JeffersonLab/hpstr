#ifndef ITERATIVECUTSELECTOR_H
#define ITERATIVECUTSELECTOR_H

#include <string>
#include <iostream>
#include <map>
#include <memory>
#include "BaseSelector.h"

#include "TH1F.h"
#include "json.hpp"

// for convenience
using json = nlohmann::json;

/**
 * @brief brief description
 * 
 * more details
 */
class IterativeCutSelector : public BaseSelector { 

    public: 

        IterativeCutSelector();
        
        IterativeCutSelector(const std::string& inputName);
        
        IterativeCutSelector(const std::string& inputName, const std::string& cfgFile);
        
        virtual ~IterativeCutSelector();

        /**
         * @brief get cut variable from name
         *
         * @param cutname
         */
        std::string getCutVar(std::string cutname);
        
        /**
         * @brief is cut of type 'greater than'
         *
         * @param cutname
         * @return true
         * @return false
         */
        bool isCutGreaterThan(std::string cutname);
        
        /**
         * @brief set cut value
         *
         * @param cutname
         * @param value
         */
        void setCutValue(std::string cutname, double value);

        /**
         * @brief does value pass cut
         *
         * @param cutname
         * @param val
         * @return true
         * @return false
         */
        bool passCutGTorLT(std::string cutname, double val);

        /**
         * @brief prints cuts and values
         *
         */
        void printCuts();

        /**
         * @brief get cut ID
         * @param cutname
         * @return cut ID
         *
         */
        int getCutID(std::string cutname){return cuts[cutname].second;};

        /**
         * @brief remove cuts that aren't specified in the list of cut
         * variables
         * @param cut_variables_list
         *
         */
        void filterCuts(std::vector<std::string> cut_variable_list);

        /**
         * @brief get pointer to the base class cuts
         *
         */
        std::map<std::string, std::pair<double,int>>* getPointerToCuts(){ return &cuts; }


    private:
        //When this variable value is encountered, do not apply cut to event
        double skipCutVarValue_ = -9876543210.0; //Must match definition in MutableTTree
};

#endif
