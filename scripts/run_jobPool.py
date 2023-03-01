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


def launchTestsArgs(options, infilename, fileN, jobN):
    import datetime
    import os
    import sys
    import subprocess
    from subprocess import CalledProcessError

    startTime = datetime.datetime.now().strftime("%Y.%m.%d.%H.%M")

    #Build Commands
    setupCmds = []
    preCmd = None
    print(options.inDir)
    #filenameBase = "apsignalv2-beamv6_2500kBunches_displaced_10mm_%i"%fileN
    filenameBase = (infilename.split("/")[-1]).split(".")[0]

    #outDir = "/nfs/slac/g/hps3/users/pbutti/hpstr_histos/ap/80MeV/"

    logfilename = options.outDir+"logs/"+filenameBase+"_"+str(fileN)+".log"
    cfgname = ((options.configFile).split("/")[-1]).split(".")[0]
    outfilename = options.outDir+filenameBase+"_"+cfgname+"_"+str(fileN)+".root"
    print("%i. Generating %s" % (jobN, outfilename))
    cmd = [options.tool, options.configFile,
           "-i", infilename,
           "-o", outfilename,
           "-t", str(options.isData),
           options.extraFlags,
           ]
    print(cmd)

    #Execute Commands
    try:
        for setupCmd in setupCmds:
            runCommand(setupCmd)
            pass
        print("Right before log declare")
        log = open(logfilename, "w")
        print("Right after log declare")
        if preCmd and config:
            runCommand(preCmd, log)
            pass
        runCommand(cmd, log)
        print("%i. Finished %s" % (jobN, outfilename))
    except CalledProcessError as e:
        print("Caught exception", e)
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
    parser.add_option("-t", "--tool", type="string", dest="tool",
                      help="Tool to use", metavar="tool", default="hpstr")
    parser.add_option("-c", "--config", type="string", dest="configFile",
                      help="Configuration file to use", metavar="configFile", default="dst.py")
    parser.add_option("-f", "--fileList", type="string", dest="fileList",
                      help="List of files to run on.", metavar="fileList", default="fileList.txt")
    parser.add_option("-m", "--fileMod", type="string", dest="fileMod",
                      help="Modifier for output file names.", metavar="fileMod", default="_hpstr")
    parser.add_option("-r", "--fileExt", dest="fileExt",
                      help="Running on root files and not LCIO files", metavar="fileExt", default="root")
    parser.add_option("-d", "--debug", action="store_true", dest="debug",
                      help="print extra debugging information", metavar="debug")
    parser.add_option("-p", "--pool", type="int", dest="poolSize",
                      help="Set the number of cores to use (pool size)", metavar="poolSize", default=10)
    parser.add_option("-o", "--outDir", type="string", dest="outDir",
                      help="Specify the directory to save the output", metavar="outDir", default="/data/run/output/")
    parser.add_option("-i", "--inDir", type="string", dest="inDir",
                      help="Specify the input directory with the files", metavar="inDir", default="")
    parser.add_option("-z", "--isData", type="int", dest="isData",
                      help="Specify if the input file is data or MC", metavar="isData", default=0)
    parser.add_option("-e", "--extraFlags", type="string", dest="extraFlags",
                      help="Specify extra flags to be added to the hpstr command", metavar="extraFlags", default="")

    (options, args) = parser.parse_args()

    cfgList = []
    fnList = range(1, 10001)

    listfiles = glob.glob(options.inDir+"/*"+options.fileExt)
    if (len(listfiles) == 0):
        print("Try */*"+options.fileExt)
        listfiles = glob.glob(options.inDir+"/*/*"+options.fileExt)

    print(options.inDir)

    fnList = range(1, len(listfiles)+1)

    #create folder if doesn't exists
    if not os.path.exists(options.outDir):
        os.makedirs(options.outDir)
        print("Created outdir", options.outDir, " and logsDir ", options.outDir+"/logs")
        os.makedirs(options.outDir+"/logs")

    if options.debug:
        print("Testing %i jobs in parallel mode (using Pool(%i))" % (len(fnList), options.poolSize))
        print(list(
                         zip([options.tool for x in range(len(fnList))],
                             [listfiles[x] for x in range(len(fnList))],
                             [options.outDir for x in range(len(fnList))],
                             [options.isData for x in range(len(fnList))],
                             [options.configFile for x in range(len(fnList))],
                             fnList,
                             fnList
                             )
        ))
    else:
        print("Running %i jobs in parallel mode (using Pool(%i))" % (len(fnList), options.poolSize))
        freeze_support()
        # from: https://stackoverflow.com/questions/11312525/catch-ctrlc-sigint-and-exit-multiprocesses-gracefully-in-python
        original_sigint_handler = signal.signal(signal.SIGINT, signal.SIG_IGN)
        pool = Pool(options.poolSize)
        signal.signal(signal.SIGINT, original_sigint_handler)
        try:
            res = pool.map_async(launchTests,
                                 zip([options for x in range(len(fnList))],
                                     [listfiles[x] for x in range(len(fnList))],
                                     fnList,
                                     fnList
                                     )
                                 )
            # timeout must be properly set, otherwise tasks will crash
            print(res.get(999999999))
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
