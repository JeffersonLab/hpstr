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

    /*
    if(isCutGT){
        cutvar = cutname.erase(std::remove(cutname.begin(), cutname.end(), "_gt"), cutname.end()); 
    }
    else
        cutvar = cutname.erase(std::remove(cutname.begin(), cutname.end(), "_lt"), cutname.end()); 
        */
    return cutvar;
}
