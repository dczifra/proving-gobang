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

    //inline bool black_win_nolines(const std::vector<board_int> & all_lines) const {
    //    // === No free lines ===
    //    return no_free_lines(all_lines);
    //}

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

    bool heuristic_stop(const std::vector<std::pair<board_int, unsigned int>>& all_lines) const{
        double sum = 0;
        for(auto line: all_lines){
            bool is_free = !(line.first & black);
            if(!is_free) continue;
            else{
                int emptynum = line.second - __builtin_popcountll(line.first & white);
                sum += std::pow(2.0,-emptynum);
                if(sum>=1.0) return false;
            }
        }

        // The sum is under 1, the game is over
        return true;
    }

    void flip(){
        board_int w=0,b=0;
        board_int col = 0x0f;

        for(int i=0;i<COL;i++){
            board_int old_w = (white & (col<<(4*i)));
            board_int old_b = (black & (col<<(4*i)));
            int move = (COL-2*(i+1)+1)*4;
            if(move>=0){
                w |= (old_w<<move);
                b |= (old_b<<move);
            }
            else{
                move = -move;
                w |= (old_w>>move);
                b |= (old_b>>move);
            }
        }
        white = w;
        black = b;
        //white = static_cast<board_int>(flip_bit(white))>>FLIP_SIZE;
        //black = static_cast<board_int>(flip_bit(black))>>FLIP_SIZE;
    }


    // === Policy ===
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

    // ==============================================
    //               SPLIT TO COMPONENTS
    // ==============================================
    void get_component(const std::vector<std::array<bool,10>>& adjacent_nodes,
                        std::vector<int>& node_component,
                        int start, int act_component){
        for(int dir=0;dir<10;dir++){
            if(adjacent_nodes[ind][dir] && node_component[ind] ==-1 ){
                node_component[ind] = act_component;
                get_components(adjacent_nodes, node_component, ind, act_component);
            }
        }
    }

    std::vector<int> get_free_fields_graph(const std::vector<Line_info>& all_lines){
        std::vector<int> emptynum_in_line(all_lines.size());         // -1 if not empty
        std::vector<bool> free_node(ACTION_SIZE, 0);
        std::vector<int> node_component(ACTION_SIZE, -1);
        std::vector<std::array<bool,10>> adjacent_nodes(ACTION_SIZE);// 3 and 7 unused
        // node1-node2 : if node1 > node2 [+5]
        // -5 -1  3
        // -4  #  4
        // -3  1  5
        int iter = 0;

        // === Iterate on lines ===
        for(auto line: all_lines){
            bool is_free = !(line.line_board & black);

            if(!is_free){
                emptynum_in_line[iter] = -1;
            }
            else{
                // === This line is free, update it's fileds
                int emptynum = line.size - __builtin_popcountll(line.line_board & white);
                emptynum_in_line[iter] = emptynum;

                free_node[line.points[0]] = true;
                for(int i=1;i<line.points.size();i++){ // We doesn't matter with lines with length 1
                    int act = line.points[i];
                    int prev = line.points[i-1];
                    free_node[act] = true;
                    
                    int direction = act-prev + 5; // Only for 4xn table
                    adjacent_nodes[act][direction] = true;
                    direction = prev-act + 5;     // Only for 4xn table
                    adjacent_nodes[prev][direction] = true;
                }
            }
            iter += 1;
        }

        // === Get components ===
        int act_component = 0;
        for(int ind = 0; ind<ACTION_SIZE; ind++){
            if(!free_node[i] || node_component[ind] != -1) continue;
            else{
                get_component(adjacent_nodes, node_component, start, act_component);
                act_component++;
            }
        }

        // === SUM lines on components ===
        // choose free node previously...

        return emptynum_in_line;
    }

    void remove_small_components(const std::vector<Line_info>& all_lines){
        std::vector<int> emptynum_in_line = get_free_fields_graph(all_lines);

    }
};



#endif