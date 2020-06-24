/*
 * @file FlatTupleMaker.cxx
 * @author Omar Moreno
 * @date January 18, 2016
 * @author PF
 * @date Jan, 2020
 * @brief 
 *
 */

#include <FlatTupleMaker.h>

FlatTupleMaker::FlatTupleMaker(std::string file_name, std::string tree_name) { 
    
    file = new TFile(file_name.c_str(), "RECREATE");

    tree = new TTree(tree_name.c_str(), tree_name.c_str());   
    
}

//Will save it in current open file
FlatTupleMaker::FlatTupleMaker(std::string tree_name) { 
    file = nullptr;
    tree = new TTree(tree_name.c_str(), tree_name.c_str());
}

FlatTupleMaker::~FlatTupleMaker() { 
    if (file)
        delete file; 
    if (tree)
        delete tree; 
}

void FlatTupleMaker::addVariable(std::string variable_name) { 
    
    // Set the default value of the variable to something unrealistic 
    variables[variable_name] = -9999;
    
    // Add a leaf to the ROOT tree and set its address to the address of the 
    // newly created variable. 
    tree->Branch(variable_name.c_str(), &variables[variable_name], (variable_name + "/D").c_str()); 
}

void FlatTupleMaker::addVector(std::string variable_name) { 
    vectors[variable_name] = {}; 
    tree->Branch(variable_name.c_str(), &vectors[variable_name]); 
}

void FlatTupleMaker::addToVector(std::string variable_name, double value) {
    vectors[variable_name].push_back(value); 
}

bool FlatTupleMaker::hasVariable(std::string variable_name) { 
    
    auto search = variables.find(variable_name); 
    if (search != variables.end()) return true; 

    return false; 
}

void FlatTupleMaker::close() { 
    if (file) {
        file->Write();
        file->Close(); 
    }
}

std::vector<double> FlatTupleMaker::getVector(std::string variable_name) {
    return vectors[variable_name];
}

void FlatTupleMaker::fill() { 
    
    // Fill the event with the current variables.
    tree->Fill();

    // Reset the variables to their original values
    for (auto& element : variables) { 
        element.second = -9999; 
    }
    
    for (auto& element : vectors) { 
        element.second.clear();
    }
}
