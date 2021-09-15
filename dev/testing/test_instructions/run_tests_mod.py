#!/usr/bin/env python3

import subprocess
import time
import random
import os
import argparse
from pathlib import Path
import sys
import re
import traceback
import pickle

import time

# Globals that are used as args for the benchmark suite

# which test will be run by the benchmark. 0 for lftt, 4 for romulus/pmdk/mnemo
setType = "0"
# Range of keys that will be used for the data structures
keyRange = "100000"
# list of transaction sizes
transactions = ["1"]
# list of thread sizes
threads = ["1", "2", "4", "8", "16", "48", "96"]
# directory of benchmakrk binary
default_dir = "../../compile/src/trans"
# ratio of insertions, deletions, and updates, remainder is Get/find
dists = [["70", "2", "18"], ["8", "2", "2"]]
# flag for which cache mechanism was used
flag = ""
# string for filename building that contains name of approach being run
work = ""
# number of operations
testSize = "100000"
# number of times a test is repeated
run_size = 1


class PaperRun:
    def __init__(self, runNumber):
        self.runNumber = runNumber


class PaperRunDS:
    def __init__(self, ds, paperRun):
        self.ds = ds
        self.paperRun = paperRun


class PaperRunDSApproach:
    def __init__(self, transactionalApproach, paperRunDS):
        self.transactionalApproach = transactionalApproach
        self.paperRunDS = paperRunDS


class PaperRunDSApproachWorkload:
    def __init__(self, paperRunDSApproach, dsWorkload):
        self.paperRunDSApproach = paperRunDSApproach
        self.dsWorkload = dsWorkload


class PaperRunDSApproachWorkloadTxSize:
    def __init__(self, txSize, paperRunDSApproachWorkload):
        self.txSize = txSize
        self.paperRunDSApproachWorkload = paperRunDSApproachWorkload


class PaperRunDSApproachWorkloadTxSizeThread:
    def __init__(self, paperRunDSApproachWorkloadTxSize, threadNum):
        self.paperRunDSApproachWorkloadTxSize = paperRunDSApproachWorkloadTxSize
        self.threadNum = threadNum
        workload = paperRunDSApproachWorkloadTxSize.paperRunDSApproachWorkload
        rc = workload.paperRunDSApproach.transactionalApproach.runCommand
        self.runCommandstr = rc.execDir + rc.execCommand + ' ' + str(workload.dsWorkload.ds.dsid) + ' ' + str(
            threadNum) + \
                             ' ' + str(workload.dsWorkload.ds.testSize) + ' ' + str(
            paperRunDSApproachWorkloadTxSize.txSize) \
                             + ' ' + str(workload.dsWorkload.ds.keyRange) + ' ' + str(
            workload.dsWorkload.commandLineOperationsRatio)
        self.runCommand = [rc.execDir + rc.execCommand, str(workload.dsWorkload.ds.dsid), str(threadNum),
                           str(workload.dsWorkload.ds.testSize), str(paperRunDSApproachWorkloadTxSize.txSize),
                           str(workload.dsWorkload.ds.keyRange)]
        for opratio in workload.dsWorkload.commandLineOperationsRatio.split(' '):
            self.runCommand.append(opratio)
        self.isCompleted = False

    def execute(self):
        print("Running:\n " + str(self.runCommandstr))
        # call cur_test, pipe output to temp file
        # append temp file data to a raw data file
        # parse file for wall time, num commits, num aborts, and num spurious abors
        with open("temp_output.txt", "w+") as f:
            # save the exit code just in case
            self.exit_code = subprocess.run(self.runCommand, stdout=f)

            # go back to the top of the file and rewrite the data there
            f.seek(0)

            # read the lines into a list
            self.outputLines = f.readlines()
        os.remove("temp_output.txt")
        self.isCompleted = True

    def printOutput(self):
        if(self.isCompleted):
            for l in self.outputLines:
                print(l)
        print("exit_code: " + str(self.exit_code))


class DSWorkLoad:
    def __init__(self, ds, name, commandLineOperationsRatio):
        self.ds = ds
        self.name = name
        self.commandLineOperationsRatio = commandLineOperationsRatio


class MakeCommand:
    def __init__(self, execDir, flags, logFileName="makelog.txt"):
        self.execDir = execDir
        self.flags = flags
        self.logFileName = logFileName
        self.makeCommandFlags = "CPPFLAGS=" + flags + ""

    def execute(self, clean=False):
        print("execute make")
        log = open(self.logFileName, "w+")
        if clean:
            subprocess.Popen(
                ["make", "clean"], stdout=log, stderr=log, cwd=self.execDir
            ).wait()
        subprocess.Popen(
            ["make", self.makeCommandFlags], stdout=log, stderr=log, cwd=self.execDir
        ).wait()
        log.close()


class RunCommand:
    def __init__(self, execDir, execCommand):
        self.execDir = execDir
        self.execCommand = execCommand


class TransactionalApproach:
    def __init__(self, name, makeCommand, runCommand):
        self.name = name
        self.makeCommand = makeCommand
        self.runCommand = runCommand


class DataStructure:
    def __init__(self, name, keyRange, testSize, dsid):
        self.name = name
        # self.operations = operations
        self.keyRange = keyRange
        self.testSize = testSize
        self.dsid = dsid


