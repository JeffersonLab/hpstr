import numpy as np
from scipy import interpolate

e137dat = np.loadtxt('./limits/E137.dat')
e137interp = interpolate.interp1d(e137dat[:, 0], e137dat[:, 1], kind='linear')


def _e137lim(aD, mAp, br):
    aD0 = 0.1
    if mAp > 0.949:
        return 1.
    return e137interp(mAp)*np.power(aD0/aD, 0.25)*np.power(1./br, 0.25)


e137lim = np.vectorize(_e137lim)

lsnddat = np.loadtxt('./limits/LSND.dat')
lsndinterp = interpolate.interp1d(lsnddat[:, 0], lsnddat[:, 1], kind='linear')


def _lsndlim(aD, mAp, mDM, br):
    mA_low = 0.00075*3
    mA_high = 0.04017*3
    mPi0 = 0.135

    if mAp < mA_low or mAp > mA_high or mAp > mPi0 or mDM > mA_high/3 or mDM < mA_low/3.:
        return 1.

    aD0 = 0.5
    mPiOvermA0 = 1/3.
    mPiOvermA = mDM/mAp

    ylim = lsndinterp(mDM)
    eps0 = np.sqrt(ylim)*np.sqrt(1./aD0)*np.power(1./mPiOvermA0, 2.)

    return eps0*np.power(aD0/aD, 0.25)*np.power(1/br, 0.25)


lsndlim = np.vectorize(_lsndlim)

miniboonedat = np.loadtxt('./limits/miniboone.dat')
minibooneinterp = interpolate.interp1d(miniboonedat[:, 0], miniboonedat[:, 1], kind='linear')


def _miniboonelim(aD, mAp, mDM, br):
    mA_low = 8e-3*3
    mA_high = 3.25e-1*3

    if mAp < mA_low or mAp > mA_high or mDM > mA_high/3 or mDM < mA_low/3.:
        return 1.

    aD0 = 0.5
    mPiOvermA0 = 1/3.
    mPiOvermA = mDM/mAp

    ylim = minibooneinterp(mDM)
    eps0 = np.sqrt(ylim)*np.sqrt(1./aD0)*np.power(1./mPiOvermA0, 2.)

    return eps0*np.power(aD0/aD, 0.25)*np.power(1./br, 0.25)


miniboonelim = np.vectorize(_miniboonelim)
