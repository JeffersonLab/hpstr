#!/usr/bin/python
import ROOT as r
import baseConfig as base
import math
import numpy as np

options = base.parser.parse_args()

inFile = r.TFile(options.inFilename[0])
outFile = open(options.outFilename+'.dat','w')
outRootFile = r.TFile(options.outFilename+'_ana.root','RECREATE')

bls = {}
encs = {}

roChs = {}
for ch in range(128):
    roChs[32*(ch%4)+8*math.floor(ch/4)-31*math.floor(ch/16)] = ch
#print(roChs)

for feb in range(10):
    bls[feb] = {}
    encs[feb] = {}
    for hyb in range(4):
        bls[feb][hyb] = []
        encs[feb][hyb] = []
        hits_hh = inFile.Get(f'raw_hits_F{feb}H{hyb}_SvtHybrids_s0_hh')
        for ss in range(1,6):
            hits_hh.Add(inFile.Get(f'raw_hits_F{feb}H{hyb}_SvtHybrids_s{ss}_hh'))
            pass
        hits_hh.Write()
        napv = 5
        if feb < 2:
            napv = 4
        nch = napv*128
        chs = []
        for ch in range(nch):
            roCh = roChs[ch%128] + 128*math.floor(ch/128)
            chs.append(float(ch))
            if ch%128 == 0: outFile.write('%i %i %i'%(feb, hyb, math.floor(ch/128)))
            #ch_hits_h = hits_hh.ProjectionY(f'ch{ch}_hits_F{feb}H{hyb}_h', roCh+1, roCh+1)
            ch_hits_h = hits_hh.ProjectionY(f'ch{ch}_hits_F{feb}H{hyb}_h', ch+1, ch+1)
            fit_r = ch_hits_h.Fit('gaus','QS')
            if(ch_hits_h.GetEntries() > 0): 
                #if False:
                if feb == 5 and hyb == 1:
                    outFile.write( ' '+ format(math.floor(fit_r.Parameter(1)+1.5*fit_r.Parameter(2)) ,'x') + ' ' )
                else:
                    outFile.write( ' '+ format(math.floor(fit_r.Parameter(1)+2.5*fit_r.Parameter(2)) ,'x') + ' ' )
                bls[feb][hyb].append(float(fit_r.Parameter(1)))
                encs[feb][hyb].append(float(fit_r.Parameter(2)))
            else: 
                outFile.write(' ffff') 
                bls[feb][hyb].append(0.0)
                encs[feb][hyb].append(0.0)
            if ch%128 == 127: outFile.write('\n')
            pass
        bl_g = r.TGraph(len(chs), np.array(chs), np.array(bls[feb][hyb]))
        bl_g.SetName(f'blF{feb}H{hyb}_g')
        bl_g.Write()
        enc_g = r.TGraph(len(chs), np.array(chs), np.array(encs[feb][hyb]))
        enc_g.SetName(f'encF{feb}H{hyb}_g')
        enc_g.Write()
        pass
    pass

outRootFile.Close()
outFile.close()
