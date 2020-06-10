# prooving-gobang

This is a project in order to proove (= Does exists win-strategy for player one ?) 7-in-a-row game on infinite board

# Key Components:

## PNS

Simple proof number search algo.

TODO:
PNS^2, PNS* for memory problems.

## Board

* The board is saved in a 2x64 bit unsigned integer (white-black board).
* You can do the below actions very fast for the current board:
    * move
    * get winner
    * get heuristic mtx

TODO:
    * link the lines here, instead og give them as parameter

## Heuristic:

* This component generates the lines for the board, and is a placeholder for them.

## Common:

* Global parameters, plots, and useful functions

# Other not important components

## MCTS

* This is not relevant yet to the project yet.

## Tree

* Component for MCTS, not used yet

# Compile:

```
mkdir build
cmake ..
make
./AMOBA
```