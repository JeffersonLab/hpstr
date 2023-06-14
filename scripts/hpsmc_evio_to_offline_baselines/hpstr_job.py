from hpsmc.tools import HPSTR

job.description = 'HPSTR recon and analysis'

# Convert LCIO to ROOT
cnv = HPSTR(run_mode=1, cfg='hh')

# Run an analysis on the ROOT file
ana = HPSTR(run_mode=1, cfg='ana')

job.add([cnv])