class MicroBenchmark:
    def __init__(self, numberOfRuns):
        self.numberOfRuns = numberOfRuns
        self.ALL_DSS = {
            'SET': DataStructure('SET', 10000, 100000, 0),
            'SKIPList': DataStructure('SKIPList', 1000000, 1000000, 1),
            'MDList': DataStructure('MDList', 1000000, 1000000, 2),
            'MAP': DataStructure('MAP', 1000000, 1000000, 3)
        }
        self.buildRootDir = '/home/sdp/ramin/src/pmem/petra/compile/'
        self.execRootDir = '/home/sdp/ramin/src/pmem/petra/compile/src/'
        self.sharedFlags = "-DENABLE_READ_ONLY_OPT -DUSE_PMEM_ALLOCATOR -DPWB_IS_CLWB"
        self.allButPMDKFlags = "-DROMULUS_LR_PTM"
        self.pmdkonlyFlags = "-DPMDK_PTM"
        self.ALL_APPROACHES = {
            'PETRA': TransactionalApproach('PETRA', MakeCommand(self.buildRootDir,
                                                                self.sharedFlags + " " + self.allButPMDKFlags),
                                           RunCommand(self.execRootDir, 'trans')),
            'OneFile': TransactionalApproach('OneFile', MakeCommand(self.buildRootDir,
                                                                    self.sharedFlags + " " + self.allButPMDKFlags),
                                             RunCommand(self.execRootDir, 'transOneFile')),
            'RomulsLR': TransactionalApproach('RomulsLR', MakeCommand(self.buildRootDir,
                                                                      self.sharedFlags + " " + self.allButPMDKFlags),
                                              RunCommand(self.execRootDir, 'transRomulus')),
            'PMDK': TransactionalApproach('PMDK', MakeCommand(self.buildRootDir,
                                                              self.sharedFlags + " " + self.pmdkonlyFlags),
                                          RunCommand(self.execRootDir, 'transRomulus'))

        }

        self.ALL_DSWORKLOADS = {
            'SET': [{'SET-write-dominated': DSWorkLoad(self.ALL_DSS['SET'], 'SET-write-dominated', '40 40')},
                    {'SET-read-dominated': DSWorkLoad(self.ALL_DSS['SET'], 'SET-read-dominated', '10 10')}],
            'SKIPList': [
                {'SKIPList-write-dominated': DSWorkLoad(self.ALL_DSS['SKIPList'], 'SKIPList-write-dominated', '40 40')},
                {'SKIPList-read-dominated': DSWorkLoad(self.ALL_DSS['SKIPList'], 'SKIPList-read-dominated', '10 10')}],
            'MDList': [
                {'MDList-write-dominated': DSWorkLoad(self.ALL_DSS['MDList'], 'MDList-write-dominated', '40 40')},
                {'MDList-read-dominated': DSWorkLoad(self.ALL_DSS['MDList'], 'MDList-read-dominated', '10 10')}],
            'MAP': [{'MAP-write-dominated': DSWorkLoad(self.ALL_DSS['MAP'], 'MAP-write-dominated', '40 30 10')},
                    {'MAP-read-dominated': DSWorkLoad(self.ALL_DSS['MAP'], 'MAP-read-dominated', '10 10 5')}],
        }

        self.ALL_TX_SIZE = [1, 2, 4, 8, 16]

        self.ALL_THREADS = [1, 2, 4, 8, 16, 48, 96]

        self.fileName = 'microBenchmarkAllRuns'
        self.totalNumberOfTests = 0

    def initFlushCompareTests(self):
        self.ALL_DSS = {
            'SET': DataStructure('SET', 10000, 100000, 0)
        }
        self.buildRootDir = '/home/sdp/ramin/src/pmem/petra/compile/'
        self.execRootDir = '/home/sdp/ramin/src/pmem/petra/compile/src/'
        self.sharedFlags = "-DENABLE_READ_ONLY_OPT -DUSE_PMEM_ALLOCATOR -DROMULUS_LR_PTM"
        self.CLWBFLAGS = "-DPWB_IS_CLWB"
        self.CLFLUSHFLAGS = "-DPWB_IS_CLFLUSH"
        self.CLFLUSHOPTFLAGS = "-DPWB_IS_CLFLUSHOPT"

        self.ALL_APPROACHES = {
            'PETRA-CLWB': TransactionalApproach('PETRA-CLWB', MakeCommand(self.buildRootDir,
                                                                self.sharedFlags + " " + self.CLWBFLAGS),
                                           RunCommand(self.execRootDir, 'trans')),
            'PETRA-CLFLUSH': TransactionalApproach('PETRA-CLFLUSH', MakeCommand(self.buildRootDir,
                                                                self.sharedFlags + " " + self.CLFLUSHFLAGS),
                                           RunCommand(self.execRootDir, 'trans')),
            'PETRA-CLFLUSHOPT': TransactionalApproach('PETRA-CLFLUSHOPT', MakeCommand(self.buildRootDir,
                                                                self.sharedFlags + " " + self.CLFLUSHOPTFLAGS),
                                           RunCommand(self.execRootDir, 'trans')),

        }

        self.ALL_DSWORKLOADS = {
            'SET': [{'SET-write-dominated': DSWorkLoad(self.ALL_DSS['SET'], 'SET-write-dominated', '40 40')}]
        }


        self.ALL_TX_SIZE = [1]

        self.ALL_THREADS = [1, 2, 4, 8, 16, 48, 96]

        self.fileName = 'microBenchmark-FLUSH-comp'
        self.totalNumberOfTests = 0    

    def initLFTTCompareTests(self):
        self.ALL_DSS = {
            'SET': DataStructure('SET', 10000, 100000, 0)
        }
        self.buildRootDir = '/home/sdp/ramin/src/pmem/petra/compile/'
        self.execRootDir = '/home/sdp/ramin/src/pmem/petra/compile/src/'
        self.buildRootDirLFTT = '/home/sdp/ramin/src/original-lftt/compile/'
        self.execRootDirLFTT = '/home/sdp/ramin/src/original-lftt/compile/src/'        
        self.sharedFlags = "-DENABLE_READ_ONLY_OPT -DUSE_PMEM_ALLOCATOR -DROMULUS_LR_PTM"
        self.CLWBFLAGS = "-DPWB_IS_CLWB"
        self.CLFLUSHFLAGS = "-DPWB_IS_CLFLUSH"
        self.CLFLUSHOPTFLAGS = "-DPWB_IS_CLFLUSHOPT"

        self.ALL_APPROACHES = {
            'LFTT': TransactionalApproach('LFTT', MakeCommand(self.buildRootDirLFTT,
                                                                self.sharedFlags),
                                           RunCommand(self.execRootDirLFTT, 'trans')),            
            'PETRA-CLWB': TransactionalApproach('PETRA-CLWB', MakeCommand(self.buildRootDir,
                                                                self.sharedFlags + " " + self.CLWBFLAGS),
                                           RunCommand(self.execRootDir, 'trans')),
            'PETRA-CLFLUSHOPT': TransactionalApproach('PETRA-CLFLUSHOPT', MakeCommand(self.buildRootDir,
                                                                self.sharedFlags + " " + self.CLFLUSHOPTFLAGS),
                                           RunCommand(self.execRootDir, 'trans')),                                           
            'PETRA-CLFLUSH': TransactionalApproach('PETRA-CLFLUSH', MakeCommand(self.buildRootDir,
                                                                self.sharedFlags + " " + self.CLFLUSHFLAGS),
                                           RunCommand(self.execRootDir, 'trans'))
        }

        self.ALL_DSWORKLOADS = {
            'SET': [{'SET-write-dominated': DSWorkLoad(self.ALL_DSS['SET'], 'SET-write-dominated', '40 40')}]
        }


        self.ALL_TX_SIZE = [1]

        self.ALL_THREADS = [1, 2, 4, 8, 16, 48, 96]

        self.fileName = 'microBenchmark-LFTT-FLUSH-comp'
        self.totalNumberOfTests = 0   
        self.numberOfRuns = 10                            

    def runTests(self):
        self.totalNumberOfTests = 11200
        currentTest = 0
        startTime = time.time()
        # print(self.ALL_APPROACHES['PMDK'].makeCommand.flags)
        # toBeIgnored = [self.all_runs[1]['MDList']['PETRA']['MDList-write-dominated'][4][48],
        # self.all_runs[1]['MDList']['PETRA']['MDList-write-dominated'][4][96],
        # self.all_runs[1]['MDList']['PETRA']['MDList-write-dominated'][8][48],
        # self.all_runs[1]['MDList']['PETRA']['MDList-write-dominated'][8][96],
        # self.all_runs[1]['MDList']['PETRA']['MDList-write-dominated'][16][16],
        # self.all_runs[1]['MDList']['PETRA']['MDList-write-dominated'][16][48],
        # self.all_runs[1]['MDList']['PETRA']['MDList-write-dominated'][16][96],
        # self.all_runs[1]['MDList']['PETRA']['MDList-read-dominated'][4][48],
        # self.all_runs[1]['MDList']['PETRA']['MDList-read-dominated'][4][96],
        # self.all_runs[1]['MDList']['PETRA']['MDList-read-dominated'][8][48],
        # self.all_runs[1]['MDList']['PETRA']['MDList-read-dominated'][8][96],
        # self.all_runs[1]['MDList']['PETRA']['MDList-read-dominated'][16][16],
        # self.all_runs[1]['MDList']['PETRA']['MDList-read-dominated'][16][48],
        # self.all_runs[1]['MDList']['PETRA']['MDList-read-dominated'][16][96]]
        # toBeIgnoredDS = [self.all_runs[1]['SET'],self.all_runs[1]['SKIPList']]
        # toBeIgnoedDSApproach = [self.all_runs[1]['MDList']['PETRA']]
        # self.all_runs[1]['MDList']['PETRA']['MDList-write-dominated'][4][96].isCompleted = False
        for run in range(8, self.numberOfRuns + 1): #self.all_runs.keys():
            print('<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< RUN: ' + str(run) + ' >>>>>>>>>>>>>>>>>>>>>>>>>>>>')
            currentRun = self.all_runs[run]
            for dsname in currentRun.keys():
                # if run == 5 and dsname == 'SET':
                #     continue

                print(dsname)
                print('########################################################################')
                currentDS = currentRun[dsname]
                # if currentDS in toBeIgnoredDS:
                #     print("ignoring: " + dsname)
                #     continue
                for approachname in currentDS.keys():
                    if run == 5 and dsname == 'SKIPList' and approachname == 'PETRA':
                        continue
                    # if dsname == 'SKIPList':
                    #     continue
                    print(approachname)
                    print('@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@')                    
                    currentApproach = currentDS[approachname]
                    # if currentApproach not in toBeIgnoedDSApproach:
                    #     print("ignoring: " + dsname + " " + approachname)
                    #     continue
                    self.ALL_APPROACHES[approachname].makeCommand.execute(True)
                    for workloadname in currentApproach.keys():
                        currentWorkload = currentApproach[workloadname]
                        for txsize in currentWorkload.keys():
                            currentTxsize = currentWorkload[txsize]
                            for thread in currentTxsize.keys():
                                currentThread = currentTxsize[thread]
                                if currentThread.isCompleted:
                                    currentTest = currentTest + 1
                                    continue

                                # if currentThread not in toBeIgnored:
                                #     print("ignoring: " + currentThread.runCommandstr)
                                #     continue
                                timeSinceStart = round(time.time() - startTime)
                                print(str(timeSinceStart) + " seconds from the start: Test " + str(currentTest) + "/" + str(self.totalNumberOfTests))
                                currentThread.execute()
                                currentThread.printOutput()
                                cleanup()
                                with open(self.fileName, 'wb') as handle:
                                    pickle.dump(self.all_runs, handle, protocol=pickle.HIGHEST_PROTOCOL)
                                currentTest = currentTest + 1

    def prepareTests(self):
        self.all_runs = {}
        for run in range(1, self.numberOfRuns + 1):
            pr = PaperRun(run)

            ds_all_runs = {}
            self.all_runs[run] = ds_all_runs
            for dsname in self.ALL_DSS.keys():
                ds = self.ALL_DSS[dsname]
                prds = PaperRunDS(ds, pr)

                ds_approach_all_runs = {}
                ds_all_runs[dsname] = ds_approach_all_runs

                for approachname in self.ALL_APPROACHES.keys():
                    approach = self.ALL_APPROACHES[approachname]
                    prdsa = PaperRunDSApproach(approach, prds)

                    ds_approach_workload_all_runs = {}
                    ds_approach_all_runs[approachname] = ds_approach_workload_all_runs
                    dsworkloadsList = self.ALL_DSWORKLOADS[dsname]
                    for dsworkloads in dsworkloadsList:
                        for dsworkloadname in dsworkloads.keys():
                            dsworkload = dsworkloads[dsworkloadname]
                            prdsaw = PaperRunDSApproachWorkload(prdsa, dsworkload)

                            ds_approach_workload_tx_size_all_runs = {}
                            ds_approach_workload_all_runs[dsworkloadname] = ds_approach_workload_tx_size_all_runs

                            for txsize in self.ALL_TX_SIZE:
                                prdsawt = PaperRunDSApproachWorkloadTxSize(txsize, prdsaw)

                                ds_approach_workload_tx_size_thread_all_runs = {}
                                ds_approach_workload_tx_size_all_runs[
                                    txsize] = ds_approach_workload_tx_size_thread_all_runs

                                for thread in self.ALL_THREADS:
                                    prdsawtt = PaperRunDSApproachWorkloadTxSizeThread(prdsawt, thread)

                                    ds_approach_workload_tx_size_thread_all_runs[thread] = prdsawtt
                                    self.totalNumberOfTests = self.totalNumberOfTests + 1

    def test_all_runs(self):
        for run in self.all_runs.keys():
            currentRun = self.all_runs[run]
            for dsname in currentRun.keys():
                print(dsname)
                print('########################################################################')
                currentDS = currentRun[dsname]
                for approachname in currentDS.keys():
                    currentApproach = currentDS[approachname]
                    for workloadname in currentApproach.keys():
                        currentWorkload = currentApproach[workloadname]
                        for txsize in currentWorkload.keys():
                            currentTxsize = currentWorkload[txsize]
                            for thread in currentTxsize.keys():
                                currentThread = currentTxsize[thread]
                                print(currentThread.runCommandstr)
                                currentThread.printOutput()

    def loadAllRuns(self):
        file = open(self.fileName, 'rb')
        self.all_runs = pickle.load(file)
        file.close()
    
    def run_flush_exp(self):
        self.initFlushCompareTests()
        self.prepareTests()
        self.runTests()

    def run_lftt_exp(self):
        self.initLFTTCompareTests()
        self.prepareTests()
        self.runTests()        

