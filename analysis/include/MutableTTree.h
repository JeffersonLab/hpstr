#ifndef MUTABLE_TTREE_H
#define MUTABLE_TTREE_H

#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <TTree.h>
#include <TFile.h>
#include <TBranch.h>

class MutableTTree {

    public:

        MutableTTree(TFile* infile, std::string tree_name);

        int GetEntries(){return tree_->GetEntries();}

        void GetEntry(int entry){tree_->GetEntry(entry);}

        void Fill();

        //Apply corrections
        void shiftVariable(std::string variable, double shift);

        double getValue(std::string branch_name);

        void printEvent();

        void setBranchValue(std::string branch_name, double value){*tuple_[branch_name] = value;}

        void addNewBranch(std::string branch);

        void defineMassWindow(double lowMass, double highMass);

        std::vector<std::string> getAllVariables();

        bool variableExists(std::string variable);

        ~MutableTTree();

    protected:
        TTree* tree_{nullptr};
        TTree* newtree_{nullptr};
        std::map<std::string,double*> tuple_;
        std::map<std::string,TBranch*> new_branches;
        std::map<std::string, double*> new_variables_;
        std::map<std::string,std::function<double()>> functions_;
        std::map<std::string,std::function<double()>> variable_shifts_;

    private:
        void initializeFlatTuple(TTree* tree, std::map<std::string, double*> &tuple_map);
        void copyTTree();
        double lowMass_{-999.9};
        double highMass_{-999.9};
};

#endif // __MUTABLE_TTREE_H
