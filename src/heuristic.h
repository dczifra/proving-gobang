#pragma once 

#include <iostream>
#include <vector>
#include <array>
#include <cmath>
#include <map>

#include "common.h"

using Line = std::vector<std::pair<int,int>>;

class Heuristic{
public:
    board_int forbidden_fields_right = 0;
    board_int forbidden_fields_left = 0;
    board_int forbidden_fields_inner = 0;
    board_int forbidden_all = 0;
    std::map<board_int, board_int> side_strategy;


    // === Containes all lines comprassed, and the points, which lie on the line
    std::vector<Line_info> all_linesinfo;
    std::array<std::vector<Line_info>, ACTION_SIZE> linesinfo_per_field;


public:
    Heuristic(){
        std::cout<<"=== Heuristic begin ===\n";
        generate_lines();
        std::cout<<"[Done] Generate lines\n";
        generate_compressed_lines();
        std::cout<<"===Heuristic end ===\n";
    }
    
    void generate_lines();
    void generate_compressed_lines();

    void read_forbidden_strategy();

private:
    std::vector<Line> lines;
    mtx<std::vector<Line>> lines_per_field;
    std::vector<std::vector<Line>> lines_per_action;
};
