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

        void eraseCut(std::string cutname);

        bool passCutGTorLT(std::string cutname, double val);

        void printCuts();

        std::map<std::string, std::pair<double,int>>* getPointerToCuts(){ return &cuts; }


    private:
};

#endif
