#pragma once

#include "common.h"
#include "board.h"
#include "PNS.h"

class Node;

void add_descendents(Node* node, PNS& tree, int depth, int maxdepth,
                    std::map<Board, std::pair<int,int>>& ors,
                    std::map<Board, std::pair<int,int>>& ands);

void prove_node(Args& args);

struct Descendents{
    std::string filename;
    std::map<Board, std::pair<int,int>> boards; // Depth and occurance
    Descendents(std::string fname):filename(fname){
        
    }
};

void generate_roots_descendents(Args& args, int depth = 3);

void read_descendents(Node* node, PNS& tree, int depth, int maxdepth, std::string foldername);

void merge_solutions(Args& args, std::string filename);