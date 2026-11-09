import HpstrConf
import sys
import os
import baseConfig as base

base.parser.add_argument(
        '--sample', choices=['data','sim_bkgd','ap_signal', 'simp_signal'],
        help='Signal which type of sample this is', required=False
)
base.parser.add_argument(
        '--smearing', dest='smearing', action='store_true',
        help='Enable track smearing (disabled by default)'
)
base.parser.add_argument(
        '--no-smearing', dest='noSmearing', action='store_true',
        help='Explicitly disable track smearing'
)
base.parser.add_argument(
        '--trigger', dest='trigger', choices=['singles2', 'pairs'], default='singles2',
        help='Which trigger to require in preselection (default: singles2)'
)

options = base.parser.parse_args()

infile = options.inFilename
outfile = options.outFilename

print('Input file: %s' % infile)
print('Output file: %s' % outfile)

p = HpstrConf.Process()

p.run_mode = 1
p.skip_events = options.skip_events
p.max_events = options.nevents

p.add_library("libprocessors")

def file_in_hpstr(relpath):
    fullpath = os.environ['HPSTR_BASE']+"/"+relpath
    if not os.path.isfile(fullpath):
        raise ValueError(f'{fullpath} does not exist!')
    return fullpath

if options.isData:
    options.sample = 'data'
if not options.isData and options.sample == 'data':
    raise ValueError('If running on MC, sample cannot be "data", use "sim_bkgd", "ap_signal" or "simp_signal" instead')

preselect = HpstrConf.Processor('preselect', 'PreselectAndCategorize2021')
preselect.parameters["isData"] = 1 if ('data' in options.sample) else 0 
preselect.parameters["isSignal"] = 1 if ('signal' in options.sample) else 0
preselect.parameters["isSimpSignal"] = 1 if ('simp' in options.sample) else 0
preselect.parameters["isApSignal"] = 1 if ('ap' in options.sample) else 0
preselect.parameters["beamPosCfg"] = "" # has already been done for these samples
preselect.parameters["pSmearingFile"] = ""
#preselect.parameters["debug"] = 1
#preselect.parameters["apPDG"] = 623
#preselect.parameters["disablePreselection"] = 1
preselect.parameters["disableTimingCuts"] = 0
preselect.parameters["vtxCollection"] = "UnconstrainedV0Vertices_KF"
preselect.parameters["v0ProjectionFitsCfg"] = file_in_hpstr(
        'analysis/data/v0_projection_2021_v9_config.json'
        if options.isData else
        'analysis/data/v0_projection_2021_v9_mc_14272_config.json'
)
preselect.parameters['trackBiasCfg'] = ""
#preselect.parameters['trackBiasCfg'] = file_in_hpstr(
#        'analysis/data/track_bias_corrections_data_2021.json'
#        if options.isData else
#        'analysis/data/track_bias_corrections_tritrig_2021.json'
#)

#preselect.parameters["beamPosCfg"] = file_in_hpstr(
#        'analysis/data/beam_pos_2021_config.json'
#        if options.isData else
#        'analysis/data/beam_pos_2021_mc_config.json'
#)

preselect.parameters['calTimeOffset'] = 37.3
#preselect.parameters['calTimeOffset'] = 37.3 if options.isData else 24.

preselect.parameters["doZ0Corrections"] = 0
preselect.parameters["z0CalibCfg"] = file_in_hpstr('analysis/data/smearing/z0_calib_2021.json')
preselect.parameters["z0CalibMcCfg"] = file_in_hpstr('analysis/data/smearing/z0_calib_2021_mc.json')
preselect.parameters["smearOmega"] = 1
preselect.parameters["smearingVariable"] = "tanLambda"
preselect.parameters["scaleCorrVariable"] = ""   # scale corrections disabled
preselect.parameters["applyMeanCorr"] = 0
preselect.parameters["doV0ProjZ0"] = 0
preselect.parameters["smearingCfg"] = os.environ['HPSTR_BASE']+"/analysis/data/smearing/tool_smearing.json"
preselect.parameters["eleMinHits"] = 10 #6
preselect.parameters["posMinHits"] = 10 #8 
preselect.parameters["useVertexMomentum"] = 1  # 0 = use standalone track momenta; 1 = vertex-fitted
preselect.parameters["triggerSelection"] = options.trigger
preselect.parameters["psumCut"] = 0.0
preselect.parameters["doSmearing"] = 1 if (options.smearing and not options.noSmearing and options.sample != 'data') else 0
preselect.parameters["smearingFactor"] = 1.0
preselect.parameters["smearingSeed"] = options.seed
preselect.parameters["requireTruthMatch"] = 1

p.sequence = [preselect]

p.input_files = infile
p.output_files = [outfile]

p.printProcess()
