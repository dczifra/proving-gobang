import os
import subprocess
from subprocess import Popen, PIPE, STDOUT, check_output
from generate_side_rules import get_strat, generate_opt_strat
import time

def get_result(args, seconds):
    cmd = args
    try:
        inpfile = open("../boards/cross_board_easy.txt")
        p = run(cmd, timeout=seconds, stdin =inpfile, text=True, capture_output = True)
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

def generate_chosen_strat(firsts, seconds):
    f = firsts[76]
    s = seconds[5]
    generate_opt_strat(list(f)+list(s)+[([att1], [def1])]+[([att2], [def2])])

def get_important_side_strat(strats, base, indexes, limit):
    #base = [([att1], [def1])]
    imp = []
    for i in indexes:
        f = strats[i]
        generate_opt_strat(list(f)+base)
        os.chdir("build")
        args = ["./AMOBA", "-log"]
        begin = time.time()
        pn = get_result(args, limit)
        end = time.time()
        if(not pn):
            imp.append(i)
        os.chdir("..")
    print("Time:", time, imp)
    return imp

if(__name__ == "__main__"):
    # === Build ===
    #start_wins()
    att1=2
    def1=8
    firsts = get_strat(att1, def1)
    
    att2 = 1
    def2 = 7
    seconds = get_strat(att2, def2)
    #second = [ ([1],[2]),([1,3], [7, 2]),([1,6], [7, 2]),([1,7], [3, 2]),([1,8], [7, 2])]
    #second = [([1],[7]),([1,2], [3, 7]),([1,6], [2, 7]),([1,3], [2, 7]),([1,8], [2, 7])]
    
    imps = get_important_side_strat(firsts, [([att1], [def1])], range(len(firsts)), 1)
    imps = get_important_side_strat(firsts, [([att1], [def1])], imps, 5)

    exit(1)

    #for i,s in enumerate(firsts[:]):
    for i in [0,1,3,4,9,10,12,13]:
    #print(len(seconds))
    #for i in [49,52,67,70,76]:
        f = firsts[i]
        for j in range(len(seconds)):
        #for j in [5]:
            s = seconds[j]
            #print(list(s)+second+[([att1], [def1])])
            generate_opt_strat(list(f)+list(s)+[([att1], [def1])]+[([att2], [def2])])
            os.chdir("build")
            args = ["./AMOBA", "-log"]
            begin = time.time()
            pn = get_result(args, 5)
            end = time.time()
            if(pn):
                pass
                #print(i,j,end-begin, "Bad")
            else:
                print(i,j,end-begin, "Good")
                #print(list(s)+[([att1], [def1])])
            os.chdir("..")