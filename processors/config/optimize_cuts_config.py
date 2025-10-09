import HpstrConf
import baseConfig as base
import os

base.parser.add_argument("-mass", "--mass", type=float, dest="mass",
                         help="Enter signal mass value", metavar="mass", default=120.0)

base.parser.add_argument("-cut_variables", "--cut_variables", type=str, dest="cut_variables",
                         help="Specifcy cut variables to test", default=[], nargs='+')

base.parser.add_argument("-b", "--ztail_nevents", type=float, dest="ztail_nevents",
                         help="Define Zcut based on n background events in background fit", metavar="ztail_nevents", default=0.5)

base.parser.add_argument("-z", "--scan_zcut", type=int, dest="scan_zcut",
                         help="Choose best ZBi using Zcut Scan (1=yes, 0 = No)", metavar="scan_zcut", default=0)

base.parser.add_argument("-m", "--step_size", type=float, dest="step_size",
                         help="Cut % of signal with each iteration", metavar="step_size", default=0.01)

base.parser.add_argument("-e", "--eps", type=float, dest="eps",
                         help="strength of effective coupling epsilon", metavar="eps", default=-4)

options = base.parser.parse_args()

p = HpstrConf.Process()


############ Expected Signal Calculation ############


def radiativeFraction(mass):
    # Calculated in 'makeRadFrac.py'
    # alic 2016 simps kf 11/15/22
    radF = -1.04206e-01 + 9.92547e-03*mass + -1.99437e-04 * \
        pow(mass, 2) + 1.83534e-06*pow(mass, 3) + -7.93138e-9*pow(mass, 4) + 1.30456e-11*pow(mass, 5)
    return radF


def radiativeAcceptance(mass):
    # Calculated in 'makeTotRadAcc.py'
    # alic 2016 simps kf 11/15/22
    acc = (-7.35934e-01 + 9.75402e-02*mass + -5.22599e-03*pow(mass, 2) + 1.47226e-04*pow(mass, 3) + -2.41435e-06*pow(mass, 4) +
           2.45015e-08*pow(mass, 5) + -1.56938e-10*pow(mass, 6) + 6.19494e-13*pow(mass, 7) + -1.37780e-15*pow(mass, 8) + 1.32155e-18*pow(mass, 9))
    return acc

#####################################################


def chooseIterativeCutVariables(zbi, cut_variables=[], new_variables=[], variable_params=[]):
    zbi.parameters['cutVariables'] = cut_variables
    zbi.parameters['new_variables'] = new_variables
    zbi.parameters['new_variable_params'] = variable_params


in_file = '/Users/schababi/Desktop/data/pass_v8/hps_014269_hadd_20files_ana.root'
out_file = options.outFilename

p.run_mode = 2
p.skip_events = options.skip_events
p.max_events = options.nevents

p.libraries.append("libprocessors.dylib")  # use .so for linux

###############################
#          Processors         #
################################

# Initialize processor
zbi = HpstrConf.Processor('zbi', 'ApOptimizationProcessor')

# Configure basic settings
zbi.parameters['max_iteration'] = 10
zbi.parameters['year'] = 2021
zbi.parameters['debug'] = 0
zbi.parameters['outFileName'] = options.outFilename
# 1 will calculate ZBi as function of zcut position
zbi.parameters['scan_zcut'] = options.scan_zcut
# Specify %variable in signal to cut with each iteration
zbi.parameters['step_size'] = options.step_size
# 0.5 is the minimum allowed. ZBi calc breaks if 0.0
zbi.parameters['ztail_nevents'] = options.ztail_nevents

hpstr_analysis_base = os.environ['HPSTR_BASE'] + '/analysis/'
hpstr_selection_base = hpstr_analysis_base + 'selections/'

# Histogram Config
zbi.parameters['variableHistCfgFilename'] = hpstr_analysis_base + \
    'plotconfigs/optimization/zbi_optimization_histograms.json'

print("Using histogram config file: ", zbi.parameters['variableHistCfgFilename'])
# Config SIMP model
zbi.parameters['eq_cfgFile'] = hpstr_selection_base + 'simps/simp_parameters.json'
# Choose initial set of cuts
zbi.parameters['cuts_cfgFile'] = hpstr_selection_base + 'cutOptimization/iterativeCuts.json'
# Choose cut variables to tighten iteratively. Must be present in json file above^
chooseIterativeCutVariables(zbi, ["unc_vtx_proj_sig"])

# Configure Background
zbi.parameters['bkgVtxAnaFilename'] = '/Users/schababi/Desktop/data/pass_v8/hps_014269_hadd_20files_ana.root'
zbi.parameters['bkgVtxAnaTreename'] = 'preselection'
zbi.parameters['background_sf'] = 1.0

# Configure Signal
zbi.parameters['signal_sf'] = 1e6
zbi.parameters['signal_mass'] = options.mass * 1e-3  # in GeV
zbi.parameters['mass_window_nsigma'] = 2.
zbi.parameters['signalVtxSubsetAnaFilename'] = '/Users/schababi/Desktop/data/pass_v8/signal_subsets/ap_pulser_{}MeV_hadd10files.root'.format(
    int(options.mass))
zbi.parameters['signalVtxAnaFilename'] = '/Users/schababi/Desktop/data/pass_v8/ap_pulser_{}MeV_hadd_250files_ana.root'.format(
    int(options.mass))
zbi.parameters['signalVtxAnaTreename'] = 'preselection'
# pre-trigger signal MC analysis file
zbi.parameters['signalMCAnaFilename'] = '/Users/schababi/Desktop/data/pass_v8/MC_truth/ap{}_3pt74_MC_truth_40_stdhep_files.root'.format(
    int(options.mass))
zbi.parameters['signalMCAnaTreename'] = 'tree'
zbi.parameters['signal_pdgid'] = '623'
zbi.parameters['eps'] = options.eps

zbi.parameters['radFrac'] = radiativeFraction(options.mass)
zbi.parameters['hit_category'] = 'l1l1'
zbi.parameters['ztarget'] = -0.5  # in mm

# Sequence which the processors will run.
p.sequence = [zbi]

p.input_files = [in_file]
p.output_files = [out_file]
p.printProcess()
