#pragma once

#include <array>
#include <vector>
#include <iostream>
#include <stdio.h>
#include <numeric>
#include <iomanip>
#include <iostream>
#include <cfloat>
// ============================================
//                BOARD INFOS
// ============================================
#define ROW 5
#define COL 10
#define LINEINROW 7
#define LAYERNUM 7

#define BITSIZE 64
#define board_int std::uint64_t

#define INNER_LINE  true
#define DIFFERENT_CORNER  false
#define ONLY_4 false
#define LOG false

#define EVAL_TRESHOLD 5
#define ISOM false
#define HEURISTIC true
#define PNS2_START 250000000
#define LOG_CUT_DEPTH 2

#if HEURISTIC
    #define var float
    #define var_MAX FLT_MAX
#else
    #define var unsigned int
    #define var_MAX UINT_MAX
#endif

unsigned long long set_full_board();

const unsigned long long FULL_BOARD = set_full_board();
const unsigned int ACTION_SIZE = ROW*COL;
const unsigned int MAX_ROUND = ROW*COL;
// The number of bits need to be moved back after flip board:
const unsigned int FLIP_SIZE = BITSIZE-ACTION_SIZE;
const board_int flip_base = ((1ULL << 0) | (1ULL <<5) | (1ULL <<10) | (1ULL <<15) | (1ULL <<20) | (1ULL <<25) | (1ULL <<30) | (1ULL <<35) | (1ULL <<40) | (1ULL <<45));
// ============================================
//                  DISPLAY
// ============================================
template<class T>
using mtx = std::array<std::array<T,ROW>, COL> ;
void print_v(std::vector<int>& v);
struct Board;
void display(const Board board, bool end, std::vector<int> show = std::vector<int>(), bool nocolor=false);
void display(const std::vector<Board> board, bool end, std::vector<int> show = std::vector<int>(), bool nocolor=false);
void display(std::vector<mtx<int>>& boards, bool end, std::vector<std::pair<int, int>> scores, std::vector<int> show, bool nocolor);
void display(const board_int board, bool end,  std::vector<int> show = std::vector<int>(), bool nocolor=false);

// ============================================
//                   ARGS
// ============================================
struct Args{
    bool log = false;
    bool talky = true;
    bool play = false;
    bool test = false;
    bool disproof = false;
    bool show_lines = false;
    bool PNS_square = false;
    bool parallel = false;
    bool generate_parallel = false;
    float A = 200000000000.0;
    float B = 15000000000.0;
    int potencial_n = 128;
    int START=-1;
    std::string get_filename();
    Args(int argc, char* argv[]);
    Args(){}
};

inline int clip(int n, int lower, int upper){
    return n <= lower ? lower : n >= upper ? upper : n;
}

// ============================================
//      Line infos (Shouldn't be in heuristic)
// ============================================
struct Line_info{
    unsigned int index;
    unsigned int size;
    board_int line_board;
    std::vector<int> points;
};
unsigned int selectBit(const board_int v, unsigned int r);
unsigned int flip_bit(board_int val);


// === Why cannot we put it into the .cpp ??? ===
template <typename T>
void print_mtx(T& mtx, int prec=5){
    std::cout.precision(prec);
    std::cout << std::fixed;
    
    for(int j=0;j<ROW;j++){
        for(int i=0;i<COL;i++){
            std::cout<<mtx[i*ROW+j]<<" ";
        }
        std::cout<<"\n";
    }
}
