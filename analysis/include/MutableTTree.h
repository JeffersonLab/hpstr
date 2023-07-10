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

        //New variables
        void addVariableZalpha(double y_intercept, double slope, double alpha_z);
        void addVariableZalpha(double slope);
        void addVariableZbravo();
        void addVariableZbravoAlpha(double slope);
        void addVariableZbravosum();
        void addVariableZbravosumAlpha(double slope);

        void addVariableZalphaTop(double slope);
        void addVariableZalphaBot(double slope);
        void addVariableZalphaTopBot(double top_slope, double bot_slope);
        void addVariableZbravoAlphaTop(double slope);
        void addVariableZbravoAlphaBot(double slope);

        void addVariableZalphaBotEle(double slope);
        void addVariableZalphaTopEle(double slope);
        void addVariableZalphaTopPos(double slope);
        void addVariableZalphaBotPos(double slope);

        //Apply corrections
        void shiftVariable(std::string variable, double shift);

        //misc
        bool impactParameterCut2016Canonical(double mass);
        bool testImpactParameterCut();

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
        std::map<std::string,double*> readTuple_;
        std::map<std::string,double> writeTuple_;

    private:
        void initializeFlatTuple(TTree* tree, std::map<std::string, double*> &tuple_map);
        void copyTTree();
        std::map<std::string,TBranch*> new_branches;
        std::map<std::string, double*> new_variables_;
        std::map<std::string,std::function<double()>> functions_;
        std::map<std::string,std::function<double()>> variable_shifts_;
        double lowMass_{-999.9};
        double highMass_{-999.9};
        double skipCutVarValue_ = -9876543210.0;
};

#endif // __MUTABLE_TTREE_H
