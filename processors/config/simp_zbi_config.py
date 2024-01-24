import HpstrConf
import baseConfig as base
import os
import math
import json

base.parser.add_argument("-mass", "--mass", type=float, dest="mass",
            help="Enter signal mass value", metavar="mass", default=55.0)

base.parser.add_argument("-new_vars", "--new_vars", type=str, dest="new_vars",
        help="List of new variables, strings defined in processor addNewVariables", default=[], nargs='+')

base.parser.add_argument("-new_vars_params", "--new_vars_params", type=float, dest="new_vars_params",
        help="New variable parameters", default=[], nargs='+')

base.parser.add_argument("-cut_variables", "--cut_variables", type=str, dest="cut_variables",
        help="Specifcy cut variables to test", default=[], nargs='+')

base.parser.add_argument("-b", "--ztail_nevents", type=float, dest="ztail_nevents",
            help="Define Zcut based on n background events in background fit", metavar="ztail_nevents", default=0.5)

base.parser.add_argument("-z", "--scan_zcut", type=int, dest="scan_zcut",
            help="Choose best ZBi using Zcut Scan (1=yes, 0 = No)", metavar="scan_zcut", default=0)

base.parser.add_argument("-m", "--step_size", type=float, dest="step_size",
            help="Cut % of signal with each iteration", metavar="step_size", default=0.01)

base.parser.add_argument("-e", "--logeps2", type=float, dest="logeps2",
            help="Cut % of signal with each iteration", metavar="logeps2", default=-5.5)
options = base.parser.parse_args()

p = HpstrConf.Process()

######################################### Expected Signal Calculation #####################################################
#Calculated in 'makeRadFrac.py'
def radiativeFraction(mass):
    radF = -1.04206e-01 + 9.92547e-03*mass + -1.99437e-04*pow(mass,2) + 1.83534e-06*pow(mass,3) + -7.93138e-9*pow(mass,4) + 1.30456e-11*pow(mass,5) #alic 2016 simps kf 11/15/22
    return radF

#Calculated in 'makeTotRadAcc.py'
def radiativeAcceptance(mass):
    acc = ( -7.35934e-01 + 9.75402e-02*mass + -5.22599e-03*pow(mass,2) + 1.47226e-04*pow(mass,3) + -2.41435e-06*pow(mass,4) + 2.45015e-08*pow(mass,5) + -1.56938e-10*pow(mass,6) + 6.19494e-13*pow(mass,7) + -1.37780e-15*pow(mass,8) + 1.32155e-18*pow(mass,9) ) #alic 2016 simps kf 11/15/22 
    return acc

###############################################################################################################

def chooseIterativeCutVariables(zbi, cut_variables = [], new_variables = [], variable_params = []):
    zbi.parameters['cutVariables'] = cut_variables
    zbi.parameters['add_new_variaibles'] = new_variables
    zbi.parameters['new_variable_params'] = variable_params

in_file = 'dummy.root'
out_file = options.outFilename

p.run_mode = 2
p.skip_events = options.skip_events
p.max_events = options.nevents

p.libraries.append("libprocessors.so")

###############################
#          Processors         #
################################

#Initialize processor
zbi = HpstrConf.Processor('zbi','SimpZBiOptimizationProcessor')

#Configure basic settings
zbi.parameters['max_iteration'] = 25
zbi.parameters['year'] = 2016
zbi.parameters['debug'] = 1
zbi.parameters['outFileName'] = options.outFilename
zbi.parameters['scan_zcut'] = options.scan_zcut #1 will calculate ZBi as function of zcut position
zbi.parameters['step_size'] = options.step_size #Specify %variable in signal to cut with each iteration
zbi.parameters['ztail_events'] = options.ztail_nevents # 0.5 is the minimum allowed. ZBi calc breaks if 0.0

#Histogram Config
zbi.parameters['variableHistCfgFilename'] = '/sdf/group/hps/users/alspellm/src/test/hpstr/analysis/plotconfigs/tracking/zbi_optimization_histograms.json'
#Config SIMP model
eq_cfg_file = '/sdf/group/hps/users/alspellm/src/hpstr/analysis/selections/simps/simp_parameters.json'
zbi.parameters['eq_cfgFile'] = eq_cfg_file
#Choose initial set of cuts
zbi.parameters['cuts_cfgFile'] = '/sdf/group/hps/users/alspellm/projects/THESIS/analysis/tight_selection_studies/v0_projection/zbi_opt/iterativeCuts.json'
#Choose cut variables to tighten iteratively. Must be present in json file above^
chooseIterativeCutVariables(zbi, ["unc_vtx_proj_sig"])

#Configure Background
zbi.parameters['bkgVtxAnaFilename'] = '/sdf/group/hps/users/alspellm/projects/THESIS/data/2016/BLPass4c_20231006/ana_20231019/full_hadd_blpass4c_ana.root'
zbi.parameters['bkgVtxAnaTreename'] = 'vtxana_Tight_2016_simp_reach_SR'
zbi.parameters['background_sf'] = 10.0

#Configure Signal
zbi.parameters['signal_sf'] = 1.0
zbi.parameters['signal_mass'] = options.mass
zbi.parameters['mass_window_nsigma'] = 2.8
zbi.parameters['signalVtxAnaFilename'] = '/sdf/group/hps/users/alspellm/projects/THESIS/mc/2016/simps/signal_beam/20230713_slic/20230713_readout/hps-java_v5pt2pt1/pass4/recon_20231009/ana_20231020/hadd_simp_signal_%s_MeV_beam_ana.root'%(int(options.mass))
zbi.parameters['signalVtxAnaTreename'] = 'vtxana_radMatchTight_2016_simp_reach_SR'
zbi.parameters['signalVtxMCSelection'] = 'vtxana_mc_radMatchTight_2016_simp_reach_SR'
zbi.parameters['signalMCAnaFilename'] = '/sdf/group/hps/users/alspellm/projects/THESIS/mc/2016/simps/slic/20230713_slic/20230724_slic_ana/ana_files/hadd_simp_%s_MeV_rot_slic_mcana.root'%(int(options.mass))
zbi.parameters['signal_pdgid'] = '625'
zbi.parameters['logEps2'] = options.logeps2

#Configure Expected Signal Calculation
#Read simp params from json to get A' mass
eq_cfg = open(eq_cfg_file)
eq_data = json.load(eq_cfg)
for key, val in eq_data['SIMP Parameters'].items():
    mass_ratio_Ap_to_Vd = eq_data['SIMP Parameters']['mass_ratio_Ap_to_Vd']
m_Ap = options.mass*mass_ratio_Ap_to_Vd
zbi.parameters['radFrac'] = radiativeFraction(m_Ap)
zbi.parameters['radAcc'] = radiativeAcceptance(m_Ap) 
zbi.parameters['bkgControlRegionFilename'] = '/sdf/group/hps/users/alspellm/projects/THESIS/data/2016/BLPass4c_20231006/ana_20231019/full_hadd_blpass4c_ana.root'
zbi.parameters['bkgControlRegionTreename'] = 'vtxana_Tight_2016_simp_reach_CR'
zbi.parameters['dNdm_sf'] = 10.0

# Sequence which the processors will run.
p.sequence = [zbi]

p.input_files=[in_file]
p.output_files = [out_file]
p.printProcess()

