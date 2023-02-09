#!/bin/env python

#Script to make a single variable plot

import os
import sys
import utilities as utils
import ROOT


def prepare1DStack(stackList, norms=[]):

    #norms   = kwargs.get("norms",[])
    #toUnity = kwargs.get("toUnity",False)
    normalize = False

    if (len(stackList) < 1):
        print("Passed Empty stackList. Exiting")
        return [None, None]

    if (len(norms) > 0):
        normalize = True
    if (len(norms) != len(stackList)):
        print("Requested to normalize only part of the stack")
        return [None, None]

    #default
    stack_h_ = ROOT.THStack("stack_bkg", "stack_bkg")

    tot_h_ = ROOT.TH1D("tot_bkg", "tot_bkg", stackList[0].GetXaxis().GetNbins(),
                       stackList[0].GetXaxis().GetXmin(), stackList[0].GetXaxis().GetXmax())

    for i_ in range(len(stackList)):

        if (normalize):
            stackList[i_].Scale(norms[i_])
        else:
            pass

        #stackList[i_].SetFillColor(utils.colors[i_])
        stackList[i_].SetFillColor(utils.fillColors[i_])
        stackList[i_].SetLineColor(utils.fillColors[i_])
        stackList[i_].SetMarkerColor(utils.fillColors[i_])
        stackList[i_].SetLineWidth(1)

        stack_h_.Add(stackList[i_])
        tot_h_.Add(stackList[i_])
        tot_h_.SetDirectory(0)
        stack_h_.SetTitle("")
        tot_h_.SetTitle("")

    return [stack_h_, tot_h_]
