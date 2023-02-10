#!/bin/usr/python3
import ROOT as r
import numpy as np

outfile = r.TFile("zcutpoly.root", "RECREATE")
outfile.cd()
#x = np.array(np.arange(50.0,210.0,10.))
x = np.array([50., 60., 70., 80., 90., 100., 110., 120., 130., 140., 150., 160., 170., 190., 210.])
x = x/1000
y = np.array([20.33, 15.67, 14.64, 12.33, 11.72, 12.21, 10.75, 8.46, 9.87, 8.8, 8.59, 10.16, 9.57, 8.49, 9.07])
print(x)
print(y)
g = r.TGraph(len(x), x, y)
fitResult = g.Fit('pol4', "ES")
g.Draw()
g.Write()


'''Minimizer is Minuit / Migrad
Chi2                      =      69.4036
NDf                       =            9
Edm                       =  8.97768e-10
NCalls                    =         6234
p0                        =      345.965   +/-   3.083         0            +0            (Minos)
p1                        =     -17.9537   +/-   0.0493889     0            +0            (Minos)
p2                        =     0.391391   +/-   0.000320939   0            +0            (Minos)
p3                        =  -0.00437471   +/-   1.91224e-06   0            +0            (Minos)
p4                        =  2.62775e-05   +/-   1.05717e-08   0            +0            (Minos)
p5                        = -8.04293e-08   +/-   5.29544e-11   0            +0            (Minos)
p6                        =  9.82938e-11   +/-   2.11067e-13   0            +0            (Minos)
'''


'''
Minimizer is Minuit / Migrad
Chi2                      =      8.37506
NDf                       =           10
Edm                       =   2.7411e-17
NCalls                    =         2945
p0                        =      51.1503   +/-   1.10125       -13.482      +13.482       (Minos)
p1                        =    -0.998485   +/-   0.0191484     0            +0            (Minos)
p2                        =   0.00912254   +/-   0.000128601   0            +0            (Minos)
p3                        = -3.75679e-05   +/-   7.71042e-07   0            +0            (Minos)
p4                        =  5.81857e-08   +/-   2.71481e-09   0            +0            (Minos)
'''
