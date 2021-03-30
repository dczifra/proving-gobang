from subprocess import Popen, STDOUT, PIPE

import os
import sys
import time
import random
import multiprocessing
import subprocess
import resource
import pandas

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
    if(out.split("\n")[-2].split(": ")[-1] == ' std::bad_alloc'):
        tree_size = -1
    else:
        tree_size = int(out.split("\n")[-2].split(": ")[-1])

    print("Tree size:", tree_size, "time: ",end-start)
    
    set_param(params,"false")
    return tree_size, end-start

if(__name__ == "__main__"):
    base_params = ["HEURISTIC_STOP", "REMOVE_DEAD_FIELDS", "REMOVE_2_LINE", "REMOVE_LINE_WITH_2x1_DEGREE",
              "ONE_WAY", "TRANSPOSITION_TABLE", "ISOMORPHIC_TABLE", "HEURISTIC_PN_DN_INIT"]

    params = ["HEURISTIC_STOP", "REMOVE_DEAD_FIELDS", "REMOVE_2_LINE", "REMOVE_LINE_WITH_2x1_DEGREE",
              "ONE_WAY", "TRANSPOSITION_TABLE", "HEURISTIC_PN_DN_INIT"]

    df = pandas.DataFrame(columns = base_params+["col", "time", "tree_size"])
    for p in base_params:
        df[p]=False

    print(df.columns)
    for col in [7,8]:
        set_col(col)
        for p in params:
            act_params = [p]
            tree_size, time0 = run_experiment(act_params)
            args = {"col":col, "tree_size":tree_size, "time":time0}
            for p in act_params: args[p]=True
            df = df.append(args, ignore_index=True)
        
        #run_experiment(params)
        tree_size, time0 = run_experiment(params)
        args = {"col":col, "tree_size":tree_size, "time":time0}
        for p in params: args[p]=True
        df = df.append(args, ignore_index=True)
        
        tree_size, time0 = run_experiment([])
        args = {"col":col, "tree_size":tree_size, "time":time0}
        df = df.append(args, ignore_index=True)
    df.to_csv("results1.csv")

    for col in [7,8,9,10,11]:
        set_col(col)
        for i in range(len(params)-1):
            act_params = params[:i]+params[i+1:]
            tree_size, time0 = run_experiment(act_params)
            args = {"col":col, "tree_size":tree_size, "time":time0}
            for p in act_params: args[p]=True
            df = df.append(args, ignore_index=True)
    
    df.to_csv("results2.csv")
