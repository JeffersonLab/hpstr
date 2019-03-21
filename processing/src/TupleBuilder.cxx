/**
 * @file TupleBuilder.h
 * @brief Builder class to build ROOT ntuples.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "TupleBuilder.h"

//----------//
//   ROOT   //
//----------//
#include "TTree.h"

TupleBuilder::TupleBuilder(TTree* tree) : 
    tree_(tree) {
}

/*
TTree* TupleBuilder::add(std::string name, Type type) { 

    switch(type) { 
        case Short:  variables_[name] = short(-9999); break;
        case Int:    variables_[name] = int(-9999); break;
        case Float:  variables_[name] = float(-9999); break;
        case Double: variables_[name] = double(-9999); break;
        case Long:   variables_[name] = long(-9999); break;
        case Bool:   variables_[name] = bool(0); break;
    }
  
    std::cout << "Old: " << variables_[name] << std::endl; 
    std::cout << "Old A: " << &variables_[name] << std::endl; 
    tree_->Branch(name.c_str(), &variables_[name], 
            (name + "/" + type_string_[type]).c_str()); 

    return tree_; 
}*/
