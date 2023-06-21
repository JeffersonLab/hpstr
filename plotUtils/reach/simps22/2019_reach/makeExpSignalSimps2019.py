#/bin/env python
import math
import glob
import numpy as np
import ROOT as r
import utilities as utils
import copy
import sys
from optparse import OptionParser

#Determined using 'bkg_in_signal.py'


def zCutScan(mass, shift=0.0):
    #Zcut is manually defined to be reasonably conservative, and can be scanned to be more or less conservative
    minimums = {30: -4., 40: -4, 50: -4., 60: -4., 70: -4.5, 80: -5.0, 90: -5., 100: -5.5, 110: -5.5, 120: -5.5, 130: -5.5, 140: -6.0, 150: -6.0, 160: -6.5, 170: -6.5, 180: -7.0, 190: -7., 200: -7.}
    zCut = shift + 9.71425 + -0.140865*mass + 0.000441817*math.pow(mass, 2) + -4.73974e-07*math.pow(mass, 3)  # PASS0 TRITRIG
    if mass in minimums.keys():
        if zCut < minimums[mass]:
            zCut = minimums[mass]
    else:
        if zCut < -7.0:
            zCut = -7.0
    return zCut

#Calculated in 'makeRadFrac.py'


def radFrac(mass):
    radF = -3.03024e-01 + 9.48279e-03*mass + -9.74867e-05*math.pow(mass, 2) + 4.78994e-07*math.pow(mass, 3) + -1.12040e-09*math.pow(mass, 4) + 9.92700e-13*math.pow(mass, 5)  # 2019 simps pass0d tritrig
    return radF

#Calculated in 'makeTotRadAcc.py'


def totRadAcc(mass):
    radAcc = -7.77721e-1 + 5.67200e-2*mass + -1.67381e-3*math.pow(mass, 2) + 2.60149e-05*math.pow(mass, 3) + -2.35378e-7*math.pow(mass, 4) + 1.31046e-9*math.pow(mass, 5) + -4.56049e-12*math.pow(mass, 6) + 9.67101e-15*math.pow(mass, 7) + -1.14284e-17*math.pow(mass, 8) + 5.76861e-21*math.pow(mass, 9)
    return radAcc

#Calculated by running 'vtxhProcess.py' in hpstr, then using 'makeVtxResolution.py'


def vtxRes(mass):
    mass = mass/1000.0  # cnv MeV to GeV
    res = (2.74363 - 2.282014e1*mass + 1.27987e2*pow(mass, 2) + -2.05207e2*pow(mass, 3))  # 2019 simps
    return res

#Calculated in 'makeMassRes.py'


def massRes(mass):
    massRes = 1.46696 + 1.50421e-02*mass + 3.79468e-05*math.pow(mass, 2) + -3.06407e-08*math.pow(mass, 3)  # MeV 2019 Simps
    return massRes

#Rate for A' -> 2 Dark Pions (Invisible decay)


def rate_2pi(m_Ap, m_pi, m_V, alpha_D):
    coeff = (2*alpha_D/3) * m_Ap
    pow1 = (1-(4*(m_pi)**2/(m_Ap**2)))**(3/2.)
    pow2 = ((m_V**2)/((m_Ap**2)-(m_V**2)))**2
    return coeff * pow1 * pow2

#Rate for A' -> Dark Vector + Dark Pion (Potentially visible to HPS through Dark Vector -> SM decay)


def rate_Vpi(m_Ap, m_pi, m_V, alpha_D, f_pi, rho, phi):
    x = m_pi/m_Ap
    y = m_V/m_Ap
    pi = 3.14159
    coeff = alpha_D*Tv(rho, phi)/(192*(pi**4))
    return coeff * (m_Ap/m_pi)**2 * (m_V/m_pi)**2 * (m_pi/f_pi)**4 * m_Ap*(Beta(x, y))**(3./2.)

#Branching ratio for A' -> Dark Vector + Dark Pion


