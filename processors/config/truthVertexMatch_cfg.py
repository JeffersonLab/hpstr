import HpstrConf
import sys
import os
import baseConfig as base

base.parser.add_argument(
        '--sample', choices=['data', 'sim_bkgd', 'ap_signal', 'simp_signal', 'rad'],
        help='Which type of sample this is', required=True
)
base.parser.add_argument(
        '--apPDG', type=int, default=622,
        help="PDG of the A' (622 displaced, 623 prompt). Ignored for SIMP."
)
base.parser.add_argument(
        '--massSelect', type=float, default=-1.0,
        help="Target truth e+e- invariant mass [GeV] to select (e.g. 0.120). <0 disables."
)
base.parser.add_argument(
        '--massWindow', type=float, default=0.002,
        help="Half-width of the truth-mass window [GeV] (default 0.002)."
)
base.parser.add_argument(
        '--vtxZMin', type=float, default=-1.0e9,
        help="Min truth decay-vertex z [mm] to consider (default disabled)."
)
base.parser.add_argument(
        '--vtxZMax', type=float, default=1.0e9,
        help="Max truth decay-vertex z [mm] to consider, e.g. 50 = before first tracking layer."
)
base.parser.add_argument(
        '--eleMomCut', type=float, default=0.5,
        help="Truth e- momentum threshold [GeV] for the secondary category (default 0.5)."
)
base.parser.add_argument(
        '--eleClusEMin', type=float, default=0.2,
        help="Sanity floor [GeV] on the electron leg's ECal cluster energy for has_ele_clus "
             "(default 0.2)."
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

# rad (radiative trident) MC tags its hard e+e- pair with a prompt PDG=623 mother at the
# target, so it is handled by the A' code path with apPDG=623 (truth vertex = mother endpoint).
is_rad = (options.sample == 'rad')

tvm = HpstrConf.Processor('tvm', 'TruthVertexMatch2021')
tvm.parameters["isData"] = options.isData
tvm.parameters["isApSignal"] = 1 if (('ap' in options.sample) or is_rad) else 0
tvm.parameters["isSimpSignal"] = 1 if ('simp' in options.sample) else 0
tvm.parameters["apPDG"] = 623 if is_rad else options.apPDG
tvm.parameters["vtxCollection"] = "UnconstrainedV0Vertices_KF"
tvm.parameters["calTimeOffset"] = 37.3
tvm.parameters["massSelect"] = options.massSelect
tvm.parameters["massWindow"] = options.massWindow
tvm.parameters["vtxZMin"] = options.vtxZMin
tvm.parameters["vtxZMax"] = options.vtxZMax
tvm.parameters["eleMomCut"] = options.eleMomCut
tvm.parameters["eleClusEMin"] = options.eleClusEMin
#tvm.parameters["debug"] = 1

p.sequence = [tvm]

p.input_files = infile
p.output_files = [outfile]

p.printProcess()
