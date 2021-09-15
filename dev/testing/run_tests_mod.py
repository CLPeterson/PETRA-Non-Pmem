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

# Build all of the commands to run and then execute them in random order, run_size amount of times
def run_test():

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
    for dist in dists:
        for tranSize in transactions:
            for numThread in threads:
                insertion = dist[0]
                deletion = dist[1]
                # update = dist[2]

                # this tuple representing the command is used as a key for the num_runs and results dictionaries
                command = (
                    default_dir,
                    setType,
                    numThread,
                    testSize,
                    tranSize,
                    keyRange,
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
        filename = f"raw_{work}_{flag}_{keyRange}.txt"

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
            if num_runs[cur_test] == run_size:
                not_complete.remove(cur_test)
            time.sleep(10)

        # remove persistent memory or mem mapped files for next test
        cleanup()
    print("All Tests completed! Calculating averages now")
    calculate_results(tests, results)
    if not args.debug:
        print("Done calculating averages!\n Cleaning up...")
        os.remove("temp_output.txt")


def calculate_results(tests, results):
    # need to sort the lists if calculating results from raw data, not from the tests running
    if args.raw:
        tests = sorted(list(tests), key=lambda x: int(x[2]))

    for test in tests:
        calculate_average(test, results)
        calculate_median(test, results)


# using the results from the test, calculate the average of each data point for the 10 runs
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

    # If we are not running a make clean
    if not clean:
        cppflags = "CPPFLAGS=" + flags + ""
        if args.debug:
            print(cppflags)
            subprocess.Popen(["make", cppflags], cwd="../../compile").wait()
        else:
            log = open("makelog.txt", "w+")
            subprocess.Popen(
                ["make", cppflags], stdout=log, stderr=log, cwd="../../compile"
            ).wait()
            log.close()
    else:
        if args.debug:
            subprocess.Popen(["make", "clean"], cwd="../../compile").wait()
        else:
            log = open("makelog.txt", "w+")
            subprocess.Popen(
                ["make", "clean"], stdout=log, stderr=log, cwd="../../compile"
            ).wait()
            log.close()


# convert raw data files to plot gen format
# in case a test failed but the raw data was saved
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

        if "lfttall" in cur_file:
            work = "lfttall"
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
    global work
    global default_dir
    global transactions
    global setType
    global keyRange
    global flag

    work = "lftt"
    default_dir = "../../compile/src/trans"
    transactions = ["1"]
    setType = "3"

    # make("-DUSE_PMEM_ALLOCATOR -DPWB_IS_CLFLUSH -DROMULUS_LOG_PTM")
    # flag = "CLFLUSH"
    # keyRange="1000"
    # run_test()

    # keyRange="10000"
    # run_test()

    # make("", clean=True)

    print("Testing LFTT CLFLUSHOPT with TX Size 1\n")
    make("-DUSE_PMEM_ALLOCATOR -DPWB_IS_CLFLUSHOPT -DROMULUS_LOG_PTM")
    flag = "CLFLUSHOPT"
    # keyRange = "1000"
    # run_test()

    # keyRange="10000"
    run_test()

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


# run lftt tests with all transaction sizes
def lfttall():
    print("Testing LFTT with TX Size 1 2 4 8 16\n")
    global work
    global default_dir
    global transactions
    global setType
    global keyRange
    global flag

    work = "lfttall"
    default_dir = "../../compile/src/trans"
    transactions = ["1", "2", "4", "8", "16"]
    setType = "0"

    make("-DUSE_PMEM_ALLOCATOR -DPWB_IS_CLFLUSH -DROMULUS_LOG_PTM")
    flag = "CLFLUSH"
    keyRange = "1000"
    run_test()

    keyRange = "10000"
    run_test()

    make("", clean=True)

    make("-DUSE_PMEM_ALLOCATOR -DPWB_IS_CLFLUSHOPT -DROMULUS_LOG_PTM")
    flag = "CLFLUSHOPT"
    keyRange = "1000"
    run_test()

    keyRange = "10000"
    run_test()

    make("", clean=True)

    make("-DUSE_PMEM_ALLOCATOR -DPWB_IS_CLWB -DROMULUS_LOG_PTM")
    flag = "CLWB"
    keyRange = "1000"
    run_test()

    keyRange = "10000"
    run_test()

    make("", clean=True)


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
    default_dir = "../../pmemkv-tools/pmemkv_bench"
    threads = ["1", "2", "4", "8", "16", "48", "96"]
    engines = ["dlfttmap", "cmap_int"]
    benchmarks = [
        "fillrandom,deleterandom,overwrite,fillseq,readrandom,readseq,readmissing,deleteseq",
        "readrandomwriterandom",
    ]
    db_file = "--db=/dev/shm/pmemkv"
    db_size = "--db_size_in_gb=1"
    histogram = "--histogram=1"
    num_runs = {}
    for engine in engines:
        for benchmark in benchmarks:
            for thread in threads:

                command = (
                    f"PMEM_IS_PMEM_FORCE=1 sudo {default_dir} {db_file} {db_size} {histogram} --engine={engine} --threads={thread} --benchmarks={benchmark}",
                    engine,
                    thread,
                )
                num_runs[command] = 10

    while len(num_runs) != 0:
        cur_test = random.choice(list(num_runs.keys()))
        os.system(cur_test[0] + f" >> raw_db_{cur_test[1]}_{cur_test[2]}.txt")
        subprocess.Popen(["make", "reset"], cwd="../../pmemkv-tools").wait()
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
    default_dir = "../../compile/src/transRomulus"
    transactions = ["2", "4", "8", "16"]
    dists = [["50", "50"]]
    setType = "4"

    # make("-DUSE_PMEM_ALLOCATOR -DPWB_IS_CLFLUSH -DROMULUS_LOG_PTM")
    # flag = "CLFLUSH"
    # keyRange="1000"
    # run_test()

    keyRange="10000"
    # run_test()

    # make("", clean=True)

    make("-DUSE_PMEM_ALLOCATOR -DPWB_IS_CLFLUSHOPT -DROMULUS_LOG_PTM")
    flag = "CLFLUSHOPT"
    # keyRange = "1000"
    run_test()

    # keyRange="10000"
    # run_test()

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
    dists = [["33","33"], ["15", "5"]]

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
        "--lfttall",
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
        "--romulusall", help="Run Romulus with transaction sizes of 2, 4, 8 and 16", action="store_true"
    )

    parser.add_argument(
        "--originallftt", help="Run non persistent lftt", action="store_true"
    )

    parser.add_argument(
        "--mnemosyneall", help="Run Mnemosyne with transaction sizes of 2, 4, 8, and 16", action="store_true"
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

    if args.lfttall:
        lfttall()
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