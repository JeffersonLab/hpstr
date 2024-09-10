#!/usr/bin/env python3
# coding: utf-8

# # $\bar{C}_\text{max}(1 - \alpha, \mu)$

# Computing $\bar{C}_\text{max}(C, \mu)$ for optimum interval calculation, where $\mu$ is the number of expected events and $1 - \alpha$ is how frequently you reject the null hypothesis when it is true.

# The <b>single-event energy spectrum</b>, that is, the probability density function which tells us which energy depositions are likely to occur, is independent of the chosen WIMP model -- we always expect a simple exponential recoil spectrum.
# 
# The <b>number of dark matter events</b> detected does depend on the WIMP mass and cross-section. We know, however, that it must follow a Poisson distribution, which leaves the Poisson mean (which equals the expected number of events) as the only parameter left to estimate. From an upper limit on this mean, an upper limit curve in the dark matter mass – cross-section plane can be computed.
# 
# *	A <b>list_of_energies</b> list of reconstructed energy depositions of single events (from here on simply ‘energies’), either measured during some run of an actual detector, or generated using Monte Carlo.)
# *	An <b>interval</b> is an interval in energy space.
# *	The <b>size</b> of an interval is the fraction of energies expected in that interval. Clearly, this depends on which energy spectrum we assume, but is independent of the Poisson mean we are trying to constrain. By definition this is a number between 0 and 1.
# *	The <b>K-largest</b> interval of a run is the largest interval containing K events in that run. Recall our definition of size: a ‘large’ interval is one which is unusually empty in that run. Clearly k-largest intervals will terminate at (or technically, just before) an observed energy, or at one of the boundaries of our energy space. Again, which interval in a run is the k–largest, depends on our energy spectrum, but not on our Poisson mean.
# *	The <b>extremeness</b> of a K-largest interval is the probability of finding the K-largest interval in a run to be smaller. This clearly does depend on the Poisson mean: if we expect very few events, large gap sizes are more likely. Clearly extremeness is a number between 0 and 1; values close to 1 indicate unusually large intervals, that is, usually large (almost-)empty regions in the measured energies.
#  For example, if the extremeness of a k-largest interval in a run is 0.8, that means that 80% of runs have k-largest intervals which are smaller than the k-largest interval in this run.
# *	The <b>optimum interval statistic</b> of a run is extremity of the most extreme k-largest interval in a run.
# *	The <b>extremeness</b> of the optimum interval statistic is the probability of finding a lower optimum interval statistic, that is, of finding the optimum interval in a run to be less extreme. 
# 
# The <b>max gap method</b> rejects a theory (places a mean outside the upper limit) based on a run if the 0-largest interval (the largest gap) is too extreme. 
# 
# The <b>optimum interval</b> method rejects a theory based on a run if the optimum interval statistic is too large.
# 
# * The <b>energy cumulant</b> $\epsilon(E)$ is the fraction of energies expected below the energy $E$. Whatever the (1-normalized) energy distribution $dN/dE$, $dN/d\epsilon$ is uniform[0,1], where $0$ and $1$ correspond to the boundaries of our experimental range.
# 
# 

import functools
from scipy.optimize import brenth
import matplotlib.pyplot as plt
import numpy as np
import pickle
import os
import sys
import time


# Disable
def blockPrint():
    sys.stdout = open(os.devnull, 'w')

# Restore
def enablePrint():
    sys.stdout = sys.__stdout__

def kLargestIntervals(list_of_energies, spectrumCDF = lambda x: x):
    """
    Returns a list of the sizes of the K-largest intervals in that run according to the energy spectrum (given as a CDF).
    That is, kLargestIntervals(...)[i] is the size of the largest interval containing i events, where ‘largest’ is defined above.
    
    * Transform energies to energy cumulants
    * Add events at 0 and 1
    * Foreach k, compute interval sizes, take max
    """
    answer = {}
    
    list_of_energies.sort()
    #print('list of energies', list_of_energies)
    
    energy_cumulants = spectrumCDF(list_of_energies)
    #print('cumulants', energy_cumulants)
    
    for i,interval_size in enumerate(range(len(energy_cumulants))):
        if (1 + interval_size) >= len(energy_cumulants):
            continue
            
        temp_data = energy_cumulants.copy()
        gap_sizes = temp_data[(1+interval_size):] - temp_data[0:-1*(1 + interval_size)] 
        #print(f'gap sizes in interval_size {i+1}:', gap_sizes)

        answer[interval_size] = np.max(gap_sizes)
    return answer

