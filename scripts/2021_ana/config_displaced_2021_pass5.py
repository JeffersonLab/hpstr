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
# Track corrections (z0 offsets / calibration, data scale corrections, MC smearing):
# one TrackCorrector JSON. doSmearing, the seed and requireTruthMatch stay per-job parameters.
preselect.parameters["trackCorrectionsCfg"] = file_in_hpstr('analysis/data/corrections/preselection_2021_pass5.json')
#preselect.parameters["debug"] = 1
#preselect.parameters["apPDG"] = 623
#preselect.parameters["disablePreselection"] = 1
preselect.parameters["disableTimingCuts"] = 0
preselect.parameters["vtxCollection"] = "UnconstrainedV0Vertices_KF"
# V0 projection fits for the vertex-projection significance only; the z0 offset taken from
# them (when wanted) is set in the trackCorrectionsCfg JSON.
preselect.parameters["v0ProjectionFitsCfg"] = file_in_hpstr(
        'analysis/data/v0_projection_2021_v9_config.json'
        if options.isData else
        'analysis/data/v0_projection_2021_v9_mc_14272_config.json'
)

#preselect.parameters["beamPosCfg"] = file_in_hpstr(
#        'analysis/data/beam_pos_2021_config.json'
#        if options.isData else
#        'analysis/data/beam_pos_2021_mc_config.json'
#)

preselect.parameters['calTimeOffset'] = 37.3
#preselect.parameters['calTimeOffset'] = 37.3 if options.isData else 24.

preselect.parameters["doV0ProjZ0"] = 0
preselect.parameters["eleMinHits"] = 10
preselect.parameters["posMinHits"] = 10
preselect.parameters["useVertexMomentum"] = 1  # 0 = use standalone track momenta; 1 = vertex-fitted
preselect.parameters["requireSingleVertex"] = 1  # 0 = keep events w/ >=2 preselected vertices, select best chi2; 1 = veto them
preselect.parameters["triggerSelection"] = options.trigger
preselect.parameters["psumCut"] = 0.0
preselect.parameters["doSmearing"] = 1 if (options.smearing and not options.noSmearing and options.sample != 'data') else 0
preselect.parameters["smearingSeed"] = options.seed
preselect.parameters["requireTruthMatch"] = 1

p.sequence = [preselect]

p.input_files = infile
p.output_files = [outfile]

p.printProcess()
