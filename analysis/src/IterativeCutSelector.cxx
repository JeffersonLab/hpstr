#include "IterativeCutSelector.h"
#include <fstream>
#include <iostream>
#include <algorithm> 

IterativeCutSelector::IterativeCutSelector():BaseSelector() { 
}

IterativeCutSelector::IterativeCutSelector(const std::string& inputName):BaseSelector(inputName) { 
}

IterativeCutSelector::IterativeCutSelector(const std::string& inputName, const std::string& cfgFile):BaseSelector(inputName,cfgFile) { 
}

IterativeCutSelector::~IterativeCutSelector(){}

bool IterativeCutSelector::isCutGreaterThan(std::string cutname){
    if(cutname.find("_gt") != std::string::npos){
        return true;
    }
    else
        return false;
}

std::string IterativeCutSelector::getCutVar(std::string cutname){

    std::string substr;
    bool isCutGT = isCutGreaterThan(cutname);
    std::string cutvar;

    if(isCutGT) substr = "_gt";
    else substr = "_lt";

    std::size_t ind = cutname.find(substr); // Find the starting position of substring in the string
    if(ind !=std::string::npos){
        cutvar = cutname.erase(ind,substr.length()); // erase function takes two parameter, the starting index in the string from where you want to erase characters and total no of characters you want to erase.
    }else{
        std::cout<<"Substring does not exist in the string\n";
    }

    return cutvar;
}

void IterativeCutSelector::setCutValue(std::string cutname, double value){
    std::pair<double,int> pair = cuts[cutname];
    double ogval = pair.first;
    int id = pair.second;
    pair.first = value;
    pair.second = id;
    cuts[cutname] = pair;
    if(debug_)
        std::cout << "[IterativeCutSelector] Updating cut " << cutname << " value from " << ogval << " to: " << cuts[cutname].first << std::endl; 
}

bool IterativeCutSelector::passCutGTorLT(std::string cutname, double val){

    if(isCutGreaterThan(cutname)){
        if (hasCut(cutname)) {
            if (val < cuts[cutname].first) {
                return false;
            }
        }
        return true;
    }

    else{
        if (hasCut(cutname)) {
            if (val > cuts[cutname].first) {
                return false;
            }
        }
        return true; 
    }
}

void IterativeCutSelector::printCuts(){
    for(cut_it it=cuts.begin(); it != cuts.end(); it++)
        std::cout << it->first << std::endl;
}

void IterativeCutSelector::eraseCut(std::string cutname){
    std::cout << "Erasing " << cutname << " from list of cuts" << std::endl;
    cuts.erase(cutname);
}
