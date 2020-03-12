#include <stdio.h>
#pragma once

class Player;

typedef std::pair<int, int>(Player::*FunctPtr)(mtx<int>&);

class GobangGame{
 public:
    mtx<int> init();
    void move(const int action, mtx<int>& board, const int curPlayer);
    void display(mtx<int>& board, bool end);
    void play(bool first);
    void arena(Player* player1, Player* player2, bool log);
};

struct bcolors{
 public:
    const static std::string WARNING;
    const static std::string FAIL;
    const static std::string ENDC;
};

const std::string bcolors::WARNING = "\033[93m";
const std::string bcolors::FAIL = "\033[91m";
const std::string bcolors::ENDC = "\033[0m";

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

void GobangGame::display(mtx<int>& board, bool end){
    int back_step = ROW+5; 
    printf("                           \n");
    printf("  === Gobang Game ===\n");
    printf("  ");
    for(int i=0;i<COL;i++){
        std::string num = std::to_string(i);
        num = num.substr(num.size()-1,num.size());
        printf("%s ",num.c_str());
    }
    printf("\n");
    
    printf(" +========================+\n");
    for(int x =0;x<ROW;x++){
        printf("%d|",x);
        for(int y =0;y<COL;y++){
            int piece = board[y][x];
            if(piece>0){
                printf("%s%s%s", bcolors::WARNING.c_str(), "o ", bcolors::ENDC.c_str());
            }
            else if(piece<0){
                printf("%s%s%s", bcolors::FAIL.c_str(), "x ", bcolors::ENDC.c_str());
            }
            else{
                printf("  ");
            }
        }
        printf("|\n");
    }
    printf(" +========================+\n");
    if (!end) printf("\033[%dA",back_step);
    printf("                           \n");
    printf("\033[1A");
}





