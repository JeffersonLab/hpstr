#ifndef SIMP_ANA_TTREE_H
#define SIMP_ANA_TTREE_H

#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <TTree.h>
#include <TFile.h>
#include <TBranch.h>

#include "MutableTTree.h"

class SimpAnaTTree : public MutableTTree {

    public:

        SimpAnaTTree(TFile* infile, std::string tree_name) : MutableTTree(infile,tree_name){};
        ~SimpAnaTTree();

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

        //misc
        bool impactParameterCut2016Canonical(double mass);
        bool testImpactParameterCut();

    private:
        double skipCutVarValue_ = -9876543210.0;
};

#endif // __SIMP_ANA_TTREE_H
