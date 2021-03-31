from subprocess import Popen, STDOUT, PIPE

import os
import sys
import time
import random
import pandas
import resource
import subprocess
import itertools
import multiprocessing

def get_lic_virt_mem(ulimit):
    def limit_virtual_memory():
        resource.setrlimit(resource.RLIMIT_AS, (ulimit*(1024**3), resource.RLIM_INFINITY))
    return limit_virtual_memory

def set_param(params,value):
    for p in params:
        os.system("sed -i 's/#define {}.*/#define {} '{}'/g' ../src/params.h".format(p,p,value))

def set_col(col = 8):
    os.system("sed -i 's/#define COL.*/#define COL '{}'/g' ../src/common.h".format(col))

def run_experiment(params, args):
    set_param(params,"true")
    
    os.system("make -j8 >/dev/null")
    #print("Param {} is set to {}".format(params,"true"))

    p = Popen(["./AMOBA", *args], preexec_fn=get_lic_virt_mem(8),
            stdout=PIPE, stdin=PIPE, stderr=STDOUT, bufsize=1)
    
    start = time.time()
    out,err = p.communicate()
    end = time.time()

    out = out.decode('ascii')
    #print(out)
    if(out.split("\n")[-2].split(": ")[-1] == ' std::bad_alloc'):
        tree_size = -1
    else:
        tree_size = int(out.split("\n")[-2].split(": ")[-1])

    print("Tree size:", tree_size, "time: ",end-start)
    
    set_param(params,"false")
    return tree_size, end-start

def add_to_base(base, extra, cols, args, logfile):
    file = open(logfile, "w")
    for col in cols:
        set_col(col)
        print("Experiment with base params: {} [COL {}]".format(base, col))
        print(extra+["all"])
        file.write("Col: {}\n".format(col))
        file.write(" ".join([str(a) for a in extra+["all"]])+"\n")
        tree_sizes = []
        times = []
        for p in itertools.chain(*[extra, [extra]]):
            print('\r', p, end=' ', flush=True)
            if(p == "COMPONENTS"):
                act_params = base+["COMPONENTS", "REMOVE_DEAD_FIELDS"]
            elif(type(p)==str): act_params = base+[p]
            else: act_params = base+p
            
            tree_size, time0 = run_experiment(act_params, args)
            tree_sizes.append(tree_size)
            times.append(time0)

        # === All ===
        print("Visited Nodes", tree_sizes)
        print("Runtimes", times)
        file.write(" ".join([str(a) for a in tree_sizes])+"\n")
        file.write(" ".join([str(a) for a in times])+"\n")
    file.close()

if(__name__ == "__main__"):
    base_params = ["HEURISTIC_STOP", "REMOVE_DEAD_FIELDS", "REMOVE_2_LINE",
                   "REMOVE_LINE_WITH_2x1_DEGREE", "ONE_WAY", "TRANSPOSITION_TABLE",
                   "HEURISTIC_PN_INIT", "HEURISTIC_DN_INIT", "COMPONENTS", "ISOMORPHIC_TABLE"]

    # === Base measure ===
    params = ["HEURISTIC_STOP", "REMOVE_DEAD_FIELDS", "REMOVE_2_LINE",
                   "REMOVE_LINE_WITH_2x1_DEGREE", "ONE_WAY", "TRANSPOSITION_TABLE",
                   "HEURISTIC_PN_INIT", "HEURISTIC_DN_INIT", "COMPONENTS", "ISOMORPHIC_TABLE"]

    add_to_base([], params+["vanilla"], [7,8,9], [], "result_proof.csv")
    add_to_base([], params+["vanilla"], [7,8,9], ["--disproof"], "result_disproof.csv")

    # === Advanced ===
    params = ["HEURISTIC_STOP", "COMPONENTS",
                   "HEURISTIC_PN_INIT", "HEURISTIC_DN_INIT","ISOMORPHIC_TABLE"]
    add_to_base(["TRANSPOSITION_TABLE", "REMOVE_DEAD_FIELDS", "REMOVE_2_LINE", "REMOVE_LINE_WITH_2x1_DEGREE", "ONE_WAY"],
            params+["vanilla"], [7,8,9,10,11], [], "advaced_proof.csv")
    add_to_base(["TRANSPOSITION_TABLE", "REMOVE_DEAD_FIELDS", "REMOVE_2_LINE", "REMOVE_LINE_WITH_2x1_DEGREE", "ONE_WAY"],
            params+["vanilla"], [7,8,9,10], ["--disproof"], "advanced_disproof.csv")
