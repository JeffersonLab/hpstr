/*
 * @file FlatTupleMaker.h
 * @author Omar Moreno
 * @date January 18, 2016
 * @author PF
 * @date Jan, 2020
 * @brief 
 *
 */

#ifndef __FLAT_TUPLE_MAKER_H__
#define __FLAT_TUPLE_MAKER_H__

//----------------//   
//   C++ StdLib   //
//----------------//   
#include <string>
#include <map>
#include <iostream>

//----------//
//   ROOT   //
//----------//
#include <TFile.h>
#include <TTree.h>

class FlatTupleMaker {

    public:

        /** 
         * Constructor
         *  
         */
        FlatTupleMaker(std::string file_name, std::string tree_name);
        
        FlatTupleMaker(std::string tree_name);

        /** Destructor */
        ~FlatTupleMaker();

        /**
         *
         */
        void addVariable(std::string variable_name);

        void addVector(std::string vector_name); 

        /**
         *
         */
        void setVariableValue(std::string variable_name, double value) { variables[variable_name] = value; }; 

        void addToVector(std::string variable_name, double value); 

        /**
         *
         */
        bool hasVariable(std::string variable_name); 

        /**
         *
         */
        void close();

        std::vector<double> getVector(std::string variable_name);

        /** 
         *
         */ 
        void writeTree() {
            tree->Write();
        }
        
        /**
         *
         */
        void fill();

    private: 
    
        /** ROOT file to write ntuple to. */
        TFile* file{nullptr};

        /** ROOT Tree. */
        TTree* tree{nullptr}; 

        /** Map containing ntuple variables */
        std::map <std::string, double> variables; 

        std::map <std::string, std::vector<double>> vectors; 
        

};

#endif // __FLAT_TUPLE_MAKER_H__
