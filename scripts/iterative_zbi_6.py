#!/usr/bin/python3

import math
import root_numpy as rnp
import numpy as np
import ROOT as r
import copy
import pandas as pd

def calculateZBi(n_on, n_off, tau):
    P_Bi = r.TMath.BetaIncomplete(1./(1.+tau), n_on, n_off+1)
    Z_Bi = math.sqrt(2)*r.TMath.ErfInverse(1-2*P_Bi) 
    return Z_Bi

### Expected Signal Calculations ###
#Rate for A' -> 2 Dark Pions (Invisible decay)
def rate_2pi(m_Ap,m_pi,m_V,alpha_D):
    coeff = (2*alpha_D/3) * m_Ap
    pow1 = (1-(4*(m_pi)**2/(m_Ap**2)))**(3/2.)
    pow2 = ((m_V**2)/((m_Ap**2)-(m_V**2)))**2
    return coeff * pow1 * pow2

#Rate for A' -> Dark Vector + Dark Pion (Potentially visible to HPS through Dark Vector -> SM decay) 
def rate_Vpi(m_Ap,m_pi,m_V,alpha_D,f_pi,rho,phi):
    x = m_pi/m_Ap
    y = m_V/m_Ap
    pi = 3.14159
    coeff = alpha_D*Tv(rho,phi)/(192*(pi**4))
    return coeff * (m_Ap/m_pi)**2 * (m_V/m_pi)**2 * (m_pi/f_pi)**4 * m_Ap*(Beta(x,y))**(3./2.)

#Branching ratio for A' -> Dark Vector + Dark Pion
def br_Vpi(m_Ap,m_pi,m_V,alpha_D,f_pi,rho,phi):
    rate = rate_Vpi(m_Ap,m_pi,m_V,alpha_D,f_pi,rho,phi) + rate_2pi(m_Ap,m_pi,m_V,alpha_D)
    if(2*m_V < m_Ap): rate = rate_Vpi(m_Ap,m_pi,m_V,alpha_D,f_pi,rho,phi) + rate_2pi(m_Ap,m_pi,m_V,alpha_D) + rate_2V(m_Ap,m_V,alpha_D)
    return rate_Vpi(m_Ap,m_pi,m_V,alpha_D,f_pi,rho,phi)/rate

#Branching ratio for A' -> 2 Dark Vectors 
def br_2V(m_Ap,m_pi,m_V,alpha_D,f_pi,rho,phi):
    if(2*m_V >= m_Ap): return 0.
    rate = rate_Vpi(m_Ap,m_pi,m_V1,alpha_D,f_pi,rho,phi) + rate_2pi(m_Ap,m_pi,m_V1,alpha_D) + rate_2V(m_Ap,m_V1,alpha_D)
    return rate_2V(m_Ap,m_V1,alpha_D)/rate

# There are 3 categories of Dark Vectors, 2 being Neutral Vectors Rho and Phi, the 3rd category comprised of "charged" Vectors
# HPS is only sensitive to the decay of the 2 Neutral Vectors Rho and Phi to SM particles.
# The A' decay rates to these Dark Vector categories have slightly different coefficients, given by 'Tv'
def Tv(rho,phi):
    if rho:
        return 3/4.
    elif phi:
        return 3/2.
    else:
        return 18

def Beta(x,y):
    return (1+y**2-x**2-2*y)*(1+y**2-x**2+2*y)

#Decay rate for A' -> 2 Dark Vectors
def rate_2V(m_Ap,m_V,alpha_D):
    r = m_V/m_Ap
    return alpha_D/6 * m_Ap * f(r)

def f(r):
    num = 1 + 16*r**2 - 68*r**4 - 48*r**6
    den = (1-r**2) ** 2
    return num/den * (1-4*r**2)**0.5

#Rate of Neutral Dark Vector -> Two Leptons
#Rate is different for Rho or Phi 
def rate_2l(m_Ap,m_pi,m_V,eps,alpha_D,f_pi,m_l,rho):
    alpha = 1/137.
    pi = 3.14159
    coeff = 16*pi*alpha_D*alpha*eps**2*f_pi**2/(3*m_V**2)
    term1 = (m_V**2/(m_Ap**2 - m_V**2))**2
    term2 = (1-(4*m_l**2/m_V**2))**0.5
    term3 = 1+(2*m_l**2/m_V**2)
    const = 1
    if rho:
        const = 2
    return coeff * term1 * term2 * term3 * m_V * const

#Lifetime of Neutral Dark Vectors
def getCtau(m_Ap,m_pi,m_V,eps,alpha_D,f_pi,m_l,rho):
    c = 3.00e10 #cm/s
    hbar = 6.58e-22 #MeV*sec
    rate = rate_2l(m_Ap,m_pi,m_V,eps,alpha_D,f_pi,m_l,rho)#MeV
    tau = hbar/rate
    ctau = c*tau
    return ctau

