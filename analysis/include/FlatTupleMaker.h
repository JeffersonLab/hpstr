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

/**
 * @brief description
 * 
 * details
 */
class FlatTupleMaker {

    public:
        /**
         * @brief Constructor
         * 
         * @param file_name 
         * @param tree_name 
         */
        FlatTupleMaker(std::string file_name, std::string tree_name);
        
        /**
         * @brief Constructor
         * 
         * @param tree_name 
         */
        FlatTupleMaker(std::string tree_name);

        /** Destructor */
        ~FlatTupleMaker();

        /**
         * @brief description
         * 
         * @param variable_name 
         */
        void addVariable(std::string variable_name);

        /**
         * @brief description
         * 
         * @param variable_name 
         */
        void addString(std::string variable_name);

        /**
         * @brief description
         * 
         * @param vector_name 
         */
        void addVector(std::string vector_name);

        /**
         * @brief description
         * 
         * @param variable_name 
         * @param value 
         */
        void setVariableValue(std::string variable_name, double value) { variables[variable_name] = value; };

        /**
         * @brief description
         * 
         * @param variable_name 
         * @param value 
         */
        void setVariableValue(std::string variable_name, std::string value) { string_variables[variable_name] = value; };

        /**
         * @brief description
         * 
         * @param variable_name 
         * @param value 
         */
        void addToVector(std::string variable_name, double value); 

        /**
         * @brief description
         * 
         * @param variable_name 
         * @return true 
         * @return false 
         */
        bool hasVariable(std::string variable_name); 

        /**
         * @brief description
         * 
         */
        void close();

        /**
         * @brief description
         * 
         * @param variable_name 
         * @return std::vector<double> 
         */
        std::vector<double> getVector(std::string variable_name);

        /**
         * @brief Write root tree
         * 
         */
        void writeTree() {
            tree->Write();
        }
        
        /**
         * @brief description
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

        /** description */
        std::map <std::string, std::string> string_variables; 

        /** description */
        std::map <std::string, std::vector<double>> vectors; 
        

};

#endif // __FLAT_TUPLE_MAKER_H__
