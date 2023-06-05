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

        std::string getCutVar(std::string cutname);
        
        bool isCutGreaterThan(std::string cutname);
        
        void setCutValue(std::string cutname, double value);

        bool passCutGTorLT(std::string cutname, double val);

        void printCuts();

        int getCutID(std::string cutname){return cuts[cutname].second;};

        void filterCuts(std::vector<std::string> cut_variable_list);

        std::map<std::string, std::pair<double,int>>* getPointerToCuts(){ return &cuts; }


    private:
        //When this variable value is encountered, do not apply cut to event
        double skipCutVarValue_ = -9876543210.0; //Must match definition in MutableTTree
};

#endif
