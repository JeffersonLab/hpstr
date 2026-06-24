import HpstrConf
import sys
import os
import baseConfig as base

base.parser.add_argument(
        '--sample', choices=['data', 'sim_bkgd', 'ap_signal', 'simp_signal'],
        help='Which type of sample this is', required=True
)
base.parser.add_argument(
        '--apPDG', type=int, default=622,
        help="PDG of the A' (622 displaced, 623 prompt). Ignored for SIMP."
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

if options.isData and not options.sample == 'data':
    raise ValueError('If running on data, sample must be "data"')
if not options.isData and options.sample == 'data':
    raise ValueError('If running on MC, sample cannot be "data", use "sim_bkgd", "ap_signal" or "simp_signal"')

cluseff = HpstrConf.Processor('cluseff', 'ClusterEfficiency2021')
cluseff.parameters["isData"] = options.isData
cluseff.parameters["isApSignal"] = 1 if ('ap' in options.sample) else 0
cluseff.parameters["isSimpSignal"] = 1 if ('simp' in options.sample) else 0
cluseff.parameters["apPDG"] = options.apPDG
cluseff.parameters["vtxCollection"] = "UnconstrainedV0Vertices_KF"
cluseff.parameters["calTimeOffset"] = 37.3
cluseff.parameters["clusterEnergyThresh"] = 0.0
#cluseff.parameters["debug"] = 1

p.sequence = [cluseff]

p.input_files = infile
p.output_files = [outfile]

p.printProcess()
