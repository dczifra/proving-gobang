from subprocess import Popen, STDOUT, PIPE

import os
import sys
import random
import multiprocessing

import resource

def run_board(b, ulimit):
    def limit_virtual_memory():
        resource.setrlimit(resource.RLIMIT_AS, (ulimit*(1024**3), resource.RLIM_INFINITY))


    p = Popen(["./AMOBA","--parallel", "--log"], preexec_fn=limit_virtual_memory,
                stdout=PIPE, stdin=PIPE, stderr=STDOUT, bufsize=1, universal_newlines=True)

    out,err = p.communicate(b)
    
    if(out.split('PN: ')[1][0]=='0'): return "PN"
    elif(out.split('PN: ')[1][0]=='0'): return "DN"
    else: return "fail"

def print_res(result):
    log[result]+=1
    print("\r{}/{} [failed: {}] [proof: {}]".format(log["DN"], log["all"], log["fail"], log["PN"]), flush=True, end=" ")

def run_all_board(filename, procnum):
    with open(filename, "r") as file:
        boards = file.read().split('\n')
        boards = boards[1:-1]
        random.shuffle(boards)
    
    log["all"] = len(boards)
    pool = multiprocessing.Pool(processes=procnum)
    for b in boards:
        r = pool.apply_async(run_board, args =(b,0.5), callback=print_res)

    pool.close()
    pool.join()

global log
log={
    "PN":0,
    "DN":0,
    "fail":0,
    "all":None
}

if(__name__ == "__main__"):
    run_all_board(sys.argv[1], 2)