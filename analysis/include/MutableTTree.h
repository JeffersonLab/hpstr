#ifndef __MUTABLE_TTREE_H
#define __ MUTABLE_TTREE_H

#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>


#include <TTree.h>
#include <TFile.h>
#include <TBranch.h>

class MutableTTree {

    public:

        MutableTTree(TFile* infile);

        MutableTTree(TFile* infile, std::string tree_name);

        int GetEntries(){return tree_->GetEntries();}

        void GetEntry(int entry){tree_->GetEntry(entry);}

        void Fill(){tree_->Fill();}

        double getValue(std::string branch_name);

        void addBranch(std::string branch_name);

        void printTree(){tree_->Print();}

        void printEvent();

        void setBranchValue(std::string branch_name, double value){*tuple_[branch_name] = value;}

        ~MutableTTree();

    protected:
        TTree* tree_{nullptr};
        TTree* newtree_{nullptr};
        std::map<std::string,double*> tuple_;
        std::map<std::string,double*> readTuple_;
        std::map<std::string,double> writeTuple_;



    private:
        void initializeFlatTuple(TTree* tree, std::map<std::string, double*> &tuple_map);
};

#endif // __MUTABLE_TTREE_H
