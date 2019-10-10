import HpstrConf
import sys
p = HpstrConf.Process()

# Library containing processors
p.libraries.append("libprocessors.so")

#Processors

clusters = HpstrConf.Processor('clusters','ClusterOnTrackProcessor')

p.sequence = [clusters]

p.input_files    = [
    "/nfs/slac/g/hps2/pbutti/hps_data2/hps_010487/recon/hps_10487.02292_recon_3fbfd00b3.root"
]

p.output_files  = [
    "hps_10487.02292_hist_3fbfd00b3.root"
]

#p.max_events   = 1000
p.printProcess()

