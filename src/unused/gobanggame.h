#include <stdio.h>
#pragma once

class Player;

template<class T>
using mtx = std::array<std::array<T,ROW>, COL> ;
using Line = std::vector<std::pair<int,int>>;

//typedef std::pair<int, int>(Player:: *FunctPtr)(mtx<int>&);

class GobangGame{
 public:
    mtx<int> init();
    void move(const int action, mtx<int>& board, const int curPlayer);
    void display(mtx<int>& board, bool end);
    void play(bool first);
    void arena(Player* player1, Player* player2, bool log);
};



mtx<int> GobangGame::init(){
    mtx<int> table;
    make_zero(table);
    return table;
}

void GobangGame::move(const int action, mtx<int>& board, const int curPlayer){
    int y = action/ROW;
    int x = action%ROW;
    board[y][x]=curPlayer;
}

/*
void GobangGame::arena(Player* player1, Player* player2, bool log){
    mtx<int> board = init();
    int y,x;
    Player* p;
    for(int i=0;i<COL*ROW/2;i++){
        int actPlayer = 1-2*(i%2);
        std::tie(y,x)=player1->move(board, actPlayer, log);
        board[y][x]=1;
        if(log) display(board, false);

        std::tie(y,x)=player2->move(board, actPlayer, log);
        board[y][x]=-1;
        if(log) display(board, false);
    }
    if(log) display(board, true);
}*/





