#/bin/env python
import math
import glob
import numpy as np
import ROOT as r
import utilities as utils
import copy
import sys
from optparse import OptionParser

#Calculated in 'makeRadFrac.py'


def radFrac(mass):
    radF = -8.95377e-01 + 4.15313e-02*mass + -6.92406e-04*math.pow(mass, 2) + 6.01862e-06*math.pow(mass, 3) + -2.94263e-08*math.pow(mass, 4) + 8.10924e-11*math.pow(mass, 5) + -1.17466e-13*math.pow(mass, 6) + 6.96371e-17*math.pow(mass, 7)

    return radF

#Calculated in 'makeTotRadAcc.py'


def totRadAcc(mass):
    totRadAcc = -7.40426e-01 + 5.72523e-02*mass + -1.74473e-03*math.pow(mass, 2) + 2.69127e-05*math.pow(mass, 3) + -2.28890e-07*math.pow(mass, 4) + 1.12918e-09*math.pow(mass, 5) + -3.23213e-12*math.pow(mass, 6) + 4.99070e-15*math.pow(mass, 7) + -3.21865e-18*math.pow(mass, 8)

    return totRadAcc

#Calculated by running 'vtxhProcess.py' in hpstr, then using 'makeVtxResolution.py'


def vtxRes(mass):
    mass = mass/1000.0  # cnv MeV to GeV
    res = (2.74363 - 2.282014e1*mass + 1.27987e2*pow(mass, 2) + -2.05207e2*pow(mass, 3))  # 2019 simps
    return res

#Calculated in 'makeMassRes.py'


def massRes(mass):
    #res = (1.29712  + 1.93768e-02*mass + -3.67914e-07*pow(mass,2) + 9.77287e-8*pow(mass,3)) #alic 2019 simps
    massRes = 2.04342 + -2.50772e-03*mass + 1.50841e-04*mass*mass + -2.68102e-07*math.pow(mass, 3)
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

#Branching ratio for A' -> 2 Dark Vectors


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
invMasses = [25, 30, 35, 40, 45, 50, 55, 60, 65, 70, 75, 80, 85, 90,
             95, 100, 105, 110, 115, 120, 125, 130, 135, 140, 145, 150, 155, 160, 165, 170,
             175, 180, 185, 190, 195, 200]

# Loop over AP masses based on MC gen mass ratios
ap_invMasses = [round(x*(3./1.8), 1) for x in invMasses]
nMasses = len(invMasses)
lowM = float(ap_invMasses[0] - 5.0)
highM = float(ap_invMasses[-1] + 5.0)

#Build list of totalRadiativeAcceptance for all A' masses
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
print("totRadAcc: ", totalRadiativeAcceptance)

