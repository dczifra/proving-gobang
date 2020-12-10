import numpy as np
# === STRATEGY ===
# state = ([attacker moves], [defender response])
states = [([2],[1]),
    ([2,7], [8, 1]),
    ([2,3], [8, 1]),
    ([2,6], [8, 1]),
    ([2,8], [6, 1]),
([1],[3]),
    ([1,2], [6, 3]),
    ([1,6], [8, 3]),
    ([1,7], [6, 3]),
    ([1,8], [6, 3]),
]

def add_symmetries(states):
    new_states = []
    for s,a in states:
        new_states.append(((np.array(s)+5)%10,(np.array(a)+5)%10))
        new_states.append((s,a))
    
    ret = []
    for s,a in new_states:
        ret.append((9-np.array(s), 9-np.array(a)))
        ret.append((s,a))

    ret = [(tuple(att), tuple(defender)) for att,defender in ret]
    return ret

def add_third_move(states):
    ret = []
    fields = set([1,2,3,6,7,8])
    for attacker,defender in states:
        attacker,defender = list(attacker),list(defender)
        if(len(attacker)==2):
            a,b = fields.difference(set(attacker+defender))
            ret.append((attacker+[a], defender+[b]))
            ret.append((attacker+[b], defender+[a]))
        ret.append((attacker, defender))
    return ret

def cleanup(states):
    ret = []
    for attacker,defender in states:
        attacker,defender = list(attacker),list(defender)
        ret.append((attacker, defender))
    return ret

# === Symmetry and third move ===
all_states = set(add_symmetries(states))
all_states = add_third_move(all_states)
all_states = cleanup(all_states)
#print(len(all_states), all_states)

# === Write to file ===
out = open("boards/forbidden1.txt", "w")
out.write("{} 5 10\n".format(len(all_states)))
for s,a in all_states:
    s = [str(e) for e in s]
    a = [str(e) for e in a]
    out.write(str(len(s))+" "+" ".join(s)+" "+" ".join(a)+"\n")

