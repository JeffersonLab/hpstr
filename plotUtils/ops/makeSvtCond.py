#!/usr/bin/python
import ROOT as r
import baseConfig as base
import math
import numpy as np

options = base.parser.parse_args()

inFile = r.TFile(options.inFilename[0])
outCondFile = open(options.outFilename+'_cond.dat','w')
condCh = 0
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
        bls[feb][hyb] = {}
        encs[feb][hyb] = {}
        napv = 5
        if feb < 2:
            napv = 4
        nch = napv*128
        for ss in range(6):
            bls[feb][hyb][ss] = []
            encs[feb][hyb][ss] = []
            hits_hh = inFile.Get(f'raw_hits_F{feb}H{hyb}_SvtHybrids_s{ss}_hh')
            chs = []
            for ch in range(nch):
                roCh = roChs[ch%128] + 128*math.floor(ch/128)
                chs.append(float(ch))
                #ch_hits_h = hits_hh.ProjectionY(f'ch{ch}_hits_F{feb}H{hyb}_h', roCh+1, roCh+1)
                ch_hits_h = hits_hh.ProjectionY(f'ch{ch}_hits_F{feb}H{hyb}_h', ch+1, ch+1)
                fit_r = ch_hits_h.Fit('gaus','QS')
                if(ch_hits_h.GetEntries() > 0): 
                    bls[feb][hyb][ss].append(float(fit_r.Parameter(1)))
                    encs[feb][hyb][ss].append(float(fit_r.Parameter(2)))
                else: 
                    bls[feb][hyb][ss].append(0.0)
                    encs[feb][hyb][ss].append(0.0)
                pass # ch
            bl_g = r.TGraph(len(chs), np.array(chs), np.array(bls[feb][hyb]))
            bl_g.SetName(f'blF{feb}H{hyb}_s{ss}_g')
            bl_g.Write()
            enc_g = r.TGraph(len(chs), np.array(chs), np.array(encs[feb][hyb]))
            enc_g.SetName(f'encF{feb}H{hyb}_s{ss}_g')
            enc_g.Write()
            pass # ss
        for ch in range(nch):
            outCondFile.write('%i %f %f %f %f %f %f %f %f %f %f %f %f\n'%(condCh, bls[feb][hyb][0][ch], bls[feb][hyb][1][ch], bls[feb][hyb][2][ch], bls[feb][hyb][3][ch], bls[feb][hyb][4][ch], bls[feb][hyb][5][ch], encs[feb][hyb][0][ch], encs[feb][hyb][1][ch], encs[feb][hyb][2][ch], encs[feb][hyb][3][ch], encs[feb][hyb][4][ch], encs[feb][hyb][5][ch]))
            condCh += 1
            pass
        pass # hyb
    pass # feb

outRootFile.Close()
outCondFile.close()
