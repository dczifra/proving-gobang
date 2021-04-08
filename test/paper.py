from subprocess import Popen, STDOUT, PIPE

import os
import sys
import time
import random
import pandas
import resource
import itertools
import subprocess
import numpy as np
import multiprocessing
from PARALLEL import runfile, run_all_board

convert_map = {
    "HEURISTIC_STOP":"Breaker win stop",
    "REMOVE_DEAD_FIELDS":"Dead square elim",
    "REMOVE_2_LINE":"Half lone 2-line",
    "REMOVE_LINE_WITH_2x1_DEGREE":"Lone 2-line",
    "ONE_WAY":"Forced move",
    "TRANSPOSITION_TABLE":"Transposition table",
    "HEURISTIC_PN_INIT":"Heuristic \pn",
    "HEURISTIC_DN_INIT":"Heuristic \dn",
    "COMPONENTS":"Components",
    "ISOMORPHIC_TABLE":"Isomorphy",
    "all":"Together",
    "vanilla":"vanilla PNS",
}

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
    file.write("Base: "+" ".join(base)+"\n")
    res = {}
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

            if(type(p)==list):k='all'
            else: k=p
            if(k not in res):
                res[k]={col:(time0, tree_size)}
            else:
                res[k][col] = (time0, tree_size)

        # === All ===
        print("Visited Nodes", tree_sizes)
        print("Runtimes", times)
        file.write(" ".join([str(a) for a in tree_sizes])+"\n")
        file.write(" ".join([str(a) for a in times])+"\n")
    file.close()

    for p in res:
        print(p)
        for col in res[p]:
            print("%.2f & %.2f &".format(res[p][col][0], res[p][col][1]), end = ' ')
        print('')

def remove_from_base(base, cols, args):
    res = {}
    for col in cols:
        set_col(col)
        print("Experiment with base params: {} [COL {}]".format(base, col))
        tree_sizes = []
        times = []
        for i in range(len(base)):
            print('\r', base[i], end=' ', flush=True)
            act_params = base[:i]+base[i+1:]
            if(base[i] == "REMOVE_DEAD_FIELDS"):
                act_params.remove("COMPONENTS")

            #print(act_params)
            
            tree_size, time0 = run_experiment(act_params, args)
            tree_sizes.append(tree_size)
            times.append(time0)

            if(i== len(base)):k='all'
            else: k=base[i]
            
            if(k not in res):
                res[k]={col:(time0, tree_size)}
            else:
                res[k][col] = (time0, tree_size)

    for p in res:
        print("{}".format(convert_map[p]), end = ' ')
        for col in res[p]:
            #print("{:.2f} & {:.2f} &".format(res[p][col][0], res[p][col][1]), end = ' ')
            print("& {:.2f} & {:.2f} ".format(
                res[p][col][0]/res['all'][col][0], res[p][col][1]/res['all'][col][1]), end = ' ')
        print('\\')

def table1():
    base = ["TRANSPOSITION_TABLE"]
    params = ["ONE_WAY", "REMOVE_DEAD_FIELDS",
              "REMOVE_LINE_WITH_2x1_DEGREE", "REMOVE_2_LINE", ]

    add_to_base(base, params+["vanilla"], [7,8], [], "table1_proof.csv")
    add_to_base(base, params+["vanilla"], [7,8], ["--disproof"], "table1_disproof.csv")

def table2():
    base = ["TRANSPOSITION_TABLE", "REMOVE_DEAD_FIELDS", "REMOVE_2_LINE",
              "REMOVE_LINE_WITH_2x1_DEGREE", "ONE_WAY"]
    params = ["HEURISTIC_STOP", "HEURISTIC_PN_INIT", "HEURISTIC_DN_INIT",
              "COMPONENTS", "ISOMORPHIC_TABLE"]

    add_to_base(base, params+["vanilla"], [9,10], [], "table2_proof.csv")
    add_to_base(base, params+["vanilla"], [8,9], ["--disproof"], "table2_disproof.csv")

