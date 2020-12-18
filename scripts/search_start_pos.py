import os
import subprocess
from subprocess import Popen, PIPE, STDOUT, check_output
from generate_side_rules import get_strat, generate_opt_strat
import time
import itertools

def get_result(args, seconds):
    cmd = args
    try:
        #p = subprocess.run(cmd, timeout=seconds, stdin =inpfile, text=True, capture_output = True)
        p = subprocess.run(cmd, timeout=seconds,  universal_newlines=True, stdout=PIPE, stderr=PIPE)
        #p = subprocess.run(cmd, timeout=seconds,  universal_newlines=True)
        return p.stdout.split('PN: ')[1][0]=='0'
    except subprocess.TimeoutExpired:
        #print("Timeout")
        return False

def start_wins():
    # === Search for turning point ===
    for i in range(5):
        for j in range(10):
            num = 5*j+i
            if(num in [0,1,2,3,4,5,9,10,14,35,39,40,44,45,46,47,48,49]):
                print('#', end='', flush=True)
            else:
                args = ["./AMOBA", "-start", str(num)]
                pn = get_result(args, 30)
                if(pn):
                    print('O', end='', flush=True)
                else:
                    print('X', end='', flush=True)
        print('')
        #print(i, end=' ')
        #print(i,pn)

def generate_chosen_strat(firsts, seconds, f,s, att1, def1, att2, def2):
    f = firsts[f]
    s = seconds[s]
    generate_opt_strat(list(f)+list(s)+[([att1], [def1])]+[([att2], [def2])])

def get_important_side_strat(strats, base, indexes, limit):
    #base = [([att1], [def1])]
    imp = []
    for i in indexes:
        print('\r', i,end='   ', flush=True)
        f = strats[i]
        generate_opt_strat(list(f)+base)
        os.chdir("build")
        args = ["./AMOBA"]
        begin = time.time()
        pn = get_result(args, limit)
        end = time.time()
        if(not pn):
            imp.append(i)
        #print(i,end-begin)
        os.chdir("..")
    print("\rTime:", limit, imp, '==>', len(imp))
    return imp

def get_all_promising_side_strat():
    promising = {}
    for att1,defs in [(2,[1,6,7]), (1,[2,3,6,7,8])]:
        for def1 in defs:
            firsts = get_strat(att1, def1)
            print(att1, def1)
            imps = get_important_side_strat(firsts, [([att1], [def1])], range(len(firsts)), 0.5)
            imps = get_important_side_strat(firsts, [([att1], [def1])], imps, 5)
            imps = get_important_side_strat(firsts, [([att1], [def1])], imps, 20)
            imps = get_important_side_strat(firsts, [([att1], [def1])], imps, 60)
            promising[(att1,def1)]=imps
    print(promising)
    return promising

def joint_promising_strat(promising, att1, def1, att2, def2, tlimit):
    firsts = get_strat(att1, def1)
    seconds = get_strat(att2, def2)
    base = [([att1], [def1])]+[([att2], [def2])]
    imps = []
    for f,s in itertools.product(promising[(att1,def1)], promising[(att2,def2)]):
        print('\r', f,s,end='   ', flush=True)
        generate_opt_strat(list(firsts[f])+list(seconds[s])+base)
        os.chdir("build")
        args = ["./AMOBA"]
        begin = time.time()
        pn = get_result(args, tlimit)
        end = time.time()
        if(not pn):
            imps.append((f,s))
        print("(Att1 {},Def1 {}): {}".format(att1,def1,f),
              "(Att2 {},Def2 {}): {}".format(att2,def2,s),'==>', end-begin)
        os.chdir("..")
    print("\rTime:", tlimit, imp, '==>', len(imp))
    return imp

def generate_sol(f,s,att1,def1,att2,def2):
    firsts = get_strat(att1, def1)
    seconds = get_strat(att2, def2)
    base = [([att1], [def1])]+[([att2], [def2])]

    generate_opt_strat(list(firsts[f])+list(seconds[s])+base)


if(__name__ == "__main__"):
    #promising = get_all_promising_side_strat()
    promising = {}
    promising[(2,6)]=[0, 1, 3, 4]
    promising[(1,2)] = [17, 44, 71]
    joint_promising_strat(promising, 2,6,1,2,600)

    # === Write sol ===
    generate_sol(0,0,2,6,1,2)