def Vdistribution(z,targZ,gammact):
    return np.exp(targZ/gammact-1/gammact*z)/gammact

#gamma factor used to calculate lifetime of Dark Vectors
def gamma(m_V, E_V):
    gamma = E_V/m_V
    return gamma

### ZBi Calculations ###
def calculate_ZBi(n_on, n_off, tau):
   P_Bi = r.TMath.BetaIncomplete(1./(1.+tau),n_on, n_off+1)
   Z_Bi = sqrt(2)*r.TMath.ErfInverse(1-2*P_Bi)
   return Z_Bi

def getRadFrac(tritrig_InvM_h, wab_InvM_h, rad_mcMass622_h, mV_MeV):
    ##MAKE THIS RATIO CONFIGURABLE
    mAp_GeV = (mV_MeV/1000)*(3/1.8)
    tt = ( tritrig_InvM_h.GetBinContent(tritrig_InvM_h.FindBin(mAp_GeV)) + tritrig_InvM_h.GetBinContent(tritrig_InvM_h.FindBin(mAp_GeV)-1) + tritrig_InvM_h.GetBinContent(tritrig_InvM_h.FindBin(mAp_GeV)+1) )/ 3
    wab = ( wab_InvM_h.GetBinContent(wab_InvM_h.FindBin(mAp_GeV)) + wab_InvM_h.GetBinContent(wab_InvM_h.FindBin(mAp_GeV)-1) + wab_InvM_h.GetBinContent(wab_InvM_h.FindBin(mAp_GeV)+1) )/ 3
    rad = ( rad_mcMass622_h.GetBinContent(rad_mcMass622_h.FindBin(mAp_GeV)) + rad_mcMass622_h.GetBinContent(rad_mcMass622_h.FindBin(mAp_GeV)-1) + rad_mcMass622_h.GetBinContent(rad_mcMass622_h.FindBin(mAp_GeV)+1) )/ 3
    radFrac = rad/(wab+tt)

    return radFrac

def getTotalRadiativeAcceptance(rad_mcMass622_h, rad_slic_mc622Mass_h, mV_MeV):
    mAp_GeV = (mV_MeV/1000)*(3/1.8)
    rad_slic = ( rad_slic_mc622Mass_h.GetBinContent(rad_mcMass622_h.FindBin(mAp_GeV))  + rad_slic_mc622Mass_h.GetBinContent(rad_mcMass622_h.FindBin(mAp_GeV)-1) + rad_slic_mc622Mass_h.GetBinContent(rad_mcMass622_h.FindBin(mAp_GeV)+1) )/3
    rad = ( rad_mcMass622_h.GetBinContent(rad_mcMass622_h.FindBin(mAp_GeV)) + rad_mcMass622_h.GetBinContent(rad_mcMass622_h.FindBin(mAp_GeV)-1) + rad_mcMass622_h.GetBinContent(rad_mcMass622_h.FindBin(mAp_GeV)+1) )/3
    radAcc = rad/rad_slic

    return radAcc

def iterativeSignalCuts(signal_histos, cuts, iteration, initial_integrals, cut_fraction=0.01, verbose=False):
    for cut in cuts.keys():
        if verbose: print ("Cutting",cut_fraction," of signal using cut",cut)
        var = getCutVar(cut)
        hist = signal_histos[var]
        xmax = hist.FindLastBinAbove(0.0)
        xmin = hist.FindFirstBinAbove(0.0)
        integral = hist.Integral(xmin,xmax)
        #Initial integral values for each variable histogram are saved
        if iteration < 1:
            initial_integrals[var] = integral

        if verbose: print("Initial integral of signal variable",var,"is", integral)

        #Find var value that cuts percentage of original
        cutval = None
        if isCutGT(cut):
            if verbose: print("Cut is Greater Than")
            cutval = hist.GetXaxis().GetBinLowEdge(xmin)
            if verbose: print("Initial cut value:",cutval)
            while hist.Integral(xmin,xmax) > initial_integrals[var]*(1.0-((iteration+1)*cut_fraction)):
                xmin = xmin + 1
                cutval = hist.GetXaxis().GetBinLowEdge(xmin)
                if verbose: print("New cutval:", cutval)
        else:
            if verbose: print("Cut is Less Than")
            cutval = hist.GetXaxis().GetBinUpEdge(xmax)
            if verbose: print("Initial cut value:",cutval)
            while hist.Integral(xmin,xmax) > initial_integrals[var]*(1.0-((iteration+1)*cut_fraction)):
                xmax = xmax - 1
                cutval = hist.GetXaxis().GetBinUpEdge(xmax)
                if verbose: print("New cutval:", cutval)
        cuts[cut] = cutval
        print("Cut",cut,cutval,"cuts",cut_fraction,"of signal in the variable",var)

