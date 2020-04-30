#ifndef BOARD_H_
#define BOARD_H_

#include <stdio.h>
#include <cstdint>
#include <math.h>

struct Board{
    board_int white;
    board_int black;

    Board(){
        white = 0;
        black = 0;
    }

    inline void move(const int action, const bool player){
        if(player == 0) white |= ((1)<<action);
        else black |= ((1)<<action);
    }
    
    inline bool is_valid(const int action){
        return ~(white | black) & ((1)<<action);
    }

    inline board_int get_valids(){
        return ~(white | black);
    }

    bool white_win(const std::vector<board_int> & lines){
        for(auto line: lines){
            bool blocked = (line & black);
            std::cout<<__builtin_popcount(line & white)<<" "<<__builtin_popcount(line)<<std::endl;
            if(!blocked && (__builtin_popcount(line & white)==__builtin_popcount(line))){
                return true;
            }
        }
        return false;
    }

    inline bool black_win(){
        // === No free lines ===
        return __builtin_popcount(white & black) == BITSIZE;
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
        int one_num =__builtin_popcount(valids);
        return selectBit(valids, 1 + (rand() % one_num))-1;
    }

    inline int take_random_action(int player){
        int act = random_action();
        std::cout<<act<<std::endl;
        move(act, player);
        return act;
    }

    std::array<float, ACTION_SIZE> heuristic_mtx(const std::vector<std::pair<board_int, std::vector<int>>>& lines){
        // Returns a heuristic value for every possible action
        std::array<float, ACTION_SIZE> mtx;

        for(auto line: lines){
            bool is_free = !(line.first & black);
            if(!is_free) continue;
            else{
                int emptynum = __builtin_popcount(line.first & white);
                for(int field: line.second){
                    mtx[field] = std::pow(2.0,-emptynum);
                }
            }
        }
        return mtx;
    }
};

#endif