def extremenessOfInterval(x, k, mu):
    """
    Returns the extremeness of a k-largest interval of size, if the poisson mean is mu.
    
    (Number of itvSizes[mu][k] smaller than size) / mcTrials[mu]
    
    x - also size in above comment
    k - gap (rename k)
    """
    #print('extremenessOfInterval')
    #print(f'mu is {mu}')
    # [0] is because where returns list, where [0] is answer
    if k not in itvSizes[mu]:
        return 0
    ##print('extremeness of x:', x)
    #print(f'mu is {mu}')
    #print(f'k is {k}')
    #print('mcTrials[mu]:', mcTrials[mu])
    #print(itvSizes[mu][k])
    #print(np.where(itvSizes[mu][k] < x)[0])
    #print(f'number of gaps with size less than {x}:', np.where(itvSizes[mu][k] < x)[0].size)
    #print(f'fraction of gaps less than {x}:', np.where(itvSizes[mu][k] < x)[0].size / mcTrials[mu])
    return np.where(itvSizes[mu][k] < x)[0].size / mcTrials[mu]


def optimumItvStatistic(list_of_energies, mu, spectrumCDF = lambda x: x):
    """
    Returns the optimum interval statistic of the run.
    
    Max of extremenssOfInterval's
    """
    #print('running optimumItvStatistic')
    #print(f'mu is {mu}')
    #print(f'list of energies: {list_of_energies}')
    #print('check klargest intervals:', kLargestIntervals(list_of_energies, spectrumCDF).items())
    #print('max is', np.max([extremenessOfInterval(x, k, mu) for k, x in kLargestIntervals(list_of_energies, spectrumCDF).items()]))
    return np.max([extremenessOfInterval(x, k, mu) for k, x in kLargestIntervals(list_of_energies, spectrumCDF).items()])


def extremenessOfOptItvStat(stat, mu):
    """
    Returns the extremeness of the optimum interval statistic stat, given mu
    
    (Number of optItvs[mu] smaller than stat) / mcTrials[mu]
    """
    #print('extremenessOfOptItvStat')
    #print(f'mu is {mu}')
    return np.where(optItvs[mu] < stat)[0].size / mcTrials[mu]

def optItvUpperLimit(list_of_energies, c, spectrumCDF = lambda x: x,
                     n = 1000):
    """
    Returns the c- confidence upper limit on mu using optimum interval
    
    For which mu is extremenessOfOptItvStat( optimumItvStatistic(run), mu ) = c
    
    c - e.g., 0.9
    """
    def f(mu, list_of_energies, c, spectrumCDF, n):
        generate_table(mu, n)
        x = optimumItvStatistic(list_of_energies, mu, spectrumCDF)
        prob = extremenessOfOptItvStat(x, mu)
        return prob - c

    mu = 0

    for mu in np.arange(10, 2 * list_of_energies.size):
        if f(mu, list_of_energies, c, spectrumCDF, n) > 0:
            #print('Found seed mu=%f' % mu)
            break
        
    try:
        xsec = brenth(f, mu - 5, mu + 5,
                  args=(list_of_energies, c, spectrumCDF, n),
                  xtol=1e-2) 
        #print('Improved xsec:', xsec)
    except:
        #print("ERROR: could not minimize", mu)
        return mu
    return xsec

def generate_trial_experiment(mu, n):
    #print(f'Generate Trial Experiment with mu:{mu} and n{n}')
    trials = []

    for index in range(n):
        this_mu = np.random.poisson(mu)
        #print(f'this_mu: {this_mu}')
        rand_numbers = np.random.random(size=this_mu)
        #print(f'rand numbers, {rand_numbers}')
        rand_numbers = np.append(rand_numbers, [0.0, 1.0])
        rand_numbers.sort()
        #print(f'rand numbers, {rand_numbers}')
        trials.append(rand_numbers)

    return trials


# ## Monte Carlo for populating itvSizes[$\mu$][$k$] and optItvs[$\mu$]


def get_filename():
    #return '/fs/ddn/sdf/group/hps/users/alspellm/mc_storage/opt_int_lookuptable_max50_10ktoys.p'
    #return '/fs/ddn/sdf/group/hps/users/alspellm/mc_storage/opt_int_lookuptable_max25_100ktoys.p'
    #return '/fs/ddn/sdf/group/hps/users/alspellm/mc_storage/opt_int_lookuptable_max25_100ktoys_0.05steps.p'
    return '/fs/ddn/sdf/group/hps/users/alspellm/mc_storage/opt_int_lookuptable_max25_10ktoys_0.05steps_v2.p'