#def fillHistogramsFromDF(df, histos):
#    for row in 

def fillInitialHistos(df, histos):
    for histo in histos.values():
        histo.Reset()

    for row in df.values:
        for i,var in enumerate(df.columns):
            if var not in histos:
                continue
            histos['%s'%(var)].Fill(row[i])

    #print("Length of df is",len(df))
    #for index, row in df.iterrows():
    #    if (index/len(df))%1==0:
    #        print("On index",index)
    #    for var in df.columns:
    #        if var not in histos:
    #            continue
    #        histos['%s'%(var)].Fill(row[var])

def makeInitialHistos(array, histos, variables):
    for event in array:
        for i, var in enumerate(variables):
            if var not in histos:
                continue
            value = event[i]
            histos['%s'%(var)].Fill(value)



def testing(array, histos, variables, iter_cut, verbose=True):
    for histo in histos.values():
        histo.Reset()

    isGtCut = isCutGT(iter_cut[0])
    iter_cut_var = getCutVar(iter_cut[0])
    iter_cut_val = iter_cut[1]
    if verbose: print("Applying",iter_cut[0],iter_cut_val,"to all events")
    for event in array:
        check_cut = event[variables.index(iter_cut_var)] 
        if isGtCut:
            if check_cut < iter_cut_val:
                continue
        else:
            if check_cut > iter_cut_val:
                continue
        for i, var in enumerate(variables):
            if var not in histos:
                continue
            value = event[i]
            histos['%s'%(var)].Fill(value)

def isCutGT(cut_string):
    sign = cut_string.split("_")[-1]
    if sign == "gt":
        return True
    else:
        return False

def getCutVar(cut_string):
    cut_var = ''
    if isCutGT(cut_string):
        cut_var = cut_string.replace("_gt",'')
    else:
        cut_var = cut_string.replace("_lt",'')
    return cut_var
        

def fitZTail(tritrig_zVtx_h, cut, max_bkg_integral):
    fitFunc = r.TF1("fit%s_f"%cut,"[0]*TMath::Exp([1]*x)", -10.0, 90.0)
    fitFunc = r.TF1("fit%s_f"%cut,"[0]*TMath::Exp([1]*x)+[2]*TMath::Exp([3]*x)", -10.0, 90.0)
    fitFunc = r.TF1("fitfunc","[0]*exp( (((x-[1])/[2])<[3])*(-0.5*(x-[1])^2/[2]^2) + (((x-[1])/[2])>=[3])*(0.5*[3]^2-[3]*(x-[1])/[2]))", -100.0, 100.0)
    gausResult = tritrig_zVtx_h.Fit("gaus","QS")
    gausParams = gausResult.GetParams()
    gausResult = tritrig_zVtx_h.Fit("gaus","QS","",gausParams[1]-3.0*gausParams[2],gausParams[1]+3.0*gausParams[2])
    gausParams = gausResult.GetParams()
    tailZ = gausParams[1] + 3.0*gausParams[2]
    bestChi2 = -99.9
    bestParams = [999.9, 999.9, 999.9, 999.9]
    bestFitInit = [999.9, 999.9, 999.9, 999.9]

    fitFunc.SetParameters(gausParams[0], gausParams[1], gausParams[2], 3.0)
    fitResult = tritrig_zVtx_h.Fit(fitFunc, "LSIM", "", gausParams[1]-2.0*gausParams[2], gausParams[1]+10.0*gausParams[2])

    zcut = -6.0
    testIntegral = fitFunc.Integral(zcut, 90.0)
    while testIntegral > max_bkg_integral:
        zcut = zcut+0.1
        testIntegral = fitFunc.Integral(zcut, 90.0)
        pass
    return zcut

