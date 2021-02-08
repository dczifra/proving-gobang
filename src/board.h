#pragma once

#include <stdio.h>
#include <cstdint>
#include <math.h>
#include <cmath>
#include <assert.h>
#include <tuple>

#include "common.h"

// === NODETYPE ===
enum NodeType : uint8_t {OR, AND};
int get_player(const NodeType& type);
NodeType operator!(const NodeType& type);
std::istream& operator>>(std::istream& i, NodeType& type);

// === BOARD ===
struct Board{
    friend class Artic_point;

    board_int white;
    board_int black;
    int score_left;
    int score_right;
    //board_int blocked_lines;
    NodeType node_type;
    board_int forbidden_all;
    static board_int base_forbidden;

    Board();
    Board(const Board& b);
    Board(const Board& b, int action, int player);
    bool operator==(const Board& b) const;
    void init();

    // === ACTION FUNCTIONS ===
    void move(const int action, const int player);
    void move(std::vector<int> actions, int& player);
    void set_black(const int action);
    unsigned int find_empty(Line_info& line, int skip_field);
    bool is_valid(const int action) const;
    board_int get_valids() const;

    board_int get_valids_without_ondegree(const std::vector<Line_info> & all_lines) const;
    int get_valids_num() const;
    int get_active_line_num(const std::vector<Line_info> & lines) const;

    // === GAME OVER FUNCTIONS===
    bool white_win(const std::vector<Line_info> & lines) const;
    bool black_win() const;
    int get_winner(const std::vector<Line_info>& lines) const;

    // === TODO with saved constant array ===
    bool no_free_lines(const std::vector<Line_info>& all_lines) const;

    // === Heuristic for board ===
    bool heuristic_stop(const std::vector<Line_info>& all_lines) const;
    double heuristic_value(const std::vector<Line_info>& all_lines) const;
    std::string heuristic_layers(const std::vector<Line_info>& all_lines) const;
    std::array<float, ACTION_SIZE> heuristic_mtx(const std::vector<Line_info>& lines) const;

    // === Simplify board ===
    int one_way(const std::vector<Line_info>& all_lines) const;
    void remove_dead_fields_all(const std::vector<Line_info> &all_line, board_int forbidden);
    void remove_2lines_all(const std::vector<Line_info>& all_line, board_int forbidden);
    void remove_lines_with_two_ondegree(const std::vector<Line_info>& all_line, board_int forbidden);

    // === Symmetry ===
    void flip();

    std::string to_string() const;

};

inline bool Board::is_valid(const int action) const{
    return !((white | black) & ((1ULL)<<action));
}

inline board_int Board::get_valids() const{
    return ~(white | black) & FULL_BOARD;
}

inline int Board::get_valids_num() const{
    return __builtin_popcountll(get_valids());
}

inline void Board::set_black(const int action){
    if((white & ((1ULL) << action))>0){
        white = white ^ ((1ULL) << action);
    }
    //while &= ~((1ULL) << action);
    black |= ((1ULL) << action);
}

inline bool Board::black_win() const {
    // === No free field ===
    return __builtin_popcountll(white | black) == MAX_ROUND;
}

inline void Board::init(){
    white = 0;
    black = 0;
    //blocked_lines = 0;
    score_left = 0;
    score_right = 0;
    node_type = OR;
    forbidden_all = base_forbidden;
}

// === ACTION FUNCTIONS ===
inline void Board::move(const int action, const int player){
    assert(player == get_player(node_type));
    
    if(player == 1) white |= ((1ULL)<<action);
    else if (player == -1) black |= ((1ULL)<<action);
    else{
        std::cout<<"Bad player\n";
    }
    node_type = (!node_type);
}

struct Board_Hash{
    std::size_t operator()(Board const& b) const noexcept{
        std::size_t h1 = std::hash<uint64_t>{}(b.white);
        std::size_t h2 = std::hash<uint64_t>{}(b.black);
        std::size_t h3 = std::hash<uint64_t>{}(b.score_left);
        std::size_t h4 = std::hash<uint64_t>{}(b.score_right);
        std::size_t h5 = std::hash<uint8_t>{}(b.node_type);
        std::size_t h6 = std::hash<uint8_t>{}(b.forbidden_all & ~(b.white | b.black));

        return  h1 ^ h2 ^ h3 ^ h4 ^ h5 ^ h6;
    }
};

struct Vector_Hash{
    std::size_t operator()(const std::vector<unsigned long>& v) const noexcept{
        std::size_t h_all = 0;
        for(auto h: v){
            h_all ^= std::hash<uint64_t>{}(h);
        }
        return  h_all;
    }
};
