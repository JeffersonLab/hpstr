import HpstrConf
import sys
import os
import argparse

parser = argparse.ArgumentParser()
parser.add_argument('--isData', action='store_true', help='Flag to signal that we are processing real data')
parser.add_argument('-n',type=int,help='maximum event number to process',default=-1)
parser.add_argument('input_file',type=str,help='input ROOT file to process')
parser.add_argument('output_file',type=str,help='output ROOT file to write to')
args = parser.parse_args()

p = HpstrConf.Process()

p.run_mode = 1
p.skip_events = 0
p.max_events = args.n

p.add_library("libprocessors")

def file_in_hpstr(relpath):
    fullpath = os.environ['HPSTR_BASE']+"/"+relpath
    if not os.path.isfile(fullpath):
        raise ValueError(f'{fullpath} does not exist!')
    return fullpath

preselect = HpstrConf.Processor('preselect', 'PreSelectAndCategorize')
preselect.parameters["isData"] = 1 if args.isData else 0
preselect.parameters["beamPosCfg"] = ""
preselect.parameters["pSmearingFile"] = "" if args.isData else file_in_hpstr(
        "utils/data/smearingFile_2016_all_12112023.root"
)
preselect.parameters["v0ProjectionFitsCfg"] = file_in_hpstr(
        'analysis/data/'+(
            # for data
            'v0_projection_2016_config'
            if args.isData else
            # for tritrig and wab mc
            'v0_projection_2016_mc_7800_config'
            # mc signal accidentially generated with beamspot at (0,0)
            #'v0_projection_2016_mc_signal_config'
        )+'.json'
)

preselect.parameters['calTimeOffset'] = 56. if args.isData else 42.4
preselect.parameters['eleTrackTimeBias'] = -1.5 if args.isData else -5.5
preselect.parameters['posTrackTimeBias'] = -1.5 if args.isData else -5.5

p.sequence = [preselect]

p.input_files = [args.input_file]
p.output_files = [args.output_file]

p.printProcess()
