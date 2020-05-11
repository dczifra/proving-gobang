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

    inline void move(const int action, const int player){
        if(player == 1) white |= ((1ULL)<<action);
        else if (player == -1) black |= ((1ULL)<<action);
        else{
            std::cout<<"Bad player\n";
        }
    }
    
    inline bool is_valid(const int action){
        return !((white | black) & ((1ULL)<<action));
    }

    inline board_int get_valids(){
        return ~(white | black) & FULL_BOARD;
    }

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
        // === No free lines ===
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

    std::array<float, ACTION_SIZE> heuristic_mtx(const std::vector<Line_info>& lines) const{
        // Returns a heuristic value for every possible action
        std::array<float, ACTION_SIZE> mtx= {0};

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