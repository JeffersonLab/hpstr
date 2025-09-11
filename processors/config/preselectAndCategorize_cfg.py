import HpstrConf
import sys
import os
import baseConfig as base

base.parser.add_argument(
        '--sample', choices=['data','sim_bkgd','ap_signal', 'simp_signal'],
        help='Signal which type of sample this is', required=True
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

if options.isData and not options.sample == 'data':
    raise ValueError('If running on data, sample must be "data"')
if not options.isData and options.sample == 'data':
    raise ValueError('If running on MC, sample cannot be "data", use "sim_bkgd", "ap_signal" or "simp_signal" instead')

preselect = HpstrConf.Processor('preselect', 'PreselectAndCategorize2021')
preselect.parameters["isData"] = options.isData
preselect.parameters["isSignal"] = 1 if ('signal' in options.sample) else 0
preselect.parameters["isSimpSignal"] = 1 if ('simp' in options.sample) else 0
preselect.parameters["isApSignal"] = 1 if ('ap' in options.sample) else 0
preselect.parameters["beamPosCfg"] = "" # has already been done for these samples
preselect.parameters["pSmearingFile"] = ""
#preselect.parameters["v0ProjectionFitsCfg"] = ""
preselect.parameters["v0ProjectionFitsCfg"] = file_in_hpstr(
        'analysis/data/v0_projection_2021_config.json'
        if options.isData else
        'analysis/data/v0_projection_2021_mc_signal_config.json'
)
preselect.parameters['trackBiasCfg'] = ""
#preselect.parameters['trackBiasCfg'] = file_in_hpstr(
#        'analysis/data/track_bias_corrections_data_2021.json'
#        if options.isData else
#        'analysis/data/track_bias_corrections_tritrig_2021.json'
#)

preselect.parameters['calTimeOffset'] = 37.3
#preselect.parameters['calTimeOffset'] = 37.3 if options.isData else 24.

p.sequence = [preselect]

p.input_files = infile
p.output_files = [outfile]

p.printProcess()