def table_all():
    base_params = ["COMPONENTS", "TRANSPOSITION_TABLE", "HEURISTIC_STOP",
                   "REMOVE_DEAD_FIELDS", "REMOVE_2_LINE",
                   "REMOVE_LINE_WITH_2x1_DEGREE", "ONE_WAY",
                   "HEURISTIC_PN_INIT", "HEURISTIC_DN_INIT", "all"]
    
    remove_from_base(base_params, [7, 11, 12], [])
    remove_from_base(base_params, [8,9], ["--disproof"])

def table3():
    base_params = ["HEURISTIC_STOP", "REMOVE_DEAD_FIELDS", "REMOVE_2_LINE",
                   "REMOVE_LINE_WITH_2x1_DEGREE", "ONE_WAY", "TRANSPOSITION_TABLE",
                   "HEURISTIC_PN_INIT", "HEURISTIC_DN_INIT", "COMPONENTS"]

    maxProof = 13
    maxDisProof = 10
    for cols, args in [(range(7,maxProof+1), []), (range(7,maxDisProof+1), ["--disproof"])]:
        sizes = []
        times = []
        print("{}: {}".format(args, cols))
        for col in cols:
            print("[{}]".format(col), end=' ')
            set_col(col)
            tree_size, time0 = run_experiment(base_params, [])
            sizes.append(tree_size)
            times.append(time0)

        print(np.array(sizes))
        print(np.array(times))

def table4():
    base_params = ["COMPONENTS", "TRANSPOSITION_TABLE", "HEURISTIC_STOP",
                   "REMOVE_DEAD_FIELDS", "REMOVE_2_LINE",
                   "REMOVE_LINE_WITH_2x1_DEGREE", "ONE_WAY",
                   "HEURISTIC_PN_INIT", "HEURISTIC_DN_INIT", "all"]

    set_param(base_params,"true")
    
    for col in range(7,13):
        from PARALLEL import log
        log["DN"]=0
        log["PN"]=0
        log["fail"]=0
        log["again"]=[]
        
        set_col(col)
        os.system("make -j8 >/dev/null")
        os.system("./AMOBA --generate_parallel")
        dn1,fail1,pn1 = runfile("../ors.txt", 10, False)
        dn2,fail2,pn2 = runfile("../ands.txt", 10, False)
        dn_or_fail = dn1+dn2+fail1+fail2
        all = dn_or_fail + pn1 + pn2
        proc = 100*dn_or_fail/all
        print("dn: {}({}) pn:{} /{} {}".format(dn_or_fail, fail1+fail2, pn1+pn2, all, proc))
        print("{} & {} & {} & {}".format(col, dn_or_fail, all, proc))
        run_all_board(log["again"], 2, 150)


    set_param(base_params,"false")
    
if(__name__ == "__main__"):
    #np.set_printoptions(precision=2)
    np.set_printoptions(formatter={'float': lambda x: "{0:0.2f}".format(x)})
    base_params = ["HEURISTIC_STOP", "REMOVE_DEAD_FIELDS", "REMOVE_2_LINE",
                   "REMOVE_LINE_WITH_2x1_DEGREE", "ONE_WAY", "TRANSPOSITION_TABLE",
                   "HEURISTIC_PN_INIT", "HEURISTIC_DN_INIT", "COMPONENTS", "ISOMORPHIC_TABLE"]

    # === Base measure ===
    params = ["HEURISTIC_STOP", "REMOVE_DEAD_FIELDS", "REMOVE_2_LINE",
                   "REMOVE_LINE_WITH_2x1_DEGREE", "ONE_WAY",
                   "HEURISTIC_PN_INIT", "HEURISTIC_DN_INIT", "COMPONENTS", "ISOMORPHIC_TABLE"]


    #table1()
    #table2()
    #table_all()
    #table3()
    table4()