# class that holds information for running tests
class TestInfo:

    # initializes test object with some default values
    def __init__(self):
        self.setType = "0"
        self.keyrange = "100000"
        self.transactions = ["1"]
        self.threads = ["1", "2", "4", "8", "16", "48", "96"]
        self.default_dir = "../../compile/src/trans"
        self.dists = [["70", "2", "18"], ["8", "2", "2"]]
        self.flag = ""
        self.work = ""
        self.testSize = "100000"
        self.run_size = 1


# Build all of the commands to run and then execute them in random order, run_size amount of times
def run_test(test_info):
    # keeps track of how many times each command was run
    num_runs = {}
    # Dictionary mapping command tuples to a list of results
    results = {}

    # will store all of the commands we plan on running
    tests = []
    # keeps track of commands that have not run run_size number of times
    not_complete = []

    # build the commands using the ratio, transactions size(s) and number of threads
    # these commands will be called as a subprocess and the output will be processed
    for dist in test_info.dists:
        for tranSize in test_info.transactions:
            for numThread in threads:
                insertion = dist[0]
                deletion = dist[1]
                # update = dist[2]

                # this tuple representing the command is used as a key for the num_runs and results dictionaries
                command = (
                    test_info.default_dir,
                    test_info.setType,
                    test_info.numThread,
                    test_info.testSize,
                    test_info.tranSize,
                    test_info.keyRange,
                    insertion,
                    deletion,
                    # update,
                )
                # set the current command key to have 0 num_runs, and an empty list in results
                num_runs[command] = 0
                results[command] = []
                tests.append(command)
                not_complete.append(command)

    # keep running commands as long as there are commands left to run
    while len(not_complete) != 0:
        # pick a random test to run
        cur_test = random.choice(not_complete)
        print("Running " + str(cur_test))
        filename = f"raw_{test_info.work}_{test_info.flag}_{test_info.keyRange}.txt"

        # call cur_test, pipe output to temp file
        # append temp file data to a raw data file
        # parse file for wall time, num commits, num aborts, and num spurious abors
        with open("temp_output.txt", "w+") as f:

            # save the exit code just in case
            exit_code = subprocess.run(cur_test, stdout=f)

            # go back to the top of the file and rewrite the data there
            f.seek(0)

            # read the lines into a list
            file_lines = f.readlines()
            with open(filename, "a+") as raw:

                # append the lines to the raw data file
                raw.writelines(file_lines)
                raw.write("\n")

            try:
                cpu_index = 0
                commit_index = 0

                # look through the lines in the list and figure out which lines contain the WALL time and commit numbers
                for i, cur_line in enumerate(file_lines):
                    if "CPU Time" in cur_line:
                        cpu_index = i
                    if "Total commit" in cur_line:
                        commit_index = i
                # grab the line of output with WALL time and strip out the seconds identifier
                cur_time = float(
                    file_lines[cpu_index].split()[5].strip("s").strip("\n")
                )

                cur_commits = int(file_lines[commit_index].split()[2].strip(","))

                cur_aborts = int(file_lines[commit_index].split()[5].split("/")[0])

                cur_spurious = int(
                    file_lines[commit_index].split()[5].split("/")[1].strip("\n")
                )
            except:
                # handle errors here, print trace back and clean up files, then exit
                print("Error!")
                print(file_lines)
                exc_info = sys.exc_info()
                traceback.print_exception(*exc_info)
                del exc_info
                cleanup()
                if file_lines == []:
                    continue
                exit(0)  # TODO Back to continue later
            # Calculate operations per second
            ops_per_sec = (cur_commits * int(cur_test[4])) / cur_time
            # append results of this test to the list that is the value for the test key
            results[cur_test].append(
                [cur_time, cur_commits, cur_aborts, cur_spurious, ops_per_sec]
            )

            # increment number of runs for this test
            num_runs[cur_test] += 1

            # eliminate this test from the pool if it has run run_size times
            if num_runs[cur_test] == test_info.run_size:
                not_complete.remove(cur_test)
            time.sleep(10)

        # remove persistent memory or mem mapped files for next test
        cleanup()
    print("All Tests completed! Calculating averages now")
    calculate_results(tests, results)
    if not args.debug:
        print("Done calculating averages!\n Cleaning up...")
        os.remove("temp_output.txt")


