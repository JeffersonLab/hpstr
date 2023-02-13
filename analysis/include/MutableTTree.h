#ifndef __MUTABLE_TTREE_H
#define __ MUTABLE_TTREE_H

#include <iostream>
#include <fastream>
#include <string>
#include <cstdlib>


#include <TTree.h>
#include <TFile.h>
#include <TBranch.h>

class MutableTTree {

    MutableTTree(TFile* infile);

    MutableTTree(TFile* infile, std::string tree_name);

    int GetEntries(){return ttree_->GetEntries()};

    void GetEntry(int entry){ttree_->GetEntry(entry)};

    double getValue(std::string branch_name);

    void addBranch(std::string branch_name);

    ~MutableTTree();



    protected:
        TTree* tree_{nullptr};
        std::map<std::string,*double> tuple_;


    private:
        void initializeFlatTuple(TTree* tree, std::map<std::string, double*> &tuple_map);
};

#endif // __MUTABLE_TTREE_H
