#pragma once
#include<iostream>

#include "common.h"
#include "board.h"
#include "PNS.h"

#include <map>
struct Args;

class Play{
public:
    Play(std::string filename, bool disproof, bool talky, Args* args_);
    void play_with_solution();

    void build_tree();
    void build_node(Board b);
    void build_node2(PNSNode* node);
    void build_licit_node(const Board& b, int action);
    static void read_solution(std::string filename, PNS& mytree);

    static NodeType choose_problem(Board& b, int& player, bool disproof, Args* args);

private:
    int move_human();
    Board move_in_solution(int i, int& act, std::vector<int>& color);

private:
    // === Init variables ===
    PNS tree;
    int human_player, player;
    std::map<std::vector<uint64_t>, PNSNode*> isom_map;

    bool talky;
    Args* args;
};
