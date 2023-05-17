import HpstrConf
import baseConfig as base
import os
import math

#Define functions
def radFrac(mass):
    radF = ( -1.92497e-01 + 1.47144e-02*mass + -2.91966e-04*pow(mass,2) + 2.65603e-06*pow(mass,3) + -1.12471e-8*pow(mass,4) + 1.74765e-11*pow(mass,5) + 2.235718e-15*pow(mass,6)) #alic 2016 simps 
    return radF

#Calculated in 'makeTotRadAcc.py'
def totRadAcc(mass):
    acc = ( -7.93151e-01 + 1.04324e-01*mass + -5.55225e-03*pow(mass,2) + 1.55480e-04*pow(mass,3) + -2.53281e-06*pow(mass,4) + 2.54558e-08*pow(mass,5) + -1.60877e-10*pow(mass,6) + 6.24627e-13*pow(mass,7) + -1.36375e-15*pow(mass,8) + 1.28312e-18*pow(mass,9) ) #alic 2016 simps 
    return acc

#Calculated in 'makeMassRes.py'
def massRes(mass):
    res = 9.73217e-01 + 3.63659e-02*mass + -7.32046e-05*mass*mass #2016 simps alic
    return res

#2016 displaced A'
def radFrac_ap(mGev):
    frac = 0.1168 - 1.375*mGev + 10.19*pow(mGev,2) + 9.442*pow(mGev,3) - 367.5*pow(mGev,4) -1023*pow(mGev,5)
    return frac

#2016 displaced A'
def nbin_2016_tenpct(mGev):
    nbin = math.exp(4.903 + 208.3*mGev - 1880*pow(mGev,2) - 1868*pow(mGev,3) + 6.820e4*pow(mGev,4) - 1.980e5*pow(mGev,5))
    return nbin

#2016 displaced A'
def massRes_ap(MeV):
    res = 0.9348 + 0.05442*MeV - 5.784e-4*pow(MeV,2) + 5.852e-6*pow(MeV,3) - 1.724e-8*pow(MeV,4)
    return res

base.parser.add_argument("-s", "--zalpha_slope", type=float, dest="zalpha_slope",
            help="Input slope of zalpha cut", metavar="zalpha_slope", default=0.0271352)

base.parser.add_argument("-b", "--ztail_nevents", type=float, dest="ztail_nevents",
            help="Define Zcut based on n background events in background fit", metavar="ztail_nevents", default=0.5)

base.parser.add_argument("-z", "--scan_zcut", type=int, dest="scan_zcut",
            help="Choose best ZBi using Zcut Scan (1=yes, 0 = No)", metavar="scan_zcut", default=0)

base.parser.add_argument("-m", "--step_size", type=float, dest="step_size",
            help="Cut % of signal with each iteration", metavar="step_size", default=0.005)

base.parser.add_argument("-mass", "--mass", type=float, dest="mass",
            help="Enter signal mass value", metavar="mass", default=55.0)

options = base.parser.parse_args()

p = HpstrConf.Process()

#lcio_file = options.inFilename[0]
in_file = 'dummy.root'
out_file = options.outFilename

p.run_mode = 2
p.skip_events = options.skip_events
p.max_events = options.nevents

p.libraries.append("libprocessors.so")

###############################
#          Processors         #
################################
#Get expected signal calculation values
dNdm_VdMass = {55.0:602.e3 , 60.0:324.e3 , 75.0:93200 , 90.0:26247 , 110.0:4133}
logeps2_VdMass = {55.0:-6.3 , 60.0:-6.4 , 75.0:-6.6, 90.0:-6.7 , 110.0:-6.8}
simp_vd_mass = options.mass
simp_Ap_mass = simp_vd_mass*(3.0/1.8)
radFrac = radFrac(simp_Ap_mass)
simp_radAcc = totRadAcc(simp_Ap_mass)
massResolution = massRes(simp_vd_mass)
dNdm = 513800.0
logeps2 = -6.5

#dNdm = dNdm_VdMass[simp_vd_mass]
#logeps2 = logeps2_VdMass[simp_vd_mass]

zbi = HpstrConf.Processor('zbi','SimpZBiOptimizationProcessor')
zbi.parameters['testSpecialCut'] = 0
zbi.parameters['debug'] = options.debug
zbi.parameters['outFileName'] = options.outFilename
zbi.parameters['cuts_cfgFile'] = '/sdf/group/hps/users/alspellm/src/test/hpstr/analysis/selections/iterativeCuts.json'
zbi.parameters['zalpha_slope'] = options.zalpha_slope
zbi.parameters['scan_zcut'] = options.scan_zcut
zbi.parameters['ztail_events'] = options.ztail_nevents # 0.5
zbi.parameters['step_size'] = options.step_size
zbi.parameters['cutVariables'] = ["unc_vtx_ele_track_zalpha","unc_vtx_pos_track_zalpha","unc_vtx_ele_zbravoalpha","unc_vtx_pos_zbravoalpha"]
zbi.parameters['add_new_variables'] = ['zalpha','zbravo','zbravoalpha']
zbi.parameters['new_variable_params'] = [0.02,99.9,0.005]


#Background MC Scales
luminosity = 10.7 #pb-1
zbi.parameters['luminosity'] = luminosity

## Background ##
zbi.parameters['bkgVtxAnaFilename'] = '/sdf/group/hps/users/alspellm/projects/collaboration_meetings/may_2023/kf_data/kf_041823/output/hadd_hps_BLPass4_1958_files_recon_4.2_ana_kf.root'
zbi.parameters['bkgVtxAnaTreename'] = 'vtxana_kf_Tight_2016_simp_reach_dev/vtxana_kf_Tight_2016_simp_reach_dev_tree'
zbi.parameters['background_sf'] = 10.0

#MC Signal 
zbi.parameters['signal_sf'] = 1.0
zbi.parameters['signal_mass'] = simp_vd_mass
zbi.parameters['signalVtxAnaFilename'] = '/sdf/group/hps/users/alspellm/projects/THESIS/cut_dev/zbravo_dev_04242023/signal/kf/ana/hadd_mass_55_simp_recon_KF_ana.root'
zbi.parameters['signalVtxAnaTreename'] = 'vtxana_kf_radMatchTight_2016_simp_reach_dev/vtxana_kf_radMatchTight_2016_simp_reach_dev_tree'
zbi.parameters['signalMCAnaFilename'] = '/sdf/group/hps/users/alspellm/projects/THESIS/mc/2016/simps/slic_ana/hadd_mass_55_simp_mcAna.root'
zbi.parameters['signal_pdgid'] = '625'
zbi.parameters['signalHistCfgFilename'] = '/sdf/group/hps/users/alspellm/src/test/hpstr/analysis/plotconfigs/tracking/zbiCutVariables.json'

zbi.parameters['massRes'] = massResolution
zbi.parameters['radFrac'] = radFrac
zbi.parameters['simp_radAcc'] = simp_radAcc
zbi.parameters['dNdm'] = dNdm
zbi.parameters['logEps2'] = logeps2


# Sequence which the processors will run.
p.sequence = [zbi]

p.input_files=[in_file]
p.output_files = [out_file]
p.printProcess()

