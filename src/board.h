#ifndef BOARD_H_
#define BOARD_H_

#include <stdio.h>
#include <cstdint>
#include <math.h>

struct Board{
    board_int white;
    board_int black;

    Board(){
        init();
    }

    inline void init(){
        white = 0;
        black = 0;
    }

    inline void move(const int action, const bool player){
        if(player == 0) white |= ((1ULL)<<action);
        else black |= ((1ULL)<<action);
    }
    
    inline bool is_valid(const int action){
        return ~(white | black) & ((1ULL)<<action);
    }

    inline board_int get_valids(){
        return ~(white | black) & FULL_BOARD;
    }

    bool white_win(const std::vector<board_int> & lines){
        for(auto line: lines){
            bool blocked = (line & black);
            if(!blocked && (__builtin_popcountll(line & white)==__builtin_popcountll(line))){
                return true;
            }
        }
        return false;
    }

    inline bool black_win(){
        // === No free lines ===
        return __builtin_popcountll(white & black) == BITSIZE;
    }

    // === TODO with saved constatnt array ===
    bool no_free_lines(const std::vector<board_int>& all_lines){
        for(auto line: all_lines){
            bool is_free = !(line & black);
            if(is_free) return false;
        }
        return true;
    }

    inline int random_action(){
        board_int valids = get_valids();
        int number_of_ones =__builtin_popcountll(valids);
        return selectBit(valids, 1 + (rand() % number_of_ones))-1;
    }

    inline int take_random_action(int player){
        int act = random_action();
        move(act, player);
        return act;
    }

    std::array<float, ACTION_SIZE> heuristic_mtx(const std::vector<Line_info>& lines){
        // Returns a heuristic value for every possible action
        std::array<float, ACTION_SIZE> mtx= {0};
        //for(int i=0;i<mtx.size();i++) mtx

        for(auto line: lines){
            bool is_free = !(line.line_board & black);
            if(!is_free) continue;
            else{
                int emptynum = line.size - __builtin_popcountll(line.line_board & white);
                for(int field: line.points){
                    mtx[field] += std::pow(2.0,-emptynum);
                }
            }
        }
        return mtx;
    }
};

#endif