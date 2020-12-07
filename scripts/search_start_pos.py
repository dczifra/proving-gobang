import os
import subprocess
from subprocess import Popen, PIPE, STDOUT, check_output


def get_result(start, seconds):
    cmd = ["./AMOBA", "-start", str(start)]
    try:
        inpfile = open("../boards/cross_board_easy.txt")
        p = subprocess.run(cmd, timeout=seconds, stdin =inpfile, text=True, capture_output = True)
        return p.stdout.split('PN: ')[1][0]=='0'
    except subprocess.TimeoutExpired:
        #print("Timeout")
        return False



if(__name__ == "__main__"):
    # === Build ===
    os.chdir("build")

    # === Search for turning point ===
    for i in range(5):
        for j in range(10):
            num = 5*j+i
            if(num in [0,1,2,3,4,5,9,10,14,35,39,40,44,45,46,47,48,49]):
                print('#', end='', flush=True)
            else:
                pn = get_result(num, 30)
                if(pn):
                    print('O', end='', flush=True)
                else:
                    print('X', end='', flush=True)
        print('')
        #print(i, end=' ')
        #print(i,pn)