#pragma once

#include <stdio.h>
#include <cstdint>
#include <math.h>
#include <vector>

struct Test_Board{
    /**
     * === NIM Board IMplementation ===
     * 
     * **/

    std::vector<int> columns;

    Test_Board(){
        init();
    }

    Test_Board(const Test_Board& b){
        columns = b.columns;
    }

    Test_Board(const Test_Board& b, int action, int player){
        columns = b.columns;
        move(action, player);
    }

    Test_Board& operator=(const Test_Board&& b){
        columns = b.columns;
        return *this;
    }

    inline void init(){
        columns = {1,2,3};
    }

    inline void move(const int action, const int player){
        
    }
    
    inline bool is_valid(const int action){
        return 1;
    }

    inline board_int get_valids(){
        return 0;
    }

    int get_winner(const std::vector<board_int> & lines) const {
        return 0;
        
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

};