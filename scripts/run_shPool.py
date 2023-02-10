#!/bin/env python


#Author: cbravo (main), SLAC
#      : pf           , SLAC


import glob


def runCommand(cmd, log=None):
    """
    Provides a wrapper around the subprocess.call command
    cmd should be an array of the form: ["command","arg1",...]
    log should specify a logfile to write to
    """

    import datetime
    import os
    import sys
    import subprocess
    from subprocess import CalledProcessError

    try:
        msg = "Executing command:"
        for c in cmd:
            msg += " %s" % (c)
        sys.stdout.flush()
        returncode = subprocess.call(cmd, stdout=log, stderr=log)
    except CalledProcessError as e:
        msg = "Caught exception"
        msg += str(e)
        msg += " running:"
        for c in cmd:
            msg += " %s" % (c)
        sys.stdout.flush()
        pass
    return returncode


def launchTests(args):
    return launchTestsArgs(*args)


def launchTestsArgs(cmdString, logDir, jobN):
    import datetime
    import os
    import sys
    import subprocess
    from subprocess import CalledProcessError

    startTime = datetime.datetime.now().strftime("%Y.%m.%d.%H.%M")

    #Build Commands
    setupCmds = []
    preCmd = None
    logfilename = logDir+"/job%i.log" % jobN
    cmd = cmdString.split()

    #Execute Commands
    try:
        for setupCmd in setupCmds:
            runCommand(setupCmd)
            pass
        log = file(logfilename, "w")
        if preCmd and config:
            runCommand(preCmd, log)
            pass
        runCommand(cmd, log)
        print("%i. Finished %s" % (jobN, cmdString))
    except CalledProcessError as e:
        print "Caught exception", e
        pass
    return


if __name__ == '__main__':

    import sys
    import os
    import signal
    import subprocess
    import itertools
    from multiprocessing import Pool, freeze_support
    from optparse import OptionParser

    parser = OptionParser()
    parser.add_option("-f", "--fileList", type="string", dest="fileList",
                      help="List of files to run on.", metavar="fileList", default="jobList.txt")
    parser.add_option("-m", "--fileMod", type="string", dest="fileMod",
                      help="Modifier for output file names.", metavar="fileMod", default="_hpstr")
    parser.add_option("-r", "--rootFile", action="store_true", dest="rootFile",
                      help="Running on root files and not LCIO files", metavar="rootFile")
    parser.add_option("-d", "--debug", action="store_true", dest="debug",
                      help="print extra debugging information", metavar="debug")
    parser.add_option("-p", "--pool", type="int", dest="poolSize",
                      help="Set the number of cores to use (pool size)", metavar="poolSize", default=10)
    parser.add_option("-l", "--logDir", type="string", dest="logDir",
                      help="Specify the directory to save the log files", metavar="logDir", default="./")
    parser.add_option("-i", "--inDir", type="string", dest="inDir",
                      help="Specify the input directory with the files", metavar="inDir", default="")
    parser.add_option("-z", "--isData", type="int", dest="isData",
                      help="Specify if the input file is data or MC", metavar="isData", default=0)

    (options, args) = parser.parse_args()

    jobListFile = open(options.fileList)
    shList = []
    for line in jobListFile:
        shList.append(line.rstrip())
        pass

    if options.debug:
        print "Testing %i jobs in parallel mode (using Pool(%i))" % (len(shList), options.poolSize)
        print list(
                  itertools.izip(shList,
                                 [options.logDir for x in range(len(shList))],
                                 range(len(shList))
                                 )
                  )
    else:
        print "Running %i jobs in parallel mode (using Pool(%i))" % (len(shList), options.poolSize)
        freeze_support()
        # from: https://stackoverflow.com/questions/11312525/catch-ctrlc-sigint-and-exit-multiprocesses-gracefully-in-python
        original_sigint_handler = signal.signal(signal.SIGINT, signal.SIG_IGN)
        pool = Pool(options.poolSize)
        signal.signal(signal.SIGINT, original_sigint_handler)
        try:
            res = pool.map_async(launchTests,
                                 itertools.izip(shList,
                                                [options.logDir for x in range(len(shList))],
                                                range(len(shList))
                                                )
                                 )
            # timeout must be properly set, otherwise tasks will crash
            print res.get(999999999)
            print("Normal termination")
            pool.close()
            pool.join()
        except KeyboardInterrupt:
            print("Caught KeyboardInterrupt, terminating workers")
            pool.terminate()
        except Exception as e:
            print("Caught Exception %s, terminating workers" % (str(e)))
            pool.terminate()
        except BaseException:  # catch *all* exceptions
            e = sys.exc_info()[0]
            print("Caught non-Python Exception %s" % (e))
            pool.terminate()
