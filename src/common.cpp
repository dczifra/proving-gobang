#include "common.h"
#include "board.h"

template<class T>
void print_mtx(mtx<T>& data, int prec=5){
    std::cout.precision(prec);
    std::cout << std::fixed;
    
    for(int j=0;j<ROW;j++){
        for(int i=0;i<COL;i++){
            std::cout<<data[i][j]<<" ";
        }
        std::cout<<"\n";
    }
}

unsigned long long set_full_board(){
    unsigned long long board;
    for(int i=0;i<ROW*COL;i++) board|=(1ULL<<i);
    return board;
}

void display(mtx<int>& board, bool end){
    const std::string WARNING = "\033[93m";
    const std::string FAIL = "\033[91m";
    const std::string ENDC = "\033[0m";


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
    
    printf(" +%s+\n",std::string(2*COL,'=').c_str());
    for(int x =0;x<ROW;x++){
        printf("%d|",x);
        for(int y =0;y<COL;y++){
            int piece = board[y][x];
            if(piece>0){
                printf("%s%s%s", WARNING.c_str(), "o ", ENDC.c_str());
            }
            else if(piece<0){
                printf("%s%s%s", FAIL.c_str(), "x ", ENDC.c_str());
            }
            else{
                printf("  ");
            }
        }
        printf("|\n");
    }
    printf(" +%s+\n",std::string(2*COL,'=').c_str());
    if (!end) printf("\033[%dA",back_step);
    printf("                           \n");
    printf("\033[1A");
}

void display(const board_int board, bool end){
    mtx<int> big_board;
    for(int x=0;x<ROW;x++){
        for(int y=0;y<COL;y++){
            int white = (board & ((1ULL)<<(y*ROW+x)))>0;
            big_board[y][x] = white;
        }
    }
    display(big_board, end);
}

void display(const Board board, bool end){
    mtx<int> big_board;
    for(int x=0;x<ROW;x++){
        for(int y=0;y<COL;y++){
            int white = (board.white & ((1ULL)<<(y*ROW+x)))>0;
            int black = (board.black & ((1ULL)<<(y*ROW+x)))>0;
            big_board[y][x] = white-black;
        }
    }
    display(big_board, end);
}

template<class T>
void make_zero(mtx<T>& data){
    for(int j=0;j<COL;j++){
        for(int k=0;k<ROW;k++){
            data[j][k]=0;
        }
    }
}

unsigned int selectBit(const board_int v, unsigned int r) {
    // Source: https://graphics.stanford.edu/~seander/bithacks.html
    // v - Input:  value to find position with rank r.
    // r - Input: bit's desired rank [1-64].
    unsigned int s;      // Output: Resulting position of bit with rank r [1-64]
    uint64_t a, b, c, d; // Intermediate temporaries for bit count.
    unsigned int t;      // Bit count temporary.

    // Do a normal parallel bit count for a 64-bit integer,
    // but store all intermediate steps.
    a =  v - ((v >> 1) & ~0UL/3);
    b = (a & ~0UL/5) + ((a >> 2) & ~0UL/5);
    c = (b + (b >> 4)) & ~0UL/0x11;
    d = (c + (c >> 8)) & ~0UL/0x101;
    t = (d >> 32) + (d >> 48);
    // Now do branchless select!
    s  = 64;
    s -= ((t - r) & 256) >> 3; r -= (t & ((t - r) >> 8));
    t  = (d >> (s - 16)) & 0xff;
    s -= ((t - r) & 256) >> 4; r -= (t & ((t - r) >> 8));
    t  = (c >> (s - 8)) & 0xf;
    s -= ((t - r) & 256) >> 5; r -= (t & ((t - r) >> 8));
    t  = (b >> (s - 4)) & 0x7;
    s -= ((t - r) & 256) >> 6; r -= (t & ((t - r) >> 8));
    t  = (a >> (s - 2)) & 0x3;
    s -= ((t - r) & 256) >> 7; r -= (t & ((t - r) >> 8));
    t  = (v >> (s - 1)) & 0x1;
    s -= ((t - r) & 256) >> 8;
    return s;
}