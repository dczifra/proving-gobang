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
#define ROW 4
#define COL 9
#define LINEINROW 7
#define LAYERNUM 7

#define BITSIZE 64
#define board_int std::uint64_t

#define INNER_LINE  true
#define DIFFERENT_CORNER  false

#define EVAL_TRESHOLD 12
#define TALKY false
#define ISOM true
#define HEURISTIC false

#if HEURISTIC
    #define var double
    #define var_MAX DBL_MAX
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

// ============================================
//                DISPLAY
// ============================================
template<class T>
using mtx = std::array<std::array<T,ROW>, COL> ;
void print_v(std::vector<int>& v);
struct Board;
void display(const Board board, bool end, std::vector<int> show = std::vector<int>());
void display(const mtx<int>& board, bool end, std::vector<int> show);
void display(const board_int board, bool end, std::vector<int> show = std::vector<int>());

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
