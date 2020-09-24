#pragma once
#include "common.h"
#include "board.h"

class Artic_point{
public:
    int start;
    int reach_time = 0;
    int reached_nodes = 0;
    int empty_nodes;
    std::vector<int> parent, depth, low;
    std::vector<int> parent_line, depth_line, low_line;
    std::array<std::vector<Line_info>, ACTION_SIZE>& linesinfo_per_field;
    std::vector<Line_info>& all_linesinfo;
    const Board board;

public:
    Artic_point(const Board& b, std::vector<Line_info>& all_linesinfo,
                std::array<std::vector<Line_info>, ACTION_SIZE>& linesinfo, int no_lines);
    
    Artic_point(const Board& b, std::vector<Line_info>& all_linesinfo,
                std::array<std::vector<Line_info>, ACTION_SIZE>& linesinfo);

    std::pair<int, bool> get_articulation_point_bipartite(int node, int d);
    std::pair<int, bool> get_articulation_point_bipartite_line(Line_info& line, int d);
    std::tuple<int, Board, Board> get_parts();
};