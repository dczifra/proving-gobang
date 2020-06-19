#ifndef BOARD_H_
#define BOARD_H_

#include <stdio.h>
#include <cstdint>
#include <math.h>
#include <cmath>

#include "common.h"

bool operator<(const Board& b1, const Board& b2);

struct Board{
    board_int white;
    board_int black;

    Board(){
        init();
    }
    Board(const Board& b){
        white = b.white;
        black = b.black;
    }
    Board(const Board& b, int action, int player){
        white = b.white;
        black = b.black;
        move(action, player);
    }

    Board& operator=(const Board&& b){
        white = b.white;
        black = b.black;
        return *this;
    }

    inline void init(){
        white = 0;
        black = 0;
    }

    // === ACTION FUNCTIONS ===
    inline void move(const int action, const int player){
        if(player == 1) white |= ((1ULL)<<action);
        else if (player == -1) black |= ((1ULL)<<action);
        else{
            std::cout<<"Bad player\n";
        }
    }

    inline void move(std::vector<int> actions, int& player){
        for(auto& act: actions){
            move(act, player);
            player = -player;
        }
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

    inline bool is_valid(const int action) const{
        return !((white | black) & ((1ULL)<<action));
    }

    inline board_int get_valids(){
        return ~(white | black) & FULL_BOARD;
    }

    // === GAME OVER FUNCTIONS===
    bool white_win(const std::vector<board_int> & lines)const {
        for(auto line: lines){
            bool blocked = (line & black);
            if(!blocked && (__builtin_popcountll(line & white)==__builtin_popcountll(line))){
                return true;
            }
        }
        return false;
    }

    inline bool black_win() const {
        // === No free field ===
        return __builtin_popcountll(white | black) == MAX_ROUND;
    }

    int get_winner(const std::vector<board_int> & lines) const {
        if(white_win(lines)) return 1;
        else if (black_win())
        {
            return -1;
        }
        else return 0;
        
    }

    // === TODO with saved constant array ===
    bool no_free_lines(const std::vector<std::pair<board_int, unsigned int>>& all_lines) const{
        for(auto line: all_lines){
            bool is_free = !(line.first & black);
            if(is_free) return false;
        }
        return true;
    }

    bool heuristic_stop(const std::vector<std::pair<board_int, unsigned int>>& all_lines) const;
    void flip();

    // === Policy ===
    std::array<float, ACTION_SIZE> heuristic_mtx(const std::vector<Line_info>& lines) const;
    
    // ==============================================
    //               SPLIT TO COMPONENTS
    // ==============================================
    void get_component(const std::vector<std::array<bool,11>>& adjacent_nodes,
                        std::vector<int>& node_component,
                        int start, int act_component);

    std::vector<int> get_free_fields_graph(const std::vector<Line_info>& all_lines);
    void remove_small_components(const std::vector<Line_info>& all_lines);
};


#endif