#Lumis = [0.0001,0.0005,0.001,0.01,0.1,0.2,0.5,1.0,2.0,10.0,50.,168.1,530.,698.1]
Lumis = [168.1]
for q, Lumi in enumerate(Lumis):

    print("LUMI: ", Lumi)
    #MC Scaling
    mcScale = {}
    mcScale['tritrig'] = 6.853e08*Lumi/(10000*993)
    mcScale['wab'] = 7.557e10*Lumi/(30000*3333 + 1405*40000 + 907*40000)

    '''
    zcuts from dat file
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

    #Exclusion Contours
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
        vdSimFilename = "/sdf/group/hps/users/alspellm/projects/THESIS/simp_reach_estimates/simps_3pt7/mc/simps/slic/tuple_ana/files/hadd_mass_%i_simp_slic_ana.root" % (m_vdI)
        vdSimFile = r.TFile(vdSimFilename)
        vdSimZ_hcp = copy.deepcopy(vdSimFile.Get("mcAna/mcAna_mc625Z_h"))
        vdSimFile.Close()
        vdSimZ_hcp.SetName("vdSimZ%i_hcp" % m_vdI)
        vdSimZ_h = r.TH1F("vdSimZ%i_h" % m_vdI, ";true z_{vtx} [mm];MC Events", 200, -50.3, 149.7)
        #vdSimZ_h = r.TH1F("vdSimZ%i_h"%m_vdI, ";true z_{vtx} [mm];MC Events", 200, -50.5, 149.5)
        for i in range(201):
            vdSimZ_h.SetBinContent(i, vdSimZ_hcp.GetBinContent(i))
            pass
        outFile.cd()
        if (q < 1):
            vdSimZ_h.Write()

        #Next count the differential background rate in 1 MeV bin
        dNdm = 0.0
        #tritrig
        ttFile = r.TFile("/sdf/group/hps/users/alspellm/projects/THESIS/simp_reach_estimates/simps_3pt7/mc/tritrig_beam/tuple_ana/CR/hadd_tritrig_beam_ana.root")
        ttTree = ttFile.Get("vtxana_kf_Tight_2021/vtxana_kf_Tight_2021_tree")
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
        wabFile = r.TFile("/sdf/group/hps/users/alspellm/projects/THESIS/simp_reach_estimates/simps_3pt7/mc/wab_beam/super_hadd_wab_beam_CR_ana.root")
        wabTree = wabFile.Get("vtxana_kf_Tight_2021/vtxana_kf_Tight_2021_tree")
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
        zCut = 19.4594 + -0.0677212*m_vdF + -0.000742151*pow(m_vdF, 2) + 4.64602e-06*pow(m_vdF, 3) + -6.79071e-09*pow(m_vdF, 4)  # handmade conservative zcut 2021 simps alic
        #zCut = zcuts[m_vdF]
        zCutVals.append(zCut)
        vdSelZ_h = r.TH1F("vdSelZ%i_h" % m_vdI, ";true z_{vtx} [mm];MC Events", 200, -50.3, 149.7)
        vdSelNoZ_h = r.TH1F("vdSelNoZ%i_h" % m_vdI, ";true z_{vtx} [mm];MC Events", 200, -50.3, 149.7)
        vdFilename = "/sdf/group/hps/users/alspellm/projects/THESIS/simp_reach_estimates/simps_3pt7/mc/simps/recon/tuple_ana/files/hadd_mass_%i_simp_recon_ana.root" % (m_vdI)
        vdFile = r.TFile(vdFilename)
        vdTree = vdFile.Get("vtxana_kf_Tight_2021_simpSR/vtxana_kf_Tight_2021_simpSR_tree")
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

        prevRate = 0.0
        prevRate_low = 0.0
        prevRate_up = 0.0

        excThr = 2.3
        print("Calculate expected signal rate")

        epsilons = []
        effVtxs = []
        gctaus = []
        for logEps2 in range(-1400, -100):
            tot_apProd = 0.
            Nsig = 0.
            Nsig_up = 0.0
            Nsig_low = 0.0
            logEps2 = logEps2/100.0
            eps2 = pow(10, logEps2)
            eps = float(np.sqrt(eps2))
            epsilons.append(logEps2)
            #Sensitive to two possible vector mesons produced. Add rates of each together for final rate
            for vector_meson in ["rho", "phi"]:
                rho = False
                phi = False
                if "rho" in vector_meson:
                    rho = True
                if "phi" in vector_meson:
                    phi = True

                ctau = getCtau(m_Ap, m_pi, m_vdF, eps, alpha_D, f_pi, m_l, rho)
                E_V = 1.85  # hardcoded based on selected V_D MC energy distribution...need to improve in future!
                gcTau = lt_factor * ctau * gamma(m_vdFGeV, E_V)
                gctaus.append(gcTau)

                effVtx = 0.0
                effVtx_up = 0.0
                effVtx_low = 0.0

                for zbin in range(1, 201):
                    zz = vdSelZ_h.GetBinCenter(zbin)
                    zzLow = vdSelZ_h.GetBinLowEdge(zbin)
                    zzHigh = zzLow + vdSelZ_h.GetBinWidth(zbin)
                    #Restric to beyond or at 2016 target position
                    if zz < zCut:
                        continue
                    effVtx += (r.TMath.Exp((0.0-zz)/gcTau)/gcTau)*effCalc_h.GetEfficiency(zbin)*vdSelZ_h.GetBinWidth(zbin)
                    effVtx_up += (r.TMath.Exp((0.0-zz)/gcTau)/gcTau)*(effCalc_h.GetEfficiency(zbin) + effCalc_h.GetEfficiencyErrorUp(zbin))*vdSelZ_h.GetBinWidth(zbin)
                    effVtx_low += (r.TMath.Exp((0.0-zz)/gcTau)/gcTau)*(effCalc_h.GetEfficiency(zbin) - effCalc_h.GetEfficiencyErrorLow(zbin))*vdSelZ_h.GetBinWidth(zbin)
                    pass
                effVtxs.append(effVtx)

                #total production of A's before detector acceptance/eff
                tot_apProd = (3.*137/2.)*3.14159*(m_Ap*eps2*radFrac(m_Ap)*dNdm)/totalRadiativeAcceptance[m_Ap]

                #branching ratios
                br_Vpi_val = br_Vpi(m_Ap, m_pi, m_vdF, alpha_D, f_pi, rho, phi)
                #br_Vpi_val = br_Vpi(m_Ap,m_pi,m_vdF,alpha_D,m_pi/(4*math.pi),rho,phi) #maximize BR for ratio=3
                br_V_to_ee = 1.0
                Nsig = Nsig + tot_apProd*effVtx*br_V_to_ee*br_Vpi_val
                Nsig_up = Nsig_up + tot_apProd*effVtx_up*br_V_to_ee*br_Vpi_val
                Nsig_low = Nsig_low + tot_apProd*effVtx_low*br_V_to_ee*br_Vpi_val

                if tot_apProd < 0.0:
                    print("Negative Nsig: ", Nsig)

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

    outFile.cd()
    #Contour Plots
    upExContourMass.reverse()
    upExContourEps2.reverse()
    exContourMass = upExContourMass + downExContourMass
    exContourEps2 = upExContourEps2 + downExContourEps2
    exContourEps = [math.sqrt(pow(10, x)) for x in exContourEps2]
    if len(exContourEps) > 0:
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