#def fitZTail(tritrig_zVtx_h, mcScale, mass, max_bkg_integral):
#    tritrig_zVtx_scaled_h = copy.deepcopy(tritrig_zVtx_h).Scale(mcScale['tritrig'])
#    #Fill histogram from unbinned data
#
#    fitFunc = r.TF1("fit%i_f"%mass,"[0]*TMath::Exp([1]*x)", -10.0, 90.0)
#    fitFunc = r.TF1("fit%i_f"%mass,"[0]*TMath::Exp([1]*x)+[2]*TMath::Exp([3]*x)", -10.0, 90.0)
#    fitFunc = r.TF1("fitfunc","[0]*exp( (((x-[1])/[2])<[3])*(-0.5*(x-[1])^2/[2]^2) + (((x-[1])/[2])>=[3])*(0.5*[3]^2-[3]*(x-[1])/[2]))", -100.0, 100.0)
#    gausResult = tritrig_zVtx_scaled_h.Fit("gaus","QS")
#    gausParams = gausResult.GetParams()
#    gausResult = tritrig_zVtx_scaled_h.Fit("gaus","QS","",gausParams[1]-3.0*gausParams[2],gausParams[1]+3.0*gausParams[2])
#    gausParams = gausResult.GetParams()
#    tailZ = gausParams[1] + 3.0*gausParams[2]
#    bestChi2 = -99.9
#    bestParams = [999.9, 999.9, 999.9, 999.9]
#    bestFitInit = [999.9, 999.9, 999.9, 999.9]
#
#    fitFunc.SetParameters(gausParams[0], gausParams[1], gausParams[2], 3.0)
#    fitResult = tritrig_zVtx_scaled_h.Fit(fitFunc, "LSIM", "", gausParams[1]-2.0*gausParams[2], gausParams[1]+10.0*gausParams[2])
#
#    zcut = -6.0
#    testIntegral = fitFunc.Integral(zcut, 90.0)
#    while testIntegral > max_bkg_integral:
#        zcut = zcut+0.1
#        testIntegral = fitFunc.Integral(zcut, 90.0)
#        pass
#    print("Zcut: %f"%zcut)
#    return zcut

################### SCRIPT ################################

#input files
mV_MeV = 55
mV_GeV = 55./1000.
mAp_MeV = mV_MeV*(3/1.8)
sigfile = "/sdf/group/hps/users/alspellm/projects/THESIS/ZBi/signal/hadd_mass_%s_simp_recon_KF_ana.root"%(mV_MeV)
ttfile = "/sdf/group/hps/users/alspellm/projects/THESIS/ZBi/tritrig_beam/full_hadd_tritrigv2-beamv6_2500kBunches_HPS-PhysicsRun2016-Pass2_v4_5_0_pairs1_KF_ana_nvtx1.root"
wabfile = "/sdf/group/hps/users/alspellm/projects/THESIS/ZBi/wab_beam/full_hadd_wabv3-beamv6_2500kBunches_HPS-PhysicsRun2016-Pass2_v4_5_0_pairs1_KF_ana_nvtx1.root"
radfile = "/sdf/group/hps/users/alspellm/projects/THESIS/ZBi/rad_beam/full_hadd_RADv3-beamv6_2500kBunches_HPS-PhysicsRun2016-Pass2_v4_5_0_pairs1_KF_ana_nvtx1.root"
rad_slic_file = "/sdf/group/hps/users/alspellm/projects/THESIS/ZBi/rad_slic/hadd_RADv3_MG5_noXchange_HPS-PhysicsRun2016-Pass2_ana.root"

print("Input Files Loaded")

#Get the pretrigger vtx distribution ONLY NEED TO DO THIS ONCE!
vdSimFilename = "/sdf/group/hps/users/alspellm/projects/THESIS/mc/2016/simps/slic_ana/hadd_mass_%s_simp_mcAna.root"%(mV_MeV)
vdSimFile = r.TFile(vdSimFilename)
vdSimZ_hcp = copy.deepcopy(vdSimFile.Get("mcAna/mcAna_mc625Z_h") )
vdSimFile.Close()
vdSimZ_hcp.SetName("vdSimZ_hcp")
vdSimZ_h = r.TH1F("vdSimZ_h", ";true z_{vtx} [mm];MC Events", 200, -50.3, 149.7)
for i in range(201):
    vdSimZ_h.SetBinContent(i, vdSimZ_hcp.GetBinContent(i))
    pass

#MC Bkg Scaling
Lumi =10.7 #pb-1
mcScale = {}
mcScale['tritrig'] = 1.416e9*Lumi/(50000*9853)
mcScale['wab'] = 0.1985e12*Lumi/(100000*9966)
mcScale['rad'] = 66.36e6*Lumi/(10000*9959)
mcScale['rad_slic'] = 66.36e6*Lumi/(10000*9959)

#Select cut variables from flat tuple 
variables = ['unc_vtx_mass','true_vtx_mass', 'unc_vtx_z','true_vtx_z', 'unc_vtx_chi2', 'unc_vtx_psum', 'unc_vtx_ele_track_p', 'unc_vtx_pos_track_p']

cuts = {'unc_vtx_psum_gt' : None, 'unc_vtx_ele_track_p_gt' : None, 'unc_vtx_pos_track_p_gt' : None, 'unc_vtx_psum_lt' : None, 'unc_vtx_ele_track_p_lt' : None, 'unc_vtx_pos_track_p_lt' : None, 'unc_vtx_chi2_lt' : None}

