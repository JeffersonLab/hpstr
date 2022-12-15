#!/usr/bin/python3

import root_numpy as rnp
import numpy as np
import ROOT as r
import copy

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
    tt = tritrig_InvM_h.GetBinContent(tritrig_InvM_h.FindBin(mAp_GeV))
    wab = wab_InvM_h.GetBinContent(wab_InvM_h.FindBin(mAp_GeV))
    rad = rad_mcMass622_h.GetBinContent(rad_mcMass622_h.FindBin(mAp_GeV))
    radFrac = rad/(wab+tt)
    print(rad_mcMass622_h.GetEntries())
    print(tt, wab, rad)

    return radFrac


################### SCRIPT ################################

#input files
mV_MeV = 100
sigfile = "/sdf/group/hps/users/alspellm/projects/THESIS/ZBi/signal/hadd_mass_%s_simp_recon_KF_ana.root"%(mV_MeV)
ttfile = "/sdf/group/hps/users/alspellm/projects/THESIS/ZBi/tritrig_beam/full_hadd_tritrigv2-beamv6_2500kBunches_HPS-PhysicsRun2016-Pass2_v4_5_0_pairs1_KF_ana_nvtx1.root"
radfile = "/sdf/group/hps/users/alspellm/projects/THESIS/ZBi/rad_beam/full_hadd_RADv3-beamv6_2500kBunches_HPS-PhysicsRun2016-Pass2_v4_5_0_pairs1_KF_ana_nvtx1.root"
wabfile = "/sdf/group/hps/users/alspellm/projects/THESIS/ZBi/wab_beam/full_hadd_wabv3-beamv6_2500kBunches_HPS-PhysicsRun2016-Pass2_v4_5_0_pairs1_KF_ana_nvtx1.root"

#MC Bkg Scaling
Lumi =10.7 #pb-1
tt_scaling = 1.416e9*Lumi/(50000*9853)
wab_scaling = 0.1985e12*Lumi/(100000*9966)
rad_scaling = 66.36e6*Lumi/(10000*9959)

#Select cut variables from flat tuple 
variables = ['unc_vtx_z', 'unc_vtx_chi2', 'unc_vtx_psum', 'unc_vtx_ele_track_p', 'unc_vtx_pos_track_p']

#Define 1d histograms for each var
signal_histos = {}
signal_histos['unc_vtx_z'] = r.TH1F('signal_unc_vtx_z','signal_unc_vtx_z', 1400, -20, 120)
signal_histos['unc_vtx_chi2'] = r.TH1F('signal_unc_vtx_chi2','signal_unc_vtx_chi2', 400, 0, 200)
signal_histos['unc_vtx_psum'] = r.TH1F('signal_unc_vtx_psum','signal_unc_vtx_psum', 500, 0, 5)
signal_histos['unc_vtx_ele_track_p'] = r.TH1F('signal_unc_vtx_ele_track_p','signal_unc_vtx_ele_track_p', 500, 0, 5)
signal_histos['unc_vtx_pos_track_p'] = r.TH1F('signal_unc_vtx_pos_track_p','signal_unc_vtx_pos_track_p', 500, 0, 5)

tritrig_histos = {}
tritrig_histos['unc_vtx_z'] = r.TH1F('tritrig_unc_vtx_z','tritrig_unc_vtx_z', 1400, -20, 120)
tritrig_histos['unc_vtx_chi2'] = r.TH1F('tritrig_unc_vtx_chi2','tritrig_unc_vtx_chi2', 400, 0, 200)
tritrig_histos['unc_vtx_psum'] = r.TH1F('tritrig_unc_vtx_psum','tritrig_unc_vtx_psum', 500, 0, 5)
tritrig_histos['unc_vtx_ele_track_p'] = r.TH1F('tritrig_unc_vtx_ele_track_p','tritrig_unc_vtx_ele_track_p', 500, 0, 5)
tritrig_histos['unc_vtx_pos_track_p'] = r.TH1F('tritrig_unc_vtx_pos_track_p','tritrig_unc_vtx_pos_track_p', 500, 0, 5)


#Read flat tuple variables and convert to array
tt_tree = 'vtxana_kf_Tight_2016_simp_reach_dev/vtxana_kf_Tight_2016_simp_reach_dev_tree'
sig_tree = 'vtxana_kf_Tight_2016_simp_reach_dev/vtxana_kf_Tight_2016_simp_reach_dev_tree'
sig_arr = rnp.root2array(sigfile,sig_tree, branches= variables, start=0, stop = 50, step = 1)
tt_array = rnp.root2array(ttfile,tt_tree, branches= variables, start=0, stop = 50, step = 1)

#Fill 1d signal histograms for each variable 
print("Filling 1D signal histograms for variables: ", variables) 
for event in sig_arr:
    print(event)
    for i,var in enumerate(variables):
        signal_histos['%s'%(var)].Fill(event[i])

#Define map to hold cut values for each variable
iter_cut_map = {key: None for key in variables}

#Iterate through histos, independently for each variable, define cut as value where n% of signal is cut
print("Cutting signal for each variable")
for var,hist in signal_histos.items():
    cut_percentage = 0.1
    xmax = hist.FindLastBinAbove(0.0)
    xmin = hist.FindFirstBinAbove(0.0)
    integral = hist.Integral(xmin,xmax)

    #Find var value that cuts percentage of original
    cutval = hist.GetXaxis().GetBinUpEdge(xmax)
    while hist.Integral(xmin,xmax) > integral*(1.0-cut_percentage):
        xmax = xmax - 1
        cutval = hist.GetXaxis().GetBinUpEdge(xmax)
    iter_cut_map[var] = cutval

#calculate the radFrac for the specified A' mass value (m_VD = (1.8/3)*m_A')
print("Making Radiative Fraction")
invMassHistos = {}
vtx_selection = "vtxana_kf_Tight_2016_simp_reach_dev"
inFile = r.TFile(ttfile,"READ")
invMassHistos['tritrig'] = copy.deepcopy(inFile.Get("%s/%s_vtx_InvM_h"%(vtx_selection,vtx_selection)))
inFile.Close()
inFile = r.TFile(wabfile,"READ")
invMassHistos['wab'] = copy.deepcopy(inFile.Get("%s/%s_vtx_InvM_h"%(vtx_selection,vtx_selection)))
inFile.Close()
radmatch_selection = 'vtxana_kf_radMatchTight_2016_simp_reach_dev'
inFile = r.TFile(wabfile,"READ")
invMassHistos['rad'] = copy.deepcopy(inFile.Get("%s/%s_mcMass622_h"%(radmatch_selection,radmatch_selection)))
inFile.Close()

#Scale invMassHistos
invMassHistos['tritrig'].Scale(tt_scaling)
invMassHistos['wab'].Scale(wab_scaling)
invMassHistos['rad'].Scale(rad_scaling)

radFrac = getRadFrac(invMassHistos['tritrig'], invMassHistos['wab'], invMassHistos['rad'], mV_MeV)
print("radFrac: ", radFrac)

best_zbi = 0
best_var = None
best_cut = None
#for var,cut in iter_cut_map.items():
#    zbi = 


#outfile = r.TFile("testout.root","RECREATE")
#outfile.cd()
#for histo in signal_histos.values():
#    histo.Write()
#outfile.Close()
#
