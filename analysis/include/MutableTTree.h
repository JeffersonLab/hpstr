#ifndef MUTABLE_TTREE_H
#define MUTABLE_TTREE_H

#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <TTree.h>
#include <TFile.h>
#include <TBranch.h>
#include <functional>

/**
 * @brief Reads flat TTree and allows user to create new variables in the TTree
 */
class MutableTTree {

    public:

        MutableTTree(TFile* infile, std::string tree_name);

        /** 
         * @brief return number of entries in tree
         */
        int GetEntries(){return tree_->GetEntries();}

        /** 
         * @brief get tree entry
         * @param entry
         */
        void GetEntry(int entry){tree_->GetEntry(entry);}

        /** 
         * @brief Fill ttree with new variables included
         */
        void Fill();

        /** 
         * @brief Apply any corrections to specified variable
         * @param variable
         * @param correction/shift
         */
        void shiftVariable(std::string variable, double shift);

        /** 
         * @brief Get the value of a flat tuple variable
         * @param branch_name
         * @return value
         */
        double getValue(std::string branch_name);

        /** 
         * @brief Print TTree Event
         */
        void printEvent();

        /** 
         * @brief Set branch value
         * @param branch_name
         * @param value
         */
        void setBranchValue(std::string branch_name, double value){*tuple_[branch_name] = value;}

        /** 
         * @brief Add new branch to ttree
         * @param branch
         */
        void addNewBranch(std::string branch);

        /** 
         * @brief Set the mass window within which to read the input ttree
         * @param lowMass
         * @param highMass
         */
        void defineMassWindow(double lowMass, double highMass);

        /** 
         * @brief Get list of all variables defined in ttree
         */
        std::vector<std::string> getAllVariables();

        /** 
         * @brief Check if a variable exists in the ttree
         * @param variable
         * @return true
         * @return false
         */
        bool variableExists(std::string variable);

        ~MutableTTree();

    protected:
        TTree* tree_{nullptr}; //!< flat ttree
        TTree* newtree_{nullptr}; //!< temporary ttree used to create and fill new branches
        std::map<std::string,double*> tuple_; //!< holds all variables and values
        std::map<std::string,TBranch*> new_branches; //!< list of new branches added to ttree
        std::map<std::string, double*> new_variables_;//!< list of new variables
        std::map<std::string,std::function<double()>> functions_;//!< functions that calculate new variables
        std::map<std::string,std::function<double()>> variable_shifts_;//!< variable corrections

    private:
        /** 
         * @brief read in the initial flat TTree
         * @param tree
         * @param tuple_map
         */
        void initializeFlatTuple(TTree* tree, std::map<std::string, double*> &tuple_map);

        /** 
         * @brief copy the TTree
         */
        void copyTTree();

        double lowMass_{-999.9};//!< mass window low
        double highMass_{-999.9};//!< mass window high
};

#endif // __MUTABLE_TTREE_H
