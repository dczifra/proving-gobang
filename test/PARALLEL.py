from subprocess import Popen, STDOUT, PIPE

import os
import sys
import random
import multiprocessing
import subprocess

import resource

def get_lic_virt_mem(ulimit):
    def limit_virtual_memory():
        resource.setrlimit(resource.RLIMIT_AS, (ulimit*(1024**3), resource.RLIM_INFINITY))
    return limit_virtual_memory

def run_board(b, ulimit):
    p = Popen(["./AMOBA","--parallel"], preexec_fn=get_lic_virt_mem(ulimit),
              stdout=PIPE, stdin=PIPE, stderr=STDOUT, bufsize=1, universal_newlines=True)

    out,err = p.communicate(b)

    if(len(out.split('PN'))==1): return "fail",b
    if(out.split('PN: ')[1][0]=='0'): return "PN",b
    elif(out.split('DN: ')[1][0]=='0'): return "DN",b
    else: return "fail",b

def print_res(arg):
    result,b = arg
    log[result]+=1
    if(result == "PN"):
        print("Proof", b)
    elif(result == "fail"):
        #print("Fail", b)
        log["again"].append(b)
    else:
        print("\r{}/{} [failed: {}] [proof: {}]".format(log["DN"], log["all"], log["fail"], log["PN"]), flush=True, end=" ")


def run_all_board(boards, procnum, memory_limit):
    log["all"] = len(boards)
    pool = multiprocessing.Pool(processes=procnum)
    for b in boards:
        r = pool.apply_async(run_board, args =(b,memory_limit), callback=print_res)                                                                                  
        #print_res(run_board(b, memory_limit))
    pool.close()
    pool.join()

global log
log={
    "PN":0,
    "DN":0,
    "fail":0,
    "all":None,
    "again":[],
}

if(__name__ == "__main__"):
    with open(sys.argv[1], "r") as file:
        boards = file.read().split('\n')
        boards = boards[1:-1]
        random.shuffle(boards)
        print(len(boards))
    
    run_all_board(boards, 15, 30)
    print("Summary: {}/{} [failed: {}] [proof: {}]".format(log["DN"], log["all"],
                                                           log["fail"], log["PN"]))
    with open("../failed.txt", "w") as f:
        for b in log["again"]:
            f.write(b+"\n")

    run_all_board(log["again"], 2, 150)
    print("Summary: {}/{} [failed: {}] [proof: {}]".format(log["DN"], log["all"],
                                                           log["fail"], log["PN"]))