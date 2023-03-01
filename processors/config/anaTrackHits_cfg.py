import HpstrConf
import os
import sys
import baseConfig as base

options = base.parser.parse_args()

# Use the input file to set the output file name
inFilename = options.inFilename
outFilename = options.outFilename

print('Input file:  %s' % inFilename)
print('Output file: %s' % outFilename)

p = HpstrConf.Process()

p.run_mode = 1
p.skip_events = options.skip_events
p.max_events = options.nevents


#p.max_events = 1000

# Library containing processors
p.add_library("libprocessors")

###############################
#          Processors         #
###############################
anaTrks = HpstrConf.Processor('anaTrks', 'TrackHitAnaProcessor')

###############################
#   Processor Configuration   #
###############################
anaTrks.parameters["debug"] = 0
anaTrks.parameters["trkCollName"] = 'KalmanFullTracks'
#anaTrks.parameters["trkCollName"] = 'GBLTracks'
anaTrks.parameters["histCfg"] = os.environ['HPSTR_BASE']+'/analysis/plotconfigs/tracking/trackHit.json'
anaTrks.parameters["selectionjson"] = os.environ['HPSTR_BASE']+'/analysis/selections/trackHit/trackHitAna.json'

RegionPath = os.environ['HPSTR_BASE']+"/analysis/selections/trackHit/"
anaTrks.parameters["regionDefinitions"] = [RegionPath+'hc15_1111.json',
                                           RegionPath+'hc14_1110.json',
                                           RegionPath+'hc13_1101.json',
                                           RegionPath+'hc12_1100.json',
                                           RegionPath+'hc11_1011.json',
                                           RegionPath+'hc10_1010.json',
                                           RegionPath+'hc9_1001.json',
                                           RegionPath+'hc8_1000.json',
                                           RegionPath+'hc7_0111.json',
                                           RegionPath+'hc6_0110.json',
                                           RegionPath+'hc5_0101.json',
                                           RegionPath+'hc4_0100.json',
                                           RegionPath+'hc3_0011.json',
                                           RegionPath+'hc2_0010.json',
                                           RegionPath+'hc1_0001.json',
                                           RegionPath+'hc0_0000.json',
                                           RegionPath+'noHCreq.json'
                                           ]

# Sequence which the processors will run.
p.sequence = [anaTrks]

p.input_files = inFilename
p.output_files = [outFilename]

p.printProcess()