def br_Vpi(m_Ap, m_pi, m_V, alpha_D, f_pi, rho, phi):
    rate = rate_Vpi(m_Ap, m_pi, m_V, alpha_D, f_pi, rho, phi) + rate_2pi(m_Ap, m_pi, m_V, alpha_D)
    if (2*m_V < m_Ap):
        rate = rate_Vpi(m_Ap, m_pi, m_V, alpha_D, f_pi, rho, phi) + rate_2pi(m_Ap, m_pi, m_V, alpha_D) + rate_2V(m_Ap, m_V, alpha_D)
    return rate_Vpi(m_Ap, m_pi, m_V, alpha_D, f_pi, rho, phi)/rate

#Branching ratio for A' -> 2 Dark Vectors


def br_2V(m_Ap, m_pi, m_V, alpha_D, f_pi, rho, phi):
    if (2*m_V >= m_Ap):
        return 0.
    rate = rate_Vpi(m_Ap, m_pi, m_V1, alpha_D, f_pi, rho, phi) + rate_2pi(m_Ap, m_pi, m_V1, alpha_D) + rate_2V(m_Ap, m_V1, alpha_D)
    return rate_2V(m_Ap, m_V1, alpha_D)/rate
# There are 3 categories of Dark Vectors, 2 being Neutral Vectors Rho and Phi, the 3rd category comprised of "charged" Vectors
# HPS is only sensitive to the decay of the 2 Neutral Vectors Rho and Phi to SM particles.
# The A' decay rates to these Dark Vector categories have slightly different coefficients, given by 'Tv'


def Tv(rho, phi):
    if rho:
        return 3/4.
    elif phi:
        return 3/2.
    else:
        return 18


def Beta(x, y):
    return (1+y**2-x**2-2*y)*(1+y**2-x**2+2*y)

#Decay rate for A' -> 2 Dark Vectors


def rate_2V(m_Ap, m_V, alpha_D):
    r = m_V/m_Ap
    return alpha_D/6 * m_Ap * f(r)


def f(r):
    num = 1 + 16*r**2 - 68*r**4 - 48*r**6
    den = (1-r**2) ** 2
    return num/den * (1-4*r**2)**0.5

#Rate of Neutral Dark Vector -> Two Leptons
#Rate is different for Rho or Phi


def rate_2l(m_Ap, m_pi, m_V, eps, alpha_D, f_pi, m_l, rho):
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


def getCtau(m_Ap, m_pi, m_V, eps, alpha_D, f_pi, m_l, rho):
    c = 3.00e10  # cm/s
    hbar = 6.58e-22  # MeV*sec
    rate = rate_2l(m_Ap, m_pi, m_V, eps, alpha_D, f_pi, m_l, rho)  # MeV
    tau = hbar/rate
    ctau = c*tau
    return ctau


def Vdistribution(z, targZ, gammact):
    return np.exp(targZ/gammact-1/gammact*z)/gammact

#gamma factor used to calculate lifetime of Dark Vectors


def gamma(m_V, E_V):
    gamma = E_V/m_V
    return gamma


Lumi = 110.  # 1/pb

utils.SetStyle()

parser = OptionParser()
parser.add_option("-o", "--outputFile", type="string", dest="outputFile",
                  help="Specify the output filename.", metavar="outputFile", default="expSigRate.root")
parser.add_option("-x", "--exclusionContourFile", type="string", dest="exContFile",
                  help="Specify the output exlusion contour filename.", metavar="exContFile", default="exContour.txt")
parser.add_option("-p", "--darkPionDecayConstRatio", type="string", dest="darkPionDecayConstRatio",
                  help="Specify the Dark Pion Decay Constant (3 or 4pi).", metavar="darkPionDecayConstRatio", default="4pi")
parser.add_option("-z", "--zcutFile", type="string", dest="zcutFile",
                  help="Name of file containing zcut values.", metavar="zcutFile", default="/sdf/group/hps/users/alspellm/projects/THESIS/simp_reach_estimates/ss_2016/reach_estimate/makeComponents/components/zcuts.dat")
parser.add_option("-t", "--lifetime", type="float", dest="lifetimefactor",
                  help="lifetime factor", metavar="lifetimefactor", default=1.0)

(options, args) = parser.parse_args()
outFile = r.TFile(options.outputFile, "RECREATE")

#VD masses
invMasses = [30, 35, 40, 45, 50, 55, 60, 65, 70, 75, 80, 85, 90,
             95, 100, 105, 110, 115, 120, 125, 130, 135, 140, 145, 150, 155, 160, 165, 170,
             175, 180, 185, 190, 195, 200]