# TODO Pass in test info object
def calculate_results(tests, results):
    # need to sort the lists if calculating results from raw data, not from the tests running
    if args.raw:
        tests = sorted(list(tests), key=lambda x: int(x[2]))

    for test in tests:
        calculate_average(test, results)
        calculate_median(test, results)


# using the results from the test, calculate the average of each data point for the 10 runs

# TODO Pass in test info object
def calculate_average(test, results):
    avg_time = 0
    avg_commits = 0
    avg_aborts = 0
    avg_spurious = 0
    avg_ops = 0

    # for each list of results in the map, sum up the values
    for result in results[test]:
        avg_time += result[0]
        avg_commits += result[1]
        avg_aborts += result[2]
        avg_spurious += result[3]
        avg_ops += result[4]

    # divide the sum by number of runs to get the average
    avg_time /= run_size
    avg_commits /= run_size
    avg_aborts /= run_size
    avg_spurious /= run_size
    avg_ops /= run_size

    # construct output file name
    file_name = f"{test[6]}_{work}_{flag}_{keyRange}_average.txt"

    # write to the output file
    with open(file_name, "a+") as file:
        test_string = (
                "Transaction Size "
                + str(test[4])
                + " "
                + str(test[6])
                + " insertions "
                + str(test[7])
                + " deletions\n"
                + str(test[2])
                + " threads "
                + str(avg_time)
                + " time "
                + str(avg_commits)
                + " commits "
                + str(avg_aborts)
                + " aborts "
                + str(avg_spurious)
                + " spurious "
                + str(avg_ops)
                + " ops per sec\n\n\n"
        )
        file.write(test_string)


