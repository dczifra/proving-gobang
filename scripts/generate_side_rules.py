import numpy as np
import itertools
# === STRATEGY ===
def get_opt_strat():
    # state = ([attacker moves], [defender response])
    states = [([2],[6]),
        ([2,7], [3, 6]),
        ([2,3], [1, 6]),
        ([2,1], [3, 6]),
        ([2,8], [3, 6]),
    ([1],[2]),
        ([1,3], [7, 2]),
        ([1,6], [7, 2]),
        ([1,7], [3, 2]),#6,8
        ([1,8], [7, 2]),
    ]
    #([1],[7]),
    #    ([1,2], [3, 7]),
    #    ([1,6], [2, 7]),
    #    ([1,3], [2, 7]),
    #    ([1,8], [2, 7]),
    #]
    return states

def get_all_strategy0():
    all_fields = set([1,2,3,6,7,8])
    all_strat = []
    for att1 in [1,2]: #
        move_i = []
        for resp1 in all_fields.difference([att1]):
            strats_1 = []
            for att2 in all_fields.difference([att1, resp1]): #
                strats_2 = []
                for resp2 in all_fields.difference([att1, att2, resp1]):
                    strat = [([att1, att2], [resp1, resp2])]
                    strats_2.append(strat)
                strats_1.append(strats_2)
            move_i+=list(itertools.product(*strats_1))
            #for strat in list(itertools.product(*strats_1)):
            #    print(strat)
        print(len(move_i))
        all_strat.append(move_i)
    return all_strat

def get_strat(att1, def1):
    all_fields = set([1,2,3,6,7,8])
    all_strat = []

    all_attack_resp = []
    for att2 in all_fields.difference([att1, def1]): #
        resp_for_att = []
        for def2 in all_fields.difference([att1, att2, def1]):
            resp_for_att.append(([att1, att2], [def1, def2]))

        all_attack_resp.append(resp_for_att)
    return list(itertools.product(*all_attack_resp))
    
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
def generate_opt_strat(strat):
    all_states = set(add_symmetries(strat))
    #all_states = add_third_move(all_states)
    all_states = cleanup(all_states)
    #print(len(all_states), all_states)

    # === Write to file ===
    out = open("boards/forbidden1.txt", "w")
    out.write("{} 5 10\n".format(len(all_states)))
    for s,a in all_states:
        s = [str(e) for e in s]
        a = [str(e) for e in a]
        out.write(str(len(s))+" "+" ".join(s)+" "+" ".join(a)+"\n")
    out.close()
#strats = get_strat(2,7)
#print(len(strats))