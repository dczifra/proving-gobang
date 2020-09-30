#pragma once
#include<iostream>

#include "common.h"
#include "board.h"
#include "PNS.h"

struct Args;

class Play{
public:
    Play(std::string filename, bool disproof);
    void play_with_solution();
    void play_with_solution2();

    void build_tree();
    void build_node(Board b);

    static NodeType choose_problem(Board& b, int& player, bool disproof);

private:
    int move_human();
    Board move_in_solution(int i, int& act, std::vector<int>& color);
    void read_solution(std::string filename);

private:
    // === Init variables ===
    PNS tree;
    Board board;
    int human_player, player;
};