print("Variables read in from flat tuple:",variables)
print("Set of cuts being iterated:", cuts)

#Define 1d histograms for each var
signal_histos = {}
signal_histos['unc_vtx_mass'] = r.TH1F('signal_unc_vtx_mass','signal_unc_vtx_mass', 200, 0, 0.2)
signal_histos['true_vtx_mass'] = r.TH1F('signal_true_vtx_mass','signal_true_vtx_mass', 200, 0, 0.2)
signal_histos['unc_vtx_z'] = r.TH1F('signal_unc_vtx_z','signal_unc_vtx_z', 150, -50, 100)
signal_histos['true_vtx_z'] = r.TH1F('signal_true_vtx_z','signal_true_vtx_z', 150, -50, 100)
signal_histos['unc_vtx_chi2'] = r.TH1F('signal_unc_vtx_chi2','signal_unc_vtx_chi2', 400, 0, 200)
signal_histos['unc_vtx_psum'] = r.TH1F('signal_unc_vtx_psum','signal_unc_vtx_psum', 500, 0, 5)
signal_histos['unc_vtx_ele_track_p'] = r.TH1F('signal_unc_vtx_ele_track_p','signal_unc_vtx_ele_track_p', 500, 0, 5)
signal_histos['unc_vtx_pos_track_p'] = r.TH1F('signal_unc_vtx_pos_track_p','signal_unc_vtx_pos_track_p', 500, 0, 5)

tritrig_histos = {}
tritrig_histos['unc_vtx_mass'] = r.TH1F('tritrig_unc_vtx_mass','tritrig_unc_vtx_mass', 200, 0, 0.2)
tritrig_histos['true_vtx_mass'] = r.TH1F('tritrig_true_vtx_mass','tritrig_true_vtx_mass', 200, 0, 0.2)
tritrig_histos['unc_vtx_z'] = r.TH1F('tritrig_unc_vtx_z','tritrig_unc_vtx_z', 150, -50, 100)
tritrig_histos['true_vtx_z'] = r.TH1F('tritrig_true_vtx_z','tritrig_true_vtx_z', 150, -50, 100)
tritrig_histos['unc_vtx_chi2'] = r.TH1F('tritrig_unc_vtx_chi2','tritrig_unc_vtx_chi2', 400, 0, 200)
tritrig_histos['unc_vtx_psum'] = r.TH1F('tritrig_unc_vtx_psum','tritrig_unc_vtx_psum', 500, 0, 5)
tritrig_histos['unc_vtx_ele_track_p'] = r.TH1F('tritrig_unc_vtx_ele_track_p','tritrig_unc_vtx_ele_track_p', 500, 0, 5)
tritrig_histos['unc_vtx_pos_track_p'] = r.TH1F('tritrig_unc_vtx_pos_track_p','tritrig_unc_vtx_pos_track_p', 500, 0, 5)

wab_histos = {}
wab_histos['unc_vtx_mass'] = r.TH1F('wab_unc_vtx_mass','wab_unc_vtx_mass', 200, 0, 0.2)
wab_histos['true_vtx_mass'] = r.TH1F('wab_true_vtx_mass','wab_true_vtx_mass', 200, 0, 0.2)
wab_histos['unc_vtx_z'] = r.TH1F('wab_unc_vtx_z','wab_unc_vtx_z', 150, -50, 100)
wab_histos['true_vtx_z'] = r.TH1F('wab_true_vtx_z','wab_true_vtx_z', 150, -50, 100)
wab_histos['unc_vtx_chi2'] = r.TH1F('wab_unc_vtx_chi2','wab_unc_vtx_chi2', 400, 0, 200)
wab_histos['unc_vtx_psum'] = r.TH1F('wab_unc_vtx_psum','wab_unc_vtx_psum', 500, 0, 5)
wab_histos['unc_vtx_ele_track_p'] = r.TH1F('wab_unc_vtx_ele_track_p','wab_unc_vtx_ele_track_p', 500, 0, 5)
wab_histos['unc_vtx_pos_track_p'] = r.TH1F('wab_unc_vtx_pos_track_p','wab_unc_vtx_pos_track_p', 500, 0, 5)

