#pragma once 

#include <iostream>
#include <vector>
#include <array>
#include <cmath>

#include "common.h"


class Heuristic{
public:
    std::vector<Line> lines;
    mtx<std::vector<Line>> lines_per_field;
    std::vector<std::vector<Line>> lines_per_action;

    // === Containes all lines comprassed in line ===
    mtx<std::vector<board_int>> compressed_lines_per_field;
    std::array<std::vector<board_int>, ACTION_SIZE> compressed_lines_per_action;
    // === Containes all lines comprassed, and the points, which lie on the line
    std::vector<std::pair<board_int, std::vector<int>> > fields_on_compressed_lines;

public:
    Heuristic(){
        std::cout<<"Heuristic begin\n";
        generate_lines();
        std::cout<<"Gen lines Done\n";
        generate_compressed_lines();
        std::cout<<"Heuristic end\n";
    }
    
    void generate_lines();
    void generate_compressed_lines();


    template<class T>
    std::array<mtx<T>, LAYERNUM+1> get_layers(mtx<int>& board);

    template<class T>
    std::vector<T> get_flat_layers(mtx<int>& board);

    int getGameEnded(mtx<int>& board, const int action, const int round);
};
