import HpstrConf
import baseConfig as base
import os
import math

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
            help="Cut % of signal with each iteration", metavar="step_size", default=0.02)

base.parser.add_argument("-e", "--logeps2", type=float, dest="logeps2",
            help="Cut % of signal with each iteration", metavar="logeps2", default=-6.5)


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
#dNdm_VdMass = {55.0:602.e3 , 60.0:324.e3 , 75.0:93200 , 90.0:26247 , 110.0:4133}
#dNdm = dNdm_VdMass[simp_vd_mass]
#logeps2_VdMass = {55.0:-6.3 , 60.0:-6.4 , 75.0:-6.6, 90.0:-6.7 , 110.0:-6.8}
logeps2 = -6.5

zbi = HpstrConf.Processor('zbi','SimpZBiOptimizationProcessor')
#basic config
zbi.parameters['max_iteration'] = 90
zbi.parameters['year'] = 2016
zbi.parameters['debug'] = options.debug
zbi.parameters['outFileName'] = options.outFilename
zbi.parameters['scan_zcut'] = options.scan_zcut #1 will calculate ZBi as function of zcut position
zbi.parameters['step_size'] = options.step_size #Specify %variable in signal to cut with each iteration
zbi.parameters['ztail_events'] = options.ztail_nevents # 0.5 is the minimum allowed. ZBi calc breaks if 0.0

#json configs
zbi.parameters['variableHistCfgFilename'] = '/sdf/group/hps/users/alspellm/src/test/hpstr/analysis/plotconfigs/tracking/zbiCutVariables.json'
zbi.parameters['cuts_cfgFile'] = '/sdf/group/hps/users/alspellm/src/test/hpstr/analysis/selections/iterativeCuts.json'
zbi.parameters['eq_cfgFile'] = '/sdf/group/hps/users/alspellm/src/test/hpstr/analysis/selections/simps/simp_parameters.json'

#new variable configs
zbi.parameters['cutVariables'] = options.cut_variables
zbi.parameters['add_new_variables'] = options.new_vars
zbi.parameters['new_variable_params'] = options.new_vars_params

#special config
zbi.parameters['testSpecialCut'] = 0

#background config
zbi.parameters['bkgVtxAnaFilename'] = '/sdf/group/hps/users/alspellm/projects/THESIS/data/2016/BLPass4/ana/20230526/hadd_BLPass4_recon_4.2_ana_kf.root'
zbi.parameters['bkgVtxAnaTreename'] = 'vtxana_kf_Tight_2016_simp_reach_dev/vtxana_kf_Tight_2016_simp_reach_dev_tree'
zbi.parameters['background_sf'] = 1.0

#mc signal config
#zbi.parameters['signal_sf'] = 1.0
zbi.parameters['signal_sf'] = 0.1
zbi.parameters['signal_mass'] = options.mass
zbi.parameters['mass_window_nsigma'] = 2.8
zbi.parameters['signalVtxAnaFilename'] = '/sdf/group/hps/users/alspellm/projects/THESIS/cut_dev/zbravo_dev_04242023/signal/kf/ana/hadd_mass_55_simp_recon_KF_ana.root'
zbi.parameters['signalVtxAnaTreename'] = 'vtxana_kf_radMatchTight_2016_simp_reach_dev/vtxana_kf_radMatchTight_2016_simp_reach_dev_tree'
zbi.parameters['signalMCAnaFilename'] = '/sdf/group/hps/users/alspellm/projects/THESIS/mc/2016/simps/slic_ana/hadd_mass_55_simp_mcAna.root'
zbi.parameters['signal_pdgid'] = '625'

zbi.parameters['logEps2'] = options.logeps2


# Sequence which the processors will run.
p.sequence = [zbi]

p.input_files=[in_file]
p.output_files = [out_file]
p.printProcess()