rad_histos = {}
rad_histos['unc_vtx_mass'] = r.TH1F('rad_unc_vtx_mass','rad_unc_vtx_mass', 200, 0, 0.2)
rad_histos['true_vtx_mass'] = r.TH1F('rad_true_vtx_mass','rad_true_vtx_mass', 200, 0, 0.2)
#rad_histos['unc_vtx_z'] = r.TH1F('rad_unc_vtx_z','rad_unc_vtx_z', 1400, -20, 120)
#rad_histos['true_vtx_z'] = r.TH1F('rad_true_vtx_z','rad_true_vtx_z', 1400, -20, 120)
#rad_histos['unc_vtx_chi2'] = r.TH1F('rad_unc_vtx_chi2','rad_unc_vtx_chi2', 400, 0, 200)
#rad_histos['unc_vtx_psum'] = r.TH1F('rad_unc_vtx_psum','rad_unc_vtx_psum', 500, 0, 5)
#rad_histos['unc_vtx_ele_track_p'] = r.TH1F('rad_unc_vtx_ele_track_p','rad_unc_vtx_ele_track_p', 500, 0, 5)
#rad_histos['unc_vtx_pos_track_p'] = r.TH1F('rad_unc_vtx_pos_track_p','rad_unc_vtx_pos_track_p', 500, 0, 5)


#Read flat tuple variables and convert to array
print("Reading flat tuples")
tt_tree = 'vtxana_kf_Tight_2016_simp_reach_dev/vtxana_kf_Tight_2016_simp_reach_dev_tree'
sig_tree = 'vtxana_kf_Tight_2016_simp_reach_dev/vtxana_kf_Tight_2016_simp_reach_dev_tree'
wab_tree = 'vtxana_kf_Tight_2016_simp_reach_dev/vtxana_kf_Tight_2016_simp_reach_dev_tree'
rad_tree = 'vtxana_kf_Tight_2016_simp_reach_dev/vtxana_kf_Tight_2016_simp_reach_dev_tree'

print("Converting flat tuples to arrays")

### Fix Control Region Values from Reach Estimate ###
#Radiative Fraction taken from reach estimate @ 100 MeV A'
radFrac = 0.07
#Total Radiative Acceptance from reach estimate @ 100 MeV A'
radAcc = 0.125
#Hardcode mass resolution for now
massRes_MeV = 3.0 #Found using 2016 simp mass res plot for mV = 55
#dNdm taken from reach estimate at ~ 100 MeV A'
dNdm = 372000.0

#SIMP Params in MeV units
m_pi = mAp_MeV/3.0                          #Dark Pion Mass (Always ratio of A' mass)
alpha_D = 0.01                              #Dark coupling constant
m_l = 0.511                                 #lepton mass (ele/pos)
f_pi = m_pi/(4*math.pi)

#Initial loose signal selection
lowMass = float(mV_MeV) - 2.0*massRes_MeV/2.0
highMass = float(mV_MeV) + 2.0*massRes_MeV/2.0

loose_selection = {'unc_vtx_psum_lt' : 2.0, 'unc_vtx_mass_lt' : highMass/1000., 'unc_vtx_mass_gt' : lowMass/1000., 'unc_vtx_z_gt' : -30.0}
signal_array = rnp.root2array(sigfile,sig_tree, branches= variables)
signal_df = pd.DataFrame(signal_array)
signal_df = signal_df[(signal_df['unc_vtx_psum'] < loose_selection['unc_vtx_psum_lt']) & (signal_df['unc_vtx_mass'] < loose_selection['unc_vtx_mass_lt']) & (signal_df['unc_vtx_mass'] > loose_selection['unc_vtx_mass_gt']) & (signal_df['unc_vtx_z'] > loose_selection['unc_vtx_z_gt']) ]

tritrig_array = rnp.root2array(ttfile,tt_tree, branches= variables)
tritrig_df = pd.DataFrame(tritrig_array)
tritrig_df = tritrig_df[(tritrig_df['unc_vtx_psum'] < loose_selection['unc_vtx_psum_lt']) & (tritrig_df['unc_vtx_mass'] < loose_selection['unc_vtx_mass_lt']) & (tritrig_df['unc_vtx_mass'] > loose_selection['unc_vtx_mass_gt']) & (tritrig_df['unc_vtx_z'] > loose_selection['unc_vtx_z_gt'])]

print("Making initial variable histograms")

#Build ZBi_matrix to hold ZBi values for all cut iterations
ZBi_matrix = {}
for cut in cuts.keys():
    ZBi_matrix[cut] = []
print("Initialized ZBi Matrix:",ZBi_matrix)

### Iterate over cuts and calculate ZBi ###
cut_fraction = 0.01
iterative_cut = ()

