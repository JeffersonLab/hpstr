import HpstrConf
import sys
import os
import baseConfig as base


base.parser.add_argument("-w", "--tracking", type=str, dest="tracking",
                         help="Which tracking to use to make plots", metavar="tracking", default="KF")
options = base.parser.parse_args()


print(options)

# Use the input file to set the output file name
infile = options.inFilename
outfile = options.outFilename

analysis = options.analysis


print('Input file: %s' % infile)
print('Output file: %s' % outfile)
print('Analysis : %s' % analysis)

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

sim_part_ana = HpstrConf.Processor('sim_part', 'SimPartAnaProcessor')

###############################
#   Processor Configuration   #
###############################
#RecoHitAna
sim_part_ana.parameters["debug"] = 0
sim_part_ana.parameters["anaName"] = "sim_part"
sim_part_ana.parameters["MCParticleColl"] = "MCParticle"
sim_part_ana.parameters["MCTrackerHitColl"] = "TrackerSimHits"
sim_part_ana.parameters["MCTrackerHitECalColl"] = "TrackerSimHitsECal"
sim_part_ana.parameters["MCEcalHitColl"] = "EcalSimHits"
sim_part_ana.parameters["RecoTrackColl"] = "KalmanFullTracks"
sim_part_ana.parameters["RecoTrackColl_AtLastHit"] = "KalmanFullTracks_AtLastHit"
sim_part_ana.parameters["RecoTrackerClusterColl"] = "SiClustersOnTrack"
sim_part_ana.parameters["RecoEcalClusterColl"] = "RecoEcalClusters"
sim_part_ana.parameters["analysis"] = analysis
sim_part_ana.parameters["histCfg"] = os.environ['HPSTR_BASE']+'/analysis/plotconfigs/mc/simPart.json'

sim_part_ana.parameters["selectionjson"] = os.environ['HPSTR_BASE']+'/analysis/selections/simPart/simPartAna.json'
RegionPath = os.environ['HPSTR_BASE']+"/analysis/selections/simPart/"
sim_part_ana.parameters["regionDefinitions"] = [RegionPath+'1Sim_1Track.json',
                                                RegionPath+'Track.json',
                                                RegionPath+'Ecal.json',
                                                RegionPath+'1Track_1Ecal.json',
                                                RegionPath+'1Sim_1Track_1Ecal.json',
                                                RegionPath+'Track_Ecal.json',
                                                RegionPath+'noTrack.json',
                                                RegionPath+'noTrack_Ecal.json',
                                                RegionPath+'noEcal.json',
                                                RegionPath+'Track_noEcal.json',
                                                RegionPath+'noTrack_noEcal.json',
                                                RegionPath+'1Track_pos_Omega.json',
                                                RegionPath+'1Track_neg_Omega.json',
                                                RegionPath+'Track_nhit7.json',
                                                RegionPath+'Track_nhit8.json',
                                                RegionPath+'Track_nhit9.json',
                                                RegionPath+'Track_nhit10.json',
                                                RegionPath+'Track_nhit11.json',
                                                RegionPath+'Track_nhit12.json',
                                                RegionPath+'Track_nhit13.json',
                                                RegionPath+'Track_nhit14.json',
                                                RegionPath+'Track_Ecal_Track_p_ge4.json',
                                                RegionPath+'Track_Ecal_Sim_p_ge1.json',
                                                RegionPath+'Track_Ecal_Sim_p_le1.json',
                                                RegionPath+'Track_noEcal_lowtanlambda.json',
                                                RegionPath+'Track_top.json',
                                                RegionPath+'Track_bottom.json',
                                                RegionPath+'noTrack_Ecal_top.json',
                                                RegionPath+'noTrack_Ecal_bottom.json',
                                                RegionPath+'1Sim_1Track_1Ecal_Track_last_layer_0_top.json',
                                                RegionPath+'1Sim_1Track_1Ecal_Track_last_layer_1_top.json',
                                                RegionPath+'1Sim_1Track_1Ecal_Track_last_layer_2_top.json',
                                                RegionPath+'1Sim_1Track_1Ecal_Track_last_layer_3_top.json',
                                                RegionPath+'1Sim_1Track_1Ecal_Track_last_layer_4_top.json',
                                                RegionPath+'1Sim_1Track_1Ecal_Track_last_layer_5_top.json',
                                                RegionPath+'1Sim_1Track_1Ecal_Track_last_layer_6_top.json',
                                                RegionPath+'1Sim_1Track_1Ecal_Track_last_layer_7_top.json',
                                                RegionPath+'1Sim_1Track_1Ecal_Track_last_layer_8_top.json',
                                                RegionPath+'1Sim_1Track_1Ecal_Track_last_layer_9_top.json',
                                                RegionPath+'1Sim_1Track_1Ecal_Track_last_layer_10_top.json',
                                                RegionPath+'1Sim_1Track_1Ecal_Track_last_layer_11_top.json',
                                                RegionPath+'1Sim_1Track_1Ecal_Track_last_layer_12_top.json',
                                                RegionPath+'1Sim_1Track_1Ecal_Track_last_layer_13_top.json',
                                                RegionPath+'1Sim_1Track_1Ecal_Track_last_layer_0_bottom.json',
                                                RegionPath+'1Sim_1Track_1Ecal_Track_last_layer_1_bottom.json',
                                                RegionPath+'1Sim_1Track_1Ecal_Track_last_layer_2_bottom.json',
                                                RegionPath+'1Sim_1Track_1Ecal_Track_last_layer_3_bottom.json',
                                                RegionPath+'1Sim_1Track_1Ecal_Track_last_layer_4_bottom.json',
                                                RegionPath+'1Sim_1Track_1Ecal_Track_last_layer_5_bottom.json',
                                                RegionPath+'1Sim_1Track_1Ecal_Track_last_layer_6_bottom.json',
                                                RegionPath+'1Sim_1Track_1Ecal_Track_last_layer_7_bottom.json',
                                                RegionPath+'1Sim_1Track_1Ecal_Track_last_layer_8_bottom.json',
                                                RegionPath+'1Sim_1Track_1Ecal_Track_last_layer_9_bottom.json',
                                                RegionPath+'1Sim_1Track_1Ecal_Track_last_layer_10_bottom.json',
                                                RegionPath+'1Sim_1Track_1Ecal_Track_last_layer_11_bottom.json',
                                                RegionPath+'1Sim_1Track_1Ecal_Track_last_layer_12_bottom.json',
                                                RegionPath+'1Sim_1Track_1Ecal_Track_last_layer_13_bottom.json'
                                           ]

# Sequence which the processors will run.
p.sequence = [sim_part_ana]

p.input_files = infile
p.output_files = [outfile]

p.printProcess()
