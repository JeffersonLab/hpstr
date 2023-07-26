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
        std::cout<<"Substring does not exist in the string: "+cutname << std::endl;
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

    //Some events are not defined in a given cut variable, and should not be cut in these cases.
    //These events are assigned a specific double value that is otherwise impossible to see.
    //If this value is encountered for a given event, do not apply cut to the event.
    if(val == skipCutVarValue_)
        return true;
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
        std::cout << it->first << ": " << it->second.first << std::endl;
}

void IterativeCutSelector::filterCuts(std::vector<std::string> cut_variable_list){

    //Loop over Test Cuts loaded in from json configuration
    //for(std::map<std::string, std::pair<double,int>>::iterator it = cuts_ptr->begin(); it != cuts_ptr->end(); it++){ 
    for(std::map<std::string, std::pair<double,int>>::iterator it = getPointerToCuts()->begin(); 
            it != getPointerToCuts()->end();){ 
        std::string cutname = it->first;
        std::string cutvariable = getCutVar(cutname);
        bool found = false;

        //Confirm that cut variable is in list of configurable cut variables
        for(std::vector<std::string>::iterator iit=cut_variable_list.begin(); iit !=cut_variable_list.end(); iit++){
            if((std::string)*iit == cutvariable){
                found = true;
                break;
            }
        }

        //If Test Cut Variable does not exist, remove the Test Cut from the list of cuts
        if(!found){
            it = getPointerToCuts()->erase(it);
        }
        else
            ++it;
    }
}

