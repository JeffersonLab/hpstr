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

/**
 *@brief Read in flat TTree and create new cut variables for SIMP analysis
 */
class SimpAnaTTree : public MutableTTree {

    public:

        SimpAnaTTree(TFile* infile, std::string tree_name) : MutableTTree(infile,tree_name){};
        ~SimpAnaTTree();

        /**
         *@brief New Variables
         */
        /*
        void addVariable_unc_vtx_zalpha(double y_intercept, double slope, double alpha_z);
        void addVariable_unc_vtx_zbravo();
        void addVariable_unc_vtx_zbravoAlpha(double slope);
        void addVariable_unc_vtx_zbravosum();
        void addVariable_unc_vtx_zbravosumAlpha(double slope);

        void addVariable_unc_vtx_ZalphaTop(double slope);
        void addVariable_unc_vtx_ZalphaBot(double slope);
        void addVariable_unc_vtx_ZalphaTopBot(double top_slope, double bot_slope);
        void addVariable_unc_vtx_ZbravoAlphaTop(double slope);
        void addVariable_unc_vtx_ZbravoAlphaBot(double slope);

        void addVariable_unc_vtx_ZalphaBotEle(double slope);
        void addVariable_unc_vtx_ZalphaTopEle(double slope);
        void addVariable_unc_vtx_ZalphaTopPos(double slope);
        void addVariable_unc_vtx_ZalphaBotPos(double slope);
        */

        void addVariable_unc_vtx_ele_zalpha(double slope);
        void addVariable_unc_vtx_pos_zalpha(double slope);
        void addVariable_unc_vtx_zalpha_max(double slope);
        void addVariable_unc_vtx_zalpha_min(double slope);

        void addVariable_unc_vtx_ele_iso_z0err();
        void addVariable_unc_vtx_pos_iso_z0err();
        void addVariable_unc_vtx_ele_z0_z0err();
        void addVariable_unc_vtx_pos_z0_z0err();
        void addVariable_unc_vtx_ele_isolation_cut();
        void addVariable_unc_vtx_pos_isolation_cut();

        void addVariable_unc_vtx_ele_z0tanlambda();
        void addVariable_unc_vtx_pos_z0tanlambda();
        void addVariable_unc_vtx_ele_z0tanlambda_right(double slope);
        void addVariable_unc_vtx_pos_z0tanlambda_right(double slope);
        void addVariable_unc_vtx_ele_z0tanlambda_left(double slope);
        void addVariable_unc_vtx_pos_z0tanlambda_left(double slope);

        //misc
        bool impactParameterCut2016Canonical(double mass);
        bool testImpactParameterCut();

    private:
        double skipCutVarValue_ = -9876543210.0;//<! if cut variable is not defined for an event, assign this value
};

#endif // __SIMP_ANA_TTREE_H
