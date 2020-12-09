import numpy as np
# === STRATEGY ===
states = [([2],7, 7),
    ([2,1],3, 7),
    ([2,3],1, 7),
    ([2,6],1, 7),
    ([2,8],3, 7),
([1],3, 3),
    ([1,2],6, 3),
    ([1,6],8, 3),
    ([1,7],6, 3),
    ([1,8],6, 3),
]

def add_symmetries(states):
    new_states = []
    for s,a,past in states:
        new_states.append(((np.array(s)+5)%10,(a+5)%10, (past+5)%10))
        new_states.append((s,a, past))
    
    ret = []
    for s,a,past in new_states:
        ret.append((9-np.array(s), 9-a, 9-past))
        ret.append((s,a, past))

    ret = [(tuple(s), a, past) for s,a,past in ret]
    return ret

def add_third_move(states):
    ret = []
    fields = set([1,2,3,6,7,8])
    for s,a,past in states:
        if(len(s)==2):
            a,b = fields.difference(set(list(s)+[a,past]))
            ret.append((list(s)+[a], b))
            ret.append((list(s)+[b], a))
        else:
            ret.append((s,a))
    return ret

# === Symmetry and third move ===
all_states = set(add_symmetries(states))
all_states = add_third_move(all_states)
#print(len(all_states), all_states)

# === Write to file ===
out = open("boards/forbidden1.txt", "w")
out.write("{} 5 10\n".format(len(all_states)))
for s,a in all_states:
    s = [str(e) for e in s]
    out.write(str(len(s))+" "+" ".join(s)+" "+ str(a)+"\n")

