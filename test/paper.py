from subprocess import Popen, STDOUT, PIPE

import os
import sys
import time
import random
import multiprocessing
import subprocess
import resource

def get_lic_virt_mem(ulimit):
    def limit_virtual_memory():
        resource.setrlimit(resource.RLIMIT_AS, (ulimit*(1024**3), resource.RLIM_INFINITY))
    return limit_virtual_memory

def set_param(params,value):
    for p in params:
        os.system("sed -i 's/#define {}.*/#define {} '{}'/g' ../src/params.h".format(p,p,value))

def set_col(col = 8):
    os.system("sed -i 's/#define COL.*/#define COL '{}'/g' ../src/common.h".format(col))

def run_experiment(params):
    set_param(params,"true")
    
    os.system("make -j8 >/dev/null")

    print("Param {} is set to {}".format(params,"true"))

    p = Popen(["./AMOBA"], preexec_fn=get_lic_virt_mem(8),
            stdout=PIPE, stdin=PIPE, stderr=STDOUT, bufsize=1)
    
    start = time.time()
    out,err = p.communicate()
    end = time.time()

    out = out.decode('ascii')
    #print(out)
    tree_size = int(out.split("\n")[-2].split(": ")[-1])

    print("Tree size:", tree_size, "time: ",end-start)
    
    set_param(params,"false")

if(__name__ == "__main__"):
    params = ["HEURISTIC_STOP", "REMOVE_DEAD_FIELDS", "REMOVE_2_LINE", "REMOVE_LINE_WITH_2x1_DEGREE",
              "ONE_WAY", "TRANSPOSITION_TABLE", "ISOMORPHIC_TABLE", "HEURISTIC_PN_DN_INIT"]

    set_col(8)
    for p in params:
        run_experiment([p])
    
    run_experiment(params)