#Hold initial integrals of histograms for each variable. When cutting n% of signal for a variable, reference these numbers
initial_integrals = {}
for it in range(10):
    print("######## Iteration",it,"###############")

    #Fill histograms of each variable for signal and background dataframes
    print("Filling signal histos")
    fillInitialHistos(signal_df, signal_histos)
    print("Filling tritrig histos")
    fillInitialHistos(tritrig_df, tritrig_histos)

    #Determine the zcut
    #fitZTail(tritrig_zVtx_h, mcScale, mass, max_bkg_integral):

    #Cut signal by n% using m independent cuts
    print("Cutting signal histograms for each cut variable")
    iterativeSignalCuts(signal_histos, cuts, it, initial_integrals)
    print("Iteration",it,"cuts:",cuts)

    #Make a vdSelection histogram for each cut. This is the only thing that changes for each Nsig    #calculation
    cut_vdSelZs = {}
    cut_tritrig_zVtxs = {}
    for cut in cuts.keys():
        print("Init vdSelZ histos for cut",cut)
        #selection histos
        vdSelZ_h = r.TH1F("vdSelZ_h_%s"%(cut), ";true z_{vtx} [mm];MC Events", 200, -50.3, 149.7)
        cut_vdSelZs[cut] = vdSelZ_h

        zVtx_h = r.TH1D("zVtx_%s_h"%(cut), "zVtx_%s_h"%(cut), 150,-50.0, 100.0)
        cut_tritrig_zVtxs[cut] = zVtx_h

    #Loop over all background events. Independently for each cut, check if event passes. If yes,
    #count scaled background event
    cut_scaled_background = {}
    cut_noff = {}
    for cut in cuts.keys():
        cut_scaled_background[cut] = 0.0
        cut_noff[cut] = 0.0

    print("Counting tritrig events for each cut")
    #for index, row in tritrig_df.iterrows():
    for row in tritrig_df.values:
        for cut, cut_value in cuts.items():
            cutvar = getCutVar(cut)
            isGtCut = isCutGT(cut)
            value = row[tritrig_df.columns.get_loc(cutvar)]
            #value = row[cutvar]
            if isGtCut:
                if value < cut_value:
                    continue
            else:
                if value > cut_value:
                    continue
            #Only count background events within mass window of Vd
            #if 1000.0*row[tritrig_df.columns.get_loc('unc_vtx_mass')] > highMass: continue
            #if 1000.0*row[tritrig_df.columns.get_loc('unc_vtx_mass')] < lowMass: continue

            #Fill z vtx distribution after each cut
            cut_tritrig_zVtxs[cut].Fill(row[tritrig_df.columns.get_loc('unc_vtx_z')], mcScale['tritrig'])

    #Calculate zcut for each cut by fitting the zvtx distribution
    cut_zcuts = {}
    #Fit z tail
    for cut, histo in cut_tritrig_zVtxs.items():
        zcut = fitZTail(histo, cut, 100.0)
        print("Zcut for ",cut,"is",zcut)
        cut_zcuts[cut] = zcut

    #Count background for each cut, after applying the zcut
    #Try using a datafram copy and apply the zcut, then just use length of dataframe as bkg count
    '''
    print("Initial length of tritrig df:",len(tritrig_df))
    for cut in cuts.keys():
        tritrig_df_zcut = tritrig_df[tritrig_df['unc_vtx_z'] > cut_zcuts[cut]]
        print("Length of tritrig_df_zcut for cut",cut,"is",len(tritrig_df_zcut))

        cut_scaled_background[cut] = len(tritrig_df_zcut)*mcScale['tritrig']
        cut_noff[cut] = len(tritrig_df_zcut)
    print("Final length of tritrig df should be unchanged:",len(tritrig_df))

    '''
    #Count background for each cut after applying the unique zcut for each cut
    for row in tritrig_df.values:
        for cut, cut_value in cuts.items():
            cutvar = getCutVar(cut)
            isGtCut = isCutGT(cut)
            value = row[tritrig_df.columns.get_loc(cutvar)]
            #value = row[cutvar]
            if isGtCut:
                if value < cut_value:
                    continue
            else:
                if value > cut_value:
                    continue

            if row[tritrig_df.columns.get_loc('unc_vtx_z')] < cut_zcuts[cut]:
                continue

            cut_scaled_background[cut] += mcScale['tritrig']
            cut_noff[cut] += 1.0

    #Loop over all signal events. For each cut, check to see if event passes cut, and if yes,
    #fill vdSelection histogram corresponding to that cut. Will end with a collection of 
    #vd Selection histograms corresponding to each cut independently
    print("Calculating vdSelZ efficiency for each cut")
    for row in signal_df.values:
        for cut, cut_value in cuts.items():
            cutvar = getCutVar(cut)
            isGtCut = isCutGT(cut)
            value = row[signal_df.columns.get_loc(cutvar)]
            if isGtCut:
                if value < cut_value:
                    continue
            else:
                if value > cut_value:
                    continue

            if row[signal_df.columns.get_loc('unc_vtx_z')] < cut_zcuts[cut]:
                continue


            #APPLY THIS MASS WINDOW TO DF
            #Vd selection
            #if 1000.0*row[signal_df.columns.get_loc('unc_vtx_mass')] > highMass: continue
            #if 1000.0*row[signal_df.columns.get_loc('unc_vtx_mass')] < lowMass: continue
            #if ev[variables.index('unc_vtx_z')] < zCut: continue 
            cut_vdSelZs[cut].Fill(row[signal_df.columns.get_loc('true_vtx_z')])

    #Calculate the expected signal for each cut
    print("Calculating expected signal")
    for cut, cut_value in cuts.items():
        vdSelZ_h = cut_vdSelZs[cut]

        #Make efficiencies to get F(z)
        vdEffVtxZ_e = r.TEfficiency(vdSelZ_h, vdSimZ_h)
        vdEffVtxZ_e.SetName("vdEffVtxZ_e_%s"%(cut))
        effCalc_h = vdEffVtxZ_e

        #Calculate n_on (aka expected signal count)
        logEps2 = -7.5
        eps2 = pow(10, logEps2)
        eps = float(np.sqrt(eps2))
        Nsig = 0.0

        for vector_meson in ["rho","phi"]:
            rho = False
            phi = False
            if "rho" in vector_meson:
                rho = True
            if "phi" in vector_meson:
                phi = True

            ctau = getCtau(mAp_MeV, m_pi, float(mV_MeV),eps,alpha_D,f_pi,m_l,rho)
            E_V = 1.35
            gcTau = ctau * gamma(mV_GeV, E_V)

            effVtx = 0.0
            for zbin in range(1,201):
                zz = vdSelZ_h.GetBinLowEdge(zbin)
                if zz < -4.3: continue
                effVtx += (r.TMath.Exp((-4.3-zz)/gcTau)/gcTau)*(effCalc_h.GetEfficiency(zbin) - effCalc_h.GetEfficiencyErrorLow(zbin))*vdSelZ_h.GetBinWidth(zbin)

            #Calculate the total A' production rate, indpendent of detector acceptance/efficiency
            tot_apProd = (3.*137/2.)*3.14159*(mAp_MeV*eps2*radFrac*dNdm)/radAcc

            #branching ratios
            br_Vpi_val = br_Vpi(mAp_MeV,m_pi,float(mV_MeV),alpha_D,f_pi,rho,phi)
            br_V_to_ee = 1.0    #Set this to 1, Dark Vector can only decay to e+e-

            #expected A' signal given V_D decays
            Nsig = Nsig + tot_apProd*effVtx*br_V_to_ee*br_Vpi_val

        del vdEffVtxZ_e
        del effCalc_h

        n_on = Nsig + cut_scaled_background[cut]
        tau = 1./mcScale['tritrig']
        n_off = cut_noff[cut]

        print("Cut:",cut,cut_value)
        print("scaled tritrig:",cut_scaled_background[cut])
        print("NSig:",Nsig)
        print("n_on:", n_on)
        print("tau:",tau)
        print("1/tau:",1./tau)
        print("n_off:", n_off)
        print("n_off/tau:",n_off/tau)
        ZBi = calculateZBi(n_on, n_off, tau)
        print("ZBi:",ZBi)
        ZBi_matrix[cut].append((cut_value,ZBi))

    #Find best ZBi...then apply that corresponding cut to ALL variable histograms
    best_ZBi = -9999.9
    best_cut = ''
    best_cut_value = -9999.9
    for cut,arr in ZBi_matrix.items():
        cut_value = arr[it][0]
        ZBi = arr[it][1]
        if ZBi > best_ZBi:
            best_ZBi = ZBi
            best_cut = cut
            best_cut_value = cut_value
    print("Best cut:", best_cut, best_cut_value, "has ZBi =", best_ZBi)

    #Apply the best cut of this iteration to the dataframes
    if isCutGT(best_cut):
        signal_df = signal_df[ signal_df[getCutVar(best_cut)] > best_cut_value ]  
        tritrig_df = tritrig_df[ tritrig_df[getCutVar(best_cut)] > best_cut_value ]  
    else:
        signal_df = signal_df[ signal_df[getCutVar(best_cut)] < best_cut_value ]  
        tritrig_df = tritrig_df[ tritrig_df[getCutVar(best_cut)] < best_cut_value ]  

    #Clear histograms
    for histo in cut_vdSelZs.values():
        del histo
    for histo in cut_tritrig_zVtxs.values():
        del histo


#Find best ZBi from all iterations
best_ZBi = -9999.9
best_cut = ''
best_cut_value = -9999.9
for cut in ZBi_matrix.keys():
    arr = ZBi_matrix[cut]
    for e in arr:
        cut_val = e[0]
        ZBi = e[1]
        if ZBi > best_ZBi:
            best_ZBi = ZBi
            best_cut = cut
            best_cut_value = cut_val

print("Best cut:", best_cut, best_cut_value, "has ZBi =", best_ZBi)


        