# TODO Pass in test info object
def calculate_median(test, results):
    global run_size

    # results[test] returns a list of tuples with the test data
    # sort that list by the 5th element in the tuple, which is the ops/sec
    sorted_results = sorted(results[test], key=lambda x: x[4])

    # divide number of runs by 2 to get the median
    median = sorted_results[(run_size - 1) // 2]

    # extract values from the tuple
    med_time = median[0]
    med_commits = median[1]
    med_aborts = median[2]
    med_spurious = median[3]
    med_ops = median[4]

    file_name = f"{test[6]}_{work}_{flag}_{keyRange}_median.txt"

    with open(file_name, "a+") as file:
        test_string = (
                "Transaction Size "
                + str(test[4])
                + " "
                + str(test[6])
                + " insertions "
                + str(test[7])
                + " deletions\n"
                + str(test[2])
                + " threads "
                + str(med_time)
                + " time "
                + str(med_commits)
                + " commits "
                + str(med_aborts)
                + " aborts "
                + str(med_spurious)
                + " spurious "
                + str(med_ops)
                + " ops per sec\n\n\n"
        )
        file.write(test_string)


# Cleans up persistement memory and memory mapped files
def cleanup():
    for rom_file in Path("/dev/shm").glob("*"):  # remove romulus memory mapped files
        if "psegments" in str(rom_file):
            continue
        rom_file.unlink()

    for lfft_file in Path("/mnt/pmem0/").glob("*"):
        lfft_file.unlink()

    # for pmdk_file in Path("/dev/shm/").glob("*"):
    # pmdk_file.unlink()

    for mnem_file in Path("/dev/shm/psegments").glob("*"):
        mnem_file.unlink()


# Run a make command to build the current work and cache mechanism that is desired
def make(flags, clean=False):
    working_dir = input("Enter build directory")
    # If we are not running a make clean
    if not clean:
        cppflags = "CPPFLAGS=" + flags + ""
        if args.debug:
            print(cppflags)
            subprocess.Popen(["make", cppflags], cwd=working_dir).wait()
        else:
            log = open("makelog.txt", "w+")
            subprocess.Popen(
                ["make", cppflags], stdout=log, stderr=log, cwd=working_dir
            ).wait()
            log.close()
    else:
        if args.debug:
            subprocess.Popen(["make", "clean"], cwd=working_dir).wait()
        else:
            log = open("makelog.txt", "w+")
            subprocess.Popen(
                ["make", "clean"], stdout=log, stderr=log, cwd=working_dir
            ).wait()
            log.close()


# convert raw data files to plot gen format
# in case a test failed but the raw data was saved

# TODO Pass in test info object
def raw_to_ratio(files):
    print("Converting raw data to plot format...")
    global work
    global flag
    global keyRange

    # given the raw data file name, figure out the cache used and the approach used
    for cur_file in files:
        if "CLFLUSHOPT" in cur_file:
            flag = "CLFLUSHOPT"
        elif "CLFLUSH" in cur_file:
            flag = "CLFLUSH"
        elif "CLWB" in cur_file:
            flag = "CLWB"
        else:
            print("File does not have flag name in it!")
            exit()

        if "petraall" in cur_file:
            work = "petraall"
        elif "lftt" in cur_file:
            work = "lftt"
        elif "pmdk" in cur_file:
            work = "pmdk"
        elif "mnemosyne" in cur_file:
            work = "mnemosyne"
        elif "romulus" in cur_file:
            work = "romulus"
        else:
            print("File does not have name of work in it!")
            exit()

        if "1000000" in cur_file:
            keyRange = "1000000"
        elif "1000" in cur_file:
            keyRange = "1000"
        else:
            print("File does not have keyRange in it!")
            exit()

        # go through the files of raw data, and parse it for information we need
        with open(cur_file, "r") as f:
            lines = f.readlines()

            # dictionary that will map test descriptions to list of results for that test
            results = {}

            # store calculations for the 70 test (need to change if using different ratios)
            test70 = set()

            # store calculations for 8 test
            test8 = set()
            if work != "mnemosyne":
                # loop through the raw data in chunks of 4 based off of raw format
                for i in range(0, len(lines), 4):
                    # read the line into a list seperated by whitespace, get thread count, num insertions, and num deletions from that list
                    split_line = lines[i].split()
                    threads = split_line[4]
                    insert = split_line[13].strip("%")
                    delete = split_line[15].strip("%")

                    # make a tuple similar to the one made when the tests are running
                    cur = ("", "", threads, "", "1", "", insert, delete)

                    # save to the appropriate set
                    if "70" in lines[i]:
                        test70.add(cur)
                    else:
                        test8.add(cur)

                    # parse data from this chunk
                    try:
                        cur_time = float(lines[i + 1].split()[5].strip("s"))

                        cur_commits = int(lines[i + 2].split()[2].strip(","))

                        cur_aborts = int(lines[i + 2].split()[5].split("/")[0])

                        cur_spurious = int(lines[i + 2].split()[5].split("/")[1])
                    except IndexError:
                        print(lines[i])
                        exit()
                    ops_per_sec = (cur_commits * int(lines[i].split()[8])) / cur_time
                    result = [
                        cur_time,
                        cur_commits,
                        cur_aborts,
                        cur_spurious,
                        ops_per_sec,
                    ]
                    # add this list to a list of results, and map it to cur
                    if cur in results:
                        results[cur].append(result)
                    else:
                        results[cur] = [result]
            # I really don't like doing this but it is quicker than refactoring so I will have to do that later
            # this does the same as the previous loop but adjusted for mnemosyne, since the output is slightly different
            else:
                for i in range(0, len(lines), 9):
                    split_line = lines[i + 4].split()
                    threads = split_line[4]
                    insert = split_line[13].strip("%")
                    delete = split_line[15].strip("%")
                    cur = ("", "", threads, "", "1", "", insert, delete)
                    if "70" in lines[i + 4]:
                        test70.add(cur)
                    else:
                        test8.add(cur)
                    try:
                        cur_time = float(lines[i + 6].split()[5].strip("s"))

                        cur_commits = int(lines[i + 7].split()[2].strip(","))

                        cur_aborts = int(lines[i + 7].split()[5].split("/")[0])

                        cur_spurious = int(lines[i + 7].split()[5].split("/")[1])
                    except IndexError:
                        print(lines[i + 4])
                        exit()
                    ops_per_sec = (
                                          cur_commits * int(lines[i + 4].split()[8])
                                  ) / cur_time
                    result = [
                        cur_time,
                        cur_commits,
                        cur_aborts,
                        cur_spurious,
                        ops_per_sec,
                    ]

                    if cur in results:
                        results[cur].append(result)
                    else:
                        results[cur] = [result]
        print(f"Finished converting {cur_file}!")
        calculate_results(test70, results)
        calculate_results(test8, results)


# run lftt tests, give it the proper make flags, and set the global parameters for the test
def lftt():
    print("Testing LFTT CLFLUSH with TX Size 1\n")
    '''global work
    global default_dir
    global transactions
    global setType
    global keyRange
    global flag
    '''
    test_info = TestInfo()
    test_info.work = "lftt"
    test_info.default_dir = "../../compile/src/trans"
    test_info.transactions = ["1"]
    test_info.setType = "3"

    # make("-DUSE_PMEM_ALLOCATOR -DPWB_IS_CLFLUSH -DROMULUS_LOG_PTM")
    # flag = "CLFLUSH"
    # keyRange="1000"
    # run_test()

    # keyRange="10000"
    # run_test()

    # make("", clean=True)

    print("Testing LFTT CLFLUSHOPT with TX Size 1\n")
    make("-DUSE_PMEM_ALLOCATOR -DPWB_IS_CLFLUSHOPT -DROMULUS_LOG_PTM")
    test_info.flag = "CLFLUSHOPT"
    # keyRange = "1000"
    # run_test()

    # keyRange="10000"
    run_test(test_info)

    make("", clean=True)

    """print("Testing LFTT CLWB with TX Size 1\n")
    #make("-DUSE_PMEM_ALLOCATOR -DPWB_IS_CLWB -DROMULUS_LOG_PTM")
    #flag = "CLWB"
    #keyRange = "1000"
    #run_test()

    keyRange="10000"
    run_test()

    make("", clean=True)
"""


def dramlftt():
    print("Testing DRAMLFTT TX Size 1\n")
    global work
    global default_dir
    global transactions
    global setType
    global keyRange
    global flag
    global dists

    work = "dramlftt"
    default_dir = "../../compile/src/trans"
    transactions = ["1"]
    setType = "0"
    dists = [["50", "50"]]
    make("-DUSE_DRAM_ALLOCATOR -DPWB_IS_CLFLUSHOPT -DROMULUS_LOG_PTM")
    flag = "DRAM"
    # keyRange="1000"
    # run_test()

    keyRange = "10000"
    run_test()

    make("", clean=True)


# run petra tests with all transaction sizes
def petraall():
    global work
    global default_dir
    global transactions
    global setType
    global keyRange
    global flag
    global dists

    work = "petraall"
    default_dir = input("Path to compiled PETRA binary")  # "../../compile/src/trans"
    transactions = input(
        "Enter space separated list of transaction sizes"
    ).split()  # ["1", "2", "4", "8", "16"]
    setType = "0"
    keyRange = input("Enter key range for test")
    dists = [["50", "50"]]

    # make("-DUSE_PMEM_ALLOCATOR -DPWB_IS_CLFLUSH -DROMULUS_LOG_PTM")
    # flag = "CLFLUSH"
    # keyRange = "1000"
    # run_test()

    # keyRange = "10000"
    # run_test()

    # make("", clean=True)

    make("-DUSE_PMEM_ALLOCATOR -DPWB_IS_CLFLUSHOPT -DROMULUS_LOG_PTM")
    flag = "CLFLUSHOPT"
    # keyRange = "1000"
    run_test()

    # keyRange = "10000"
    # run_test()

    make("", clean=True)

    # make("-DUSE_PMEM_ALLOCATOR -DPWB_IS_CLWB -DROMULUS_LOG_PTM")
    # flag = "CLWB"
    # keyRange = "1000"
    # run_test()

    # keyRange = "10000"
    # run_test()

    # make("", clean=True)


# set up romulus test with global parameters and the proper make command flags
def romulus():
    global work
    global default_dir
    global transactions
    global setType
    global keyRange
    global flag

    work = "romulus"
    default_dir = "../../compile/src/transRomulus"
    transactions = ["1"]
    setType = "5"

    # make("-DUSE_PMEM_ALLOCATOR -DPWB_IS_CLFLUSH -DROMULUS_LOG_PTM")
    # flag = "CLFLUSH"
    # keyRange="1000"
    # run_test()

    # keyRange="10000"
    # run_test()

    # make("", clean=True)

    make("-DUSE_PMEM_ALLOCATOR -DPWB_IS_CLFLUSHOPT -DROMULUS_LOG_PTM")
    flag = "CLFLUSHOPT"
    # keyRange = "1000"
    run_test()

    # keyRange="10000"
    # run_test()

    make("", clean=True)

    # make("-DUSE_PMEM_ALLOCATOR -DPWB_IS_CLWB -DROMULUS_LOG_PTM")
    # flag = "CLWB"
    # keyRange = "1000"
    # run_test()

    # keyRange="10000"
    # run_test()

    # make("", clean=True)


def pmdk():
    global work
    global default_dir
    global transactions
    global setType
    global keyRange
    global flag

    work = "pmdk"
    default_dir = "../../compile/src/transRomulus"
    transactions = ["1"]
    setType = "5"

    make("-DUSE_PMEM_ALLOCATOR -DPWB_IS_CLFLUSHOPT -DPMDK_PTM")
    flag = "CLFLUSHOPT"
    # keyRange = "1000"
    run_test()

    # keyRange="10000"
    # run_test()

    make("", clean=True)


""" make("-DUSE_PMEM_ALLOCATOR -DPWB_IS_CLFLUSH -DPMDK_PTM") 
    flag = "CLFLUSH"
    keyRange="1000"
    run_test()

    keyRange="10000"
    run_test()

    make("", clean=True)




    make("-DUSE_PMEM_ALLOCATOR -DPWB_IS_CLWB -DPMDK_PTM")
    flag = "CLWB"
    keyRange = "1000"
    run_test()

    keyRange="10000"
    run_test()

    make("", clean=True)
"""


def mnemosyne():
    global work
    global default_dir
    global transactions
    global setType
    global keyRange
    global flag
    global threads
    threads = ["1", "2", "4", "8", "16", "24", "31"]
    work = "mnemosyne"
    transactions = ["1"]
    setType = "5"

    os.chdir("../../mnemosyne/mnemosyne-gcc/usermode/")
    # default_dir = "build/examples/romulus/transMnemosyne_CLFLUSH"
    # flag = "CLFLUSH"
    # keyRange = "1000"
    # run_test()
    # keyRange = "10000"
    # run_test()

    default_dir = "build/examples/romulus/transMnemosyne_CLFLUSHOPT"
    flag = "CLFLUSHOPT"
    # keyRange = "100000"
    # run_test()
    # keyRange = "10000"
    run_test()

    # default_dir = "build/examples/romulus/transMnemosyne_CLWB"
    # flag = "CLFLUSHOPT"
    # keyRange = "1000"
    # run_test()
    # key_range = "10000"


def db():
    # default_dir = input(
        # "Path to directory containing benchmarktest binary"
    # )  # ../../pmemkv-tools/pmemkv_bench"
    default_dir = "../../../../pmemkv-tools/pmemkv-tools/pmemkv_bench"
    # build_dir = input("Path to build directory")
    build_dir = "../../../../pmemkv-tools/pmemkv-tools/"
    # threads = (
    #     input("Enter a space seperated list of thread counts")
    # ).split()  # ["1", "2", "4", "8", "16", "48", "96"]
    threads =  ["1", "2", "4", "8", "16", "48", "96"]
    engines = ["dlfttmap", "cmap_int"]
    benchmarks = [
        "fillrandom,deleterandom,overwrite,fillseq,readrandom,readseq,readmissing,deleteseq",
        "readrandomwriterandom",
        "readwhilewriting"
    ]
    # db_path = input("Path to db file")
    db_path = f"/mnt/pmem0/pmemkv"
    # db_size = int(input("Enter size of DB in Gigabytes as an integer"))
    db_size = 1
    db_file = f"--db={db_path}"  # "--db=/dev/shm/pmemkv"
    # db_file = f"/mnt/pmem0/pmemkv"
    db_size_string = f"--db_size_in_gb={db_size}"
    histogram = "--histogram=1"
    num_runs = {}
    # iterations = int(input("Enter number of times to run each test"))
    iterations = 10
    for engine in engines:
        for benchmark in benchmarks:
            for thread in threads:
                command = (
                    f"PMEM_IS_PMEM_FORCE=1 sudo {default_dir} {db_file} {db_size_string} {histogram} --engine={engine} --threads={thread} --benchmarks={benchmark}",
                    engine,
                    thread,
                )
                num_runs[command] = iterations

    while len(num_runs) != 0:
        cur_test = random.choice(list(num_runs.keys()))
        os.system(cur_test[0] + f" >> raw_db_{cur_test[1]}_{cur_test[2]}.txt")
        subprocess.Popen(["make", "reset"], cwd=f"{build_dir}").wait()
        time.sleep(10)
        num_runs[cur_test] -= 1
        if num_runs[cur_test] == 0:
            del num_runs[cur_test]


def romulusall():
    global work
    global default_dir
    global transactions
    global setType
    global keyRange
    global flag
    global dists

    work = "romulusall"
    default_dir = input(
        "enter directory for romulus binary"
    )  # "../../compile/src/transRomulus"
    transactions = input(
        "enter space separated list of transaction sizes"
    ).split()  # ["2", "4", "8", "16"]
    dists = [["50", "50"]]
    setType = "4"

    keyRange = input("Etner key range for test")  # "10000"

    make("-DUSE_PMEM_ALLOCATOR -DPWB_IS_CLFLUSHOPT -DROMULUS_LOG_PTM")
    flag = "CLFLUSHOPT"

    run_test()

    make("", clean=True)


def originallftt():
    global work
    global default_dir
    global transactions
    global setType
    global keyRange
    global flag
    global dists

    work = "originallftt"
    default_dir = "../../../original-lftt/compile/src/trans"
    transactions = ["1"]
    setType = "0"
    keyRange = "10000"
    flag = "original"
    dists = [["33", "33"], ["15", "5"]]

    run_test()


def mnemosyneall():
    global work
    global default_dir
    global transactions
    global setType
    global keyRange
    global flag
    global threads
    global dists

    threads = ["1", "2", "4", "8", "16", "24", "31"]
    work = "mnemosyneall"
    transactions = ["2", "4", "8", "16"]
    setType = "4"
    dists = [["50", "50"]]

    os.chdir("../../mnemosyne/mnemosyne-gcc/usermode/")
    # default_dir = "build/examples/romulus/transMnemosyne_CLFLUSH"
    # flag = "CLFLUSH"
    # keyRange = "1000"
    # run_test()
    # keyRange = "10000"
    # run_test()

    default_dir = "build/examples/romulus/transMnemosyne_CLFLUSHOPT"
    flag = "CLFLUSHOPT"
    keyRange = "10000"
    # run_test()
    # keyRange = "10000"
    run_test()


if __name__ == "__main__":
    # db()
    m = MicroBenchmark(10)
    
    # m.run_flush_exp()
    # m.run_lftt_exp()

    # m.prepareTests()
    # m.test_all_runs()
    
    m.loadAllRuns()
    m.runTests()

    # m.loadAllRuns()
    # m.test_all_runs()

def oldMain():
    parser = argparse.ArgumentParser(
        description="Testing Script for Durable-LFTT and Related Works"
    )
    parser.add_argument(
        "--lftt", help="Run Durable-Lftt with Tx size 1", action="store_true"
    )
    parser.add_argument(
        "--romulus", help="Run Romulus with Tx size 1", action="store_true"
    )
    parser.add_argument("--pmdk", help="Run PMDK with Tx size 1", action="store_true")
    parser.add_argument(
        "--mnemosyne", help="Run Mnemosyne with Tx size 1", action="store_true"
    )
    parser.add_argument(
        "--petraall",
        help="Run Durable-Lftt with Tx sizes 1 2 4 8 16",
        action="store_true",
    )
    parser.add_argument(
        "--dramlftt", help="Run Durable-lftt on DRAM", action="store_true"
    )
    # parser.add_argument(
    #    "--all",
    #    help="Run tests on all available works with Tx size 1",
    #    action="store_true",
    # )
    parser.add_argument(
        "--debug",
        "-d",
        help="Run script in Debug mode which adds verbosity",
        action="store_true",
    )
    parser.add_argument(
        "--raw",
        help="Convert raw data in form of raw*.txt to format for plot generation (need to supply --files flag)",
        action="store_true",
    )
    parser.add_argument(
        "--files",
        help="List raw filenames to be converted to plot format",
        type=str,
        nargs="+",
    )

    parser.add_argument(
        "--romulusall",
        help="Run Romulus with transaction sizes of 2, 4, 8 and 16",
        action="store_true",
    )

    parser.add_argument(
        "--originallftt", help="Run non persistent lftt", action="store_true"
    )

    parser.add_argument(
        "--mnemosyneall",
        help="Run Mnemosyne with transaction sizes of 2, 4, 8, and 16",
        action="store_true",
    )

    parser.add_argument("--db", help="Run Database benchmarks", action="store_true")

    args = parser.parse_args()

    if len(sys.argv) < 2 or (len(sys.argv) == 2 and args.debug):
        print(
            "Please supply a flag before running. The flags are listed by using -h when running this script"
        )
        exit()

    if args.raw:
        if not args.files:
            print("Please use the --files flag and provide filenames to convert")
        else:
            raw_to_ratio(args.files)

    if args.lftt:
        lftt()

    if args.petraall:
        petraall()
    if args.dramlftt:
        dramlftt()

    if args.romulus:
        romulus()

    if args.pmdk:
        pmdk()

    if args.mnemosyne:
        mnemosyne()

    if args.db:
        db()

    if args.romulusall:
        romulusall()

    if args.originallftt:
        originallftt()

    if args.mnemosyneall:
        mnemosyneall()
