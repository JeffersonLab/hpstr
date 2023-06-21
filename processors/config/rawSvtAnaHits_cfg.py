import HpstrConf
import sys
import os
import baseConfig as base

base.parser.add_argument("-A", "--onTrk", type=int, dest="onTrk",
                  help="Are we using hits on track or not", metavar="onTrk",default=0)
base.parser.add_argument("-p", "--tphase", type=int, dest="tphase",
                  help="The Phase of the Event Time", metavar="tphase",default=6)

options = base.parser.parse_args()

# Use the input file to set the output file name
root1_file = options.inFilename[0]
root2_file = options.outFilename
onTrk = options.onTrk

print('Root file Input: %s' % root1_file)
print('Root file Output: %s' % root2_file)

p = HpstrConf.Process()

p.run_mode = 1
p.skip_events = options.skip_events
p.max_events = options.nevents

# Library containing processors
p.add_library("libprocessors")

###############################
#          Processors         #
###############################

rawAnaSvt = HpstrConf.Processor('svtana', 'SvtRawDataAnaProcessor')

###############################
#   Processor Configuration   #
###############################

#SvtRawAnaData

rawAnaSvt.parameters["debug"] = 0
rawAnaSvt.parameters["anaName"] = 'rawSvtHitAna'
rawAnaSvt.parameters["trkrHitColl"] = 'SVTRawTrackerHits'  # 'SVTRawHitsOnTrack_KF'#'SVTRawTrackerHits'
rawAnaSvt.parameters["histCfg"] = os.environ['HPSTR_BASE']+'/analysis/plotconfigs/svt/rawSvtAnaHits.json'
rawAnaSvt.parameters["sample"] = 0

RegionPath = os.environ['HPSTR_BASE']+"/analysis/selections/svtHit/"

rawAnaSvt.parameters["baselineFile"] = os.environ['HPSTR_BASE']+"/processors/dat/hps_14552_offline_baselines.dat"
rawAnaSvt.parameters["timeProfiles"] = os.environ['HPSTR_BASE'] + "/processors/dat/hpssvt_014393_database_svt_pulse_shapes_final.dat"

rawAnaSvt.parameters["regionDefinitions"] = [RegionPath+'OneFit.json',
                                             RegionPath+'FirstFit.json',
                                             RegionPath+'SecondFit.json',
                                             RegionPath+'BothFit.json',
                                             RegionPath+'CTFit.json',
                                             RegionPath+'FTFit.json',
                                             RegionPath+'LowTimeDiff.json',
                                             RegionPath+'R1.json',
                                             RegionPath+'R2.json',
                                             RegionPath+'R3.json',
                                             RegionPath+'R4.json',
                                             RegionPath+'TimeResolution.json'
                                             ]

rawAnaSvt.parameters["MatchList"] = ['OneFit', 'CTFit', 'SecondFit']
rawAnaSvt.parameters["timeref"] = 0.0
rawAnaSvt.parameters["ampref"] = 0.0

#os.environ['HPSTR_BASE']+'/analysis/plotconfigs/reco/basicRecoHit.json'
#os.environ['HPSTR_BASE']+'/analysis/plotconfigs/svt/Svt2DBl.json'

# Sequence which the processors will run.
p.sequence = [rawAnaSvt]

p.input_files = [root1_file]
p.output_files = [root2_file]

p.printProcess()