#AP masses based on generated SIMP mass ratios 3:1.8:1
ap_invMasses = [round(x*(3./1.8), 1) for x in invMasses]
nMasses = len(invMasses)
lowM = float(ap_invMasses[0] - 5.0)
highM = float(ap_invMasses[-1] + 5.0)

#The Total Radiative Acceptance polynomial can sometimes oscillate through negative values as it approaches zero
#To remove those points, find the first A' mass where TotalRadAcc = 0, and set all lower masses to 0
#Find largest A' mass where totRadAcc < 0, and set all lower masses to 0
totalRadiativeAcceptance = {}
isNeg = False
for mass in ap_invMasses[::-1]:
    radacc = totRadAcc(mass)
    if radacc < 0:
        isNeg = True
    if isNeg:
        radacc = 0.0
    totalRadiativeAcceptance[mass] = radacc

#Scan of Luminosities
#Lumis = [0.0001,0.0005,0.001,0.01,0.1,0.2,0.5,1.0,2.0,10.0,50.,110.,200.]

#Single Luminosity
Lumis = [110.]

for q, Lumi in enumerate(Lumis):
    print("LUMI: ", Lumi)

    #MC Scaling
    mcScale = {}
    #mcScale['tritrig'] = 4.566e08*Lumi/(10000*1425) #pb2019 PASS0 TRITRIG
    mcScale['tritrig'] = 6.610e8*Lumi/(10000*1425)  # pb2019 PASS0d TRITRIG
    mcScale['wab'] = 4.715e10*Lumi/(10000*9944)  # pb2019

    '''
    READ zcuts from dat file
    zcuts = {}
    zcutFile = open(options.zcutFile,"r")
    for line in zcutFile:
        lineList = line.split()
        zcuts[float(lineList[0])] = float(lineList[1])
        pass
    print(zcuts)
    '''

    #Plots
    apProd_hh = r.TH2D("apProd_Lumi_%s_hh" % str(Lumi), "apProd_Lumi_%s;m_{A'} [MeV];log_{10}(#epsilon^{2})" % str(Lumi), nMasses, lowM, highM, 620, -10.005, -3.905)
    Nsig_hh = r.TH2D("Nsig_Lumi_%s_hh" % str(Lumi), "Nsig_Lumi_%s;m_{A'} [MeV];log_{10}(#epsilon^{2})" % str(Lumi), nMasses, lowM, highM, 620, -10.005, -3.905)
    Nsig_up_hh = r.TH2D("Nsig_up_Lumi_%s_hh" % str(Lumi), "Nsig_up_Lumi_%s;m_{A'} [MeV];log_{10}(#epsilon^{2})" % str(Lumi), nMasses, lowM, highM, 620, -10.005, -3.905)
    Nsig_low_hh = r.TH2D("Nsig_low_Lumi_%s_hh" % str(Lumi), "Nsig_low_Lumi_%s;m_{A'} [MeV];log_{10}(#epsilon^{2})" % str(Lumi), nMasses, lowM, highM, 620, -10.005, -3.905)
    effVtx_hh = r.TH2D("effVtx_Lumi_%s_hh" % str(Lumi), "effVtx_Lumi_%s;m_{A'} [MeV];log_{10}(#epsilon^{2})" % str(Lumi), nMasses, lowM, highM, 620, -10.005, -3.905)
    gcTau_hh = r.TH2D("gcTau_Lumi_%s_hh" % str(Lumi), "gcTau_Lumi_%s;m_{A'} [MeV];log_{10}(#epsilon^{2})" % str(Lumi), nMasses, lowM, highM, 620, -10.005, -3.905)

    #Exclusion Contours for NSig Central
    upExContourMass = []
    upExContourEps2 = []
    downExContourMass = []
    downExContourEps2 = []

    #Exclusion Contours for NSig Upper
    upExContourMass_up = []
    upExContourEps2_up = []
    downExContourMass_up = []
    downExContourEps2_up = []

    #Exclusion Contours for NSig Lower
    upExContourMass_low = []
    upExContourEps2_low = []
    downExContourMass_low = []
    downExContourEps2_low = []

    #zcut values
    zCutVals = []

    #Background rate counted
    dNdms = []

    #Looping over A' masses, NOT VECTOR MASSES
    for m_Ap in ap_invMasses:

        #low stats causes failure when totalRadiativeAcceptance = 0. Skip those masses
        if totalRadiativeAcceptance[m_Ap] == 0.0:
            continue

        print("Running A' mass = %i MeV" % m_Ap)

        #SIMP Params in MeV units
        m_ApGeV = m_Ap/1000.0  # A' Mass in GeV
        m_pi = m_Ap/3.0  # Dark Pion Mass (Always ratio of A' mass)
        m_vdI = int(round(m_Ap*(1.8/3.0), 0))  # Dark Vector Mass in Int
        m_vdF = float(m_vdI)  # Dark Vector Mass in float
        m_vdFGeV = m_vdF/1000.0  # Dark Vector Mass in GeV
        alpha_D = 0.01  # Dark coupling constant
        m_l = 0.511  # lepton mass (ele/pos)
        #Scale VD lifetime
        lt_factor = options.lifetimefactor
        print("LIFETIME FACTOR ", lt_factor)

        #Dark Pion decay constant...use one value at a time...
        f_pi = 0
        if options.darkPionDecayConstRatio == "3":
            f_pi = m_pi/3.
        elif options.darkPionDecayConstRatio == "4pi":
            f_pi = m_pi/(4*math.pi)
        else:
            print("Invalid specification of Dark Pion Decay Constant. No switch exists for value %s" % (options.darkPionDecayConstRatio))
            print("Killing program")
            break
        print('Dark Pion Mass : Pion Decay Constant = ', f_pi)

        #First grab the pretrigger vtx z distribution
        vdSimFilename = "/sdf/group/hps/users/alspellm/projects/THESIS/simp_reach_estimates/simps_2019/mc/simps/gen/slic/tuple_ana/files/hadd_mass_%i_simp_400bins_ana.root" % (m_vdI)
        vdSimFile = r.TFile(vdSimFilename)
        vdSimZ_hcp = copy.deepcopy(vdSimFile.Get("mcAna/mcAna_mc625Z_h"))
        vdSimFile.Close()
        vdSimZ_hcp.SetName("vdSimZ%i_hcp" % m_vdI)
        vdSimZ_h = r.TH1F("vdSimZ%i_h" % m_vdI, ";true z_{vtx} [mm];MC Events", 400, -47.5, 152.5)
        for i in range(401):
            vdSimZ_h.SetBinContent(i, vdSimZ_hcp.GetBinContent(i))
            pass
        outFile.cd()
        if (q < 1):
            vdSimZ_h.Write()

        #Next count the differential background rate in 1 MeV bin
        dNdm = 0.0
        #tritrig
        ttFile = r.TFile("/sdf/group/hps/users/alspellm/projects/THESIS/simp_reach_estimates/simps_2019/mc/tritrig_beam/hadd_tritrig_beam_ana_CR.root")
        ttTree = ttFile.Get("vtxana_kf_Tight_2019_simpCR/vtxana_kf_Tight_2019_simpCR_tree")
        ttTree.SetName("tritrig_Tight_tree")
        print("Counting background rate")
        Mbin = 30.0
        for ev in ttTree:
            if 1000.0*ev.unc_vtx_mass > m_Ap + (Mbin/2):
                continue
            if 1000.0*ev.unc_vtx_mass < m_Ap - (Mbin/2):
                continue
            dNdm += mcScale['tritrig']
            pass
        ttFile.Close()

        #WAB
        wabFile = r.TFile("/sdf/group/hps/users/alspellm/projects/THESIS/simp_reach_estimates/simps_2019/mc/wab_beam/hadd_wab_beam_ana_CR.root")
        wabTree = wabFile.Get("vtxana_kf_Tight_2019_simpCR/vtxana_kf_Tight_2019_simpCR_tree")
        wabTree.SetName("wab_Tight_tree")
        for ev in wabTree:
            if 1000.0*ev.unc_vtx_mass > m_Ap + (Mbin/2):
                continue
            if 1000.0*ev.unc_vtx_mass < m_Ap - (Mbin/2):
                continue
            dNdm += mcScale['wab']
            pass
        dNdm = dNdm/Mbin
        dNdms.append(dNdm)
        wabFile.Close()
        print("Background Rate: %f" % dNdm)

        #Next get flat tuple from anaVtx and fill eff_vtx numerator
        lowMass = m_vdF - 2.8*massRes(m_vdF)/2.0
        highMass = m_vdF + 2.8*massRes(m_vdF)/2.0
        #zCut = zcuts[m_vdF]
        #zCut = 9.71425 + -0.140865*m_vdF + 0.000441817*math.pow(m_vdF,2) + -4.73974e-07*math.pow(m_vdF,3) #More conservative zcut 2019 SIMPS
        #Scan zCut
        zCut = zCutScan(m_vdF, shift=1.0)

        zCutVals.append(zCut)

        vdSelZ_h = r.TH1F("vdSelZ%i_h" % m_vdI, ";true z_{vtx} [mm];MC Events", 400, -47.5, 152.5)
        vdSelNoZ_h = r.TH1F("vdSelNoZ%i_h" % m_vdI, ";true z_{vtx} [mm];MC Events", 400, -47.5, 152.5)

        vdFilename = "/sdf/group/hps/users/alspellm/projects/THESIS/simp_reach_estimates/simps_2019/mc/simps/gen/recon/tuple_ana/files/hadd_mass_%i_simp_recon_ana.root" % (m_vdI)
        vdFile = r.TFile(vdFilename)
        vdTree = vdFile.Get("vtxana_kf_Tight_2019_simpSR/vtxana_kf_Tight_2019_simpSR_tree")
        print("Counting Signal")
        for ev in vdTree:
            if 1000.0*ev.unc_vtx_mass > highMass:
                continue
            if 1000.0*ev.unc_vtx_mass < lowMass:
                continue
            if ev.true_vtx_z > 135.0:
                continue
            vdSelNoZ_h.Fill(ev.true_vtx_z)
            if ev.unc_vtx_z < zCut:
                continue
            vdSelZ_h.Fill(ev.true_vtx_z)
            pass
        vdFile.Close()

        #Make the efficiencies
        vdEffVtxZ_gae = r.TGraphAsymmErrors(vdSelZ_h, vdSimZ_h, "shortest")
        vdEffVtxZ_gae.SetName("vdEffVtxZ%i_gae" % m_vdI)
        vdEffVtxZ_e = r.TEfficiency(vdSelZ_h, vdSimZ_h)
        vdEffVtxZ_e.SetName("vdEffVtxZ%i_e" % m_vdI)

        outFile.cd()
        if (q < 1):
            vdSelNoZ_h.Write()
            vdSelZ_h.Write()
            vdEffVtxZ_gae.Write()
            vdEffVtxZ_e.Write()
        effCalc_h = vdEffVtxZ_e

        #count the NSig rates for central, upper, and lower estimates
        prevRate = 0.0
        prevRate_low = 0.0
        prevRate_up = 0.0

        #To exclude discovery, need at least 2.3 events
        excThr = 2.3
        print("Calculate expected signal rate")

        epsilons = []
        effVtxs = []
        gctaus_rho = []
        gctaus_phi = []
        for logEps2 in range(-1400, -100):
            tot_apProd = 0.
            #Count NSig for central, upper, and lower values
            Nsig = 0.
            Nsig_up = 0.
            Nsig_low = 0.
            logEps2 = logEps2/100.0
            eps2 = pow(10, logEps2)
            eps = float(np.sqrt(eps2))
            epsilons.append(eps)

            #Sensitive to two possible vector mesons produced. Add rates of each together for final rate
            for vector_meson in ["rho", "phi"]:
                rho = False
                phi = False
                if "rho" in vector_meson:
                    rho = True
                if "phi" in vector_meson:
                    phi = True

                ctau = getCtau(m_Ap, m_pi, m_vdF, eps, alpha_D, f_pi, m_l, rho)
                E_V = 2.3  # hardcoded based on selected V_D MC energy distribution...need to improve in future!
                gcTau = lt_factor * ctau * gamma(m_vdFGeV, E_V)
                if rho:
                    gctaus_rho.append(gcTau)
                if phi:
                    gctaus_phi.append(gcTau)

                #effVtx is calculated by integrating F(z) weighted by exponential decay
                #Calculate this term for three cases, central, lower, and upper F(z) values
                effVtx = 0.0
                effVtx_up = 0.0
                effVtx_low = 0.0
                for zbin in range(1, 401):
                    zz = vdSelZ_h.GetBinCenter(zbin)
                    zzLow = vdSelZ_h.GetBinLowEdge(zbin)
                    zzHigh = zzLow + vdSelZ_h.GetBinWidth(zbin)
                    #TO MAKE MOST CONSERVATIVE REACH ESTIMATE, INTEGRATE FROM ZCUT INSTEAD OF FROM TARGET
                    #This will not be done in the final analysis
                    if zz < zCut:
                        continue
                    #if zz < -7.5: continue
                    effVtx += (r.TMath.Exp((-7.5-zz)/gcTau)/gcTau)*effCalc_h.GetEfficiency(zbin)*vdSelZ_h.GetBinWidth(zbin)
                    effVtx_up += (r.TMath.Exp((-7.5-zz)/gcTau)/gcTau)*(effCalc_h.GetEfficiency(zbin) + effCalc_h.GetEfficiencyErrorUp(zbin))*vdSelZ_h.GetBinWidth(zbin)
                    effVtx_low += (r.TMath.Exp((-7.5-zz)/gcTau)/gcTau)*(effCalc_h.GetEfficiency(zbin) - effCalc_h.GetEfficiencyErrorLow(zbin))*vdSelZ_h.GetBinWidth(zbin)
                    pass
                effVtxs.append(effVtx)

                #total production of A's before detector acceptance/eff
                tot_apProd = (3.*137/2.)*3.14159*(m_Ap*eps2*radFrac(m_Ap)*dNdm)/totalRadiativeAcceptance[m_Ap]

                #branching ratios
                br_Vpi_val = br_Vpi(m_Ap, m_pi, m_vdF, alpha_D, f_pi, rho, phi)
                #br_Vpi_val = br_Vpi(m_Ap,m_pi,m_vdF,alpha_D,m_pi/(4*math.pi),rho,phi) #maximize BR for ratio=3
                br_V_to_ee = 1.0

                #Calculate NSig for central, upper, and lower values separately
                Nsig = Nsig + tot_apProd*effVtx*br_V_to_ee*br_Vpi_val
                Nsig_up = Nsig_up + tot_apProd*effVtx_up*br_V_to_ee*br_Vpi_val
                Nsig_low = Nsig_low + tot_apProd*effVtx_low*br_V_to_ee*br_Vpi_val

            #apProd_hh.Fill(m_Ap, logEps2, tot_apProd)
            Nsig_hh.Fill(m_Ap, logEps2, Nsig)
            Nsig_low_hh.Fill(m_Ap, logEps2, Nsig_low)
            Nsig_up_hh.Fill(m_Ap, logEps2, Nsig_up)
            effVtx_hh.Fill(m_Ap, logEps2, effVtx)
            gcTau_hh.Fill(m_Ap, logEps2, gcTau)

            #Check if NSig > threshold
            if prevRate < excThr and Nsig > excThr:
                downExContourMass.append(m_Ap)
                downExContourEps2.append(logEps2)
            if prevRate > excThr and Nsig < excThr:
                upExContourMass.append(m_Ap)
                upExContourEps2.append(logEps2)
            prevRate = Nsig

            #Check if NSig_low > threshold
            if prevRate_low < excThr and Nsig_low > excThr:
                downExContourMass_low.append(m_Ap)
                downExContourEps2_low.append(logEps2)
            if prevRate_low > excThr and Nsig_low < excThr:
                upExContourMass_low.append(m_Ap)
                upExContourEps2_low.append(logEps2)
            prevRate_low = Nsig_low

            #Check if NSig_up > threshold
            if prevRate_up < excThr and Nsig_up > excThr:
                downExContourMass_up.append(m_Ap)
                downExContourEps2_up.append(logEps2)
            if prevRate_up > excThr and Nsig_up < excThr:
                upExContourMass_up.append(m_Ap)
                upExContourEps2_up.append(logEps2)
            prevRate_up = Nsig_up

            pass

        #Debug plots
        debug_plots = False
        if (debug_plots):
            lifetime_rho_g = r.TGraph(len(epsilons), np.array(epsilons), np.array(gctaus_rho))
            lifetime_rho_g.SetName("rho_lifetime_Ap_mass_%i" % (m_Ap))
            lifetime_rho_g.SetTitle("rho_lifetime_Ap_mass_%i;epsilon;lifetime" % (m_Ap))
            lifetime_phi_g = r.TGraph(len(epsilons), np.array(epsilons), np.array(gctaus_phi))
            lifetime_phi_g.SetName("phi_lifetime_Ap_mass_%i" % (m_Ap))
            lifetime_phi_g.SetTitle("phi_lifetime_Ap_mass_%i;epsilon;lifetime" % (m_Ap))

            outFile.cd()
            lifetime_rho_g.Write()
            lifetime_phi_g.Write()

    outFile.cd()
    #Contour Plots
    upExContourMass.reverse()
    upExContourEps2.reverse()
    exContourMass = upExContourMass + downExContourMass
    exContourEps2 = upExContourEps2 + downExContourEps2
    exContourEps = [math.sqrt(pow(10, x)) for x in exContourEps2]
    if (len(exContourEps) > 0):
        contOutFile = open("%s_lumi_%s_central.txt" % (str(Lumi), options.exContFile), "w")
        for i in range(len(exContourMass)):
            contOutFile.write("%f\t%E\n" % (exContourMass[i], exContourEps[i]))
            pass
        contOutFile.close()
        exContourEps.append(exContourEps[0])
        exContourMass.append(exContourMass[0])
        excContour_g = r.TGraph(len(exContourMass), np.array(exContourMass), np.array(exContourEps))
        excContour_g.SetName("excContour_Lumi_%s_g" % str(Lumi))
        excContour_g.Write()

    upExContourMass_low.reverse()
    upExContourEps2_low.reverse()
    exContourMass_low = upExContourMass_low + downExContourMass_low
    exContourEps2_low = upExContourEps2_low + downExContourEps2_low
    exContourEps_low = [math.sqrt(pow(10, x)) for x in exContourEps2_low]
    if (len(exContourEps_low) > 0):
        contOutFile = open("%s_lumi_%s_low.txt" % (str(Lumi), options.exContFile), "w")
        for i in range(len(exContourMass_low)):
            contOutFile.write("%f\t%E\n" % (exContourMass_low[i], exContourEps_low[i]))
            pass
        contOutFile.close()
        exContourEps_low.append(exContourEps_low[0])
        exContourMass_low.append(exContourMass_low[0])
        excContour_low_g = r.TGraph(len(exContourMass_low), np.array(exContourMass_low), np.array(exContourEps_low))
        excContour_low_g.SetName("excContour_low_Lumi_%s_g" % str(Lumi))
        excContour_low_g.Write()

    upExContourMass_up.reverse()
    upExContourEps2_up.reverse()
    exContourMass_up = upExContourMass_up + downExContourMass_up
    exContourEps2_up = upExContourEps2_up + downExContourEps2_up
    exContourEps_up = [math.sqrt(pow(10, x)) for x in exContourEps2_up]
    if (len(exContourEps_up) > 0):
        contOutFile = open("%s_lumi_%s_up.txt" % (str(Lumi), options.exContFile), "w")
        for i in range(len(exContourMass_up)):
            contOutFile.write("%f\t%E\n" % (exContourMass_up[i], exContourEps_up[i]))
            pass
        contOutFile.close()
        exContourEps_up.append(exContourEps_up[0])
        exContourMass_up.append(exContourMass_up[0])
        excContour_up_g = r.TGraph(len(exContourMass_up), np.array(exContourMass_up), np.array(exContourEps_up))
        excContour_up_g.SetName("excContour_up_Lumi_%s_g" % str(Lumi))
        excContour_up_g.Write()

    zCuts_g = r.TGraph(len(invMasses), np.array([float(x) for x in invMasses]), np.array(zCutVals))
    zCuts_g.SetName("zCuts_g")

    dNdm_g = r.TGraph(len(ap_invMasses), np.array([float(x) for x in ap_invMasses]), np.array(dNdms))
    dNdm_g.SetName("dNdm_Lumi_%s_g" % str(Lumi))
    dNdm_g.Write()

    if (q < 1):
        zCuts_g.Write()

    effVtx_hh.Write()
    Nsig_hh.Write()
    Nsig_up_hh.Write()
    Nsig_low_hh.Write()
    apProd_hh.Write()
    gcTau_hh.Write()

outFile.Close()