def load_table_from_disk():
    global itvSizes
    global optItvs
    global mcTrials
    
    if os.path.exists(get_filename()):
        f = open(get_filename(), 'rb')
        itvSizes = pickle.load(f)
        optItvs = pickle.load(f)
        mcTrials = pickle.load(f)
        f.close()

    
def write_table_to_disk():
    f = open(get_filename(), 'wb')
    pickle.dump(itvSizes, f)
    pickle.dump(optItvs, f)
    pickle.dump(mcTrials, f)
    f.close()

itvSizes = {}
optItvs = {}
mcTrials = {}
load_table_from_disk()    

def generate_table_new(mu,n):
    """    #Generate trial runs"""    
    if mu in mcTrials and mcTrials[mu] >= n:
        return

    mcTrials[mu] = n
    itvSizes[mu] = {}
    optItvs[mu] = []

    mu_trials = np.random.poisson(mu, size=n)
    trials = [np.sort(np.append(np.random.random(mu),[0.0,1.0])) for mu in mu_trials]

    for i,trial in enumerate(trials):
    
        intermediate_result = kLargestIntervals(trial)
        for k, v in intermediate_result.items():
            if k not in itvSizes[mu]:
                itvSizes[mu][k] = []

            itvSizes[mu][k].append(v)
    #print('interm results')
    #print(itvSizes)
    # Numpy-ize it
    for k, array in itvSizes[mu].items():
        itvSizes[mu][k] = np.array(array)
    
    for i,trial in enumerate(trials):
        #print('\n')
        #print(f'trial {i}: {trial}')
        #print(f'mu is {mu}')
        optItvs[mu].append(optimumItvStatistic(trial, mu))
        #print('trial result: ', optItvs[mu])
        #print(f'trial result: {optimumItvStatistic(trial, mu)}')
        #print('\n')
        
    #print('summarize trials')
    
    # Numpy-ize it
    optItvs[mu] = np.array(optItvs[mu])
    #print(optItvs[mu])
    
def generate_table(mu, n):
    """    #Generate trial runs"""    
    if mu in mcTrials and mcTrials[mu] >= n:
        return

    #print("Generating mu=", mu)

    mcTrials[mu] = n
    trials = generate_trial_experiment(mu, mcTrials[mu])
    #print(trials[0:10])
    itvSizes[mu] = {}
    optItvs[mu] = []

    for i,trial in enumerate(trials):
    
        intermediate_result = kLargestIntervals(trial)
        for k, v in intermediate_result.items():
            if k not in itvSizes[mu]:
                itvSizes[mu][k] = []

            itvSizes[mu][k].append(v)
    #print('interm results')
    #print(itvSizes)
    # Numpy-ize it
    for k, array in itvSizes[mu].items():
        itvSizes[mu][k] = np.array(array)
    
    for i,trial in enumerate(trials):
        #print('\n')
        #print(f'trial {i}: {trial}')
        #print(f'mu is {mu}')
        optItvs[mu].append(optimumItvStatistic(trial, mu))
        #print('trial result: ', optItvs[mu])
        #print(f'trial result: {optimumItvStatistic(trial, mu)}')
        #print('\n')
        
    #print('summarize trials')
    
    # Numpy-ize it
    optItvs[mu] = np.array(optItvs[mu])
    #print(optItvs[mu])
    


def cache_values(my_max=200, n=100):
    for i in range(3, my_max):
        generate_table(i, n)
    write_table_to_disk()


import time
def plot_something():
    x, y = [], []
    
    for i,mu in enumerate(np.linspace(0.0, 200.0,1000)):
        start_time = time.time()
        generate_table(mu, 10000)
        x.append(mu)
        a = brenth(lambda x: extremenessOfOptItvStat(x, mu) - 0.9,
                   0,
                   1,
                   xtol=1e-2)
        
        y.append(a)
          
        end_time = time.time()
        print('time: ', end_time - start_time)
        
    plt.plot(x,y)
    plt.xscale('log')
    plt.xlim(0.0, 200.0)
  
    
#plot_something()
total_time = 0.0
for i,mu in enumerate(np.linspace(0.0, 25.0,500)):
    print(f'Running mu = {mu} | {100.*i/500}% complete')
    start_time = time.time()
    generate_table(mu, 10000)
    #generate_table_new(mu, 100000)
    end_time = time.time()
    elapsed_time = end_time - start_time
    total_time = total_time + elapsed_time
    avg_time = total_time/(i+1)
    print(f'Time to generate table entry: {elapsed_time}')
    print(f'Average time: {avg_time}')
    print(f'Estimated completion: {avg_time*(500-i+1)}')

write_table_to_disk()






