#pragma once

#include <array>
#include <vector>
#include <iostream>
#include <stdio.h>
#include <numeric>
#include <iomanip>
#include <iostream>

// ============================================
//                BOARD INFOS
// ============================================
#define ROW 4
#define COL 7
#define LINEINROW 7
#define LAYERNUM 7

#define BITSIZE 64
#define board_int std::uint64_t
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
struct Board;
void display(const Board board, bool end);
void display(const mtx<int>& board, bool end);
void display(const board_int board, bool end);

// ============================================
//      Line infos (Shouldn't be in heuristic)
// ============================================
struct Line_info{
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