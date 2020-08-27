# Proving-gobang

Description:
* This is a project in order find the game theoretic value of the MxNxK problem with M=N=infty, K=7.
With other words: - we want to prove (Does exists win-strategy for player one?) in the 7-in-a-row game on infinite board. 

# Compile:

```
mkdir build
cd build
cmake ..
make
```

This will generate:
* ./AMOBA
* ./AMOBA_DEBUG - same as AMOBA but compiled with debug flag
* ./TEST - simple source file for debugging and test components

AMOBA args:
* --play : play with the fresh-generated solution-tree
* --test : play with a previously generated solution tree. Set the board size in **common.h**, and rebuild the hole with **make**
* --log : prints current pn/dn number of the root, and the size of the transition matrix (std::map<Board, PNSNode*> states). 

# Future improvements

* Task-1:
The most standard initialization strategy for the proof and disproof number is 1-1.
With PNS-square, we can get a more accurate initialization to the nodes, with time penalty, which runs a second search started from the extended node with maximum depth **d**, or a maximum extend number **N_MAX** (Currently, we use PNS-square with **d**=1).
This initialized pn/dn value can be an estimation for the effort (= number of every extended descendent) proving / disproving the current node.
But based on the solution tree on smaller boards, we can learn this initialization value.
If this is perfect, with this initialization strategy PNS will find the solution only extending nodes from the solution tree!
Finding such an initialization function looks as hard, as the original problem, but good initialization strategy can direct the PNS into deeper nodes (what is a well known drawback of PNS). 

# Key Components:

## PNS

The PNS searches a big AND-OR tree for a proof subtree.
In our case the OR nodes are the attacker, the AND nodes are the defender nodes (We are looking for an attacker win strategy (any), that can beat every defender strategy. And if there is none (4x11+??), we want a proof for the nonexistence).

Each node in the search tree containes:

PNSNode:
* The current board                                               (2 integer)
* The proof and disproof number (pn/dn) and threshholds for them  (2-4 integer)
* For every child, the pointer to the child-Node.                 (ROW*COL integer)

Containes:
* PN_search: The original Proof Number Search (PNS) algorithm, with many board simplification, and heuristic.
* DFPN_search: The Depthst First Proof Number Search (DFPN) algorithm.

TODO:
PNS^2, PNS* for memory problems.

## Board

* The board is saved in a 2x64 bit unsigned integer (white-black board).
* You can do the below actions very fast for the current board:
    * move
    * get winner
    * get heuristic mtx

TODO:
    * save the lines here, instead of give them as parameter

## Heuristic:

* This component generates the lines for the board, and is a placeholder for them.

## Common:

* Global parameters, plots, and useful functions

# Other not important/used components

## MCTS

* This is not relevant yet to the project yet.

## Tree

* Component for MCTS, not used yet

# References:
* https://www.researchgate.net/publication/226914512_Proof-Number_Search_and_Its_Variants
