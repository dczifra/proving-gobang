#pragma once

#include "common.h"
#include "heuristic.h"

#define simple_max(A,B) ((A)<(B)?(B):(A))
#define MAXN (ROW*COL+2*ROW+3*COL+simple_max(0,COL-8)*ROW+2)

#include "../nauty27r1/nauty.h"

class CanonicalOrder{
public:
    CanonicalOrder(){
        // === INIT nodes and edges ===
        g = new graph[MAXN*MAXM];
        cg = new graph[MAXN*MAXM];


        // === Declare variables ===
        static DEFAULTOPTIONS_GRAPH(opt);
        options = opt;
        options.getcanon = TRUE;
    }
    ~CanonicalOrder(){
        delete[] g;
        delete[] cg;
    }

    std::vector<int> get_canonical_graph(const Board& b, const std::vector<Line_info>& all_linesinfo){
        int index[ROW*COL];
        int sum = 0;
        for(int i=0;i<ROW*COL;i++){
            if(b.is_valid(i)) index[i] = (sum++);
            else index[i]=-1;
        }
        
        int nodes = b.get_valid_num();
        int n = nodes+b.get_active_line_num(all_linesinfo) + 2;
        int m = SETWORDSNEEDED(n);
        nauty_check(WORDSIZE,m,n,NAUTYVERSIONID);

        EMPTYGRAPH(g,m,n);
        //std::cout<<m<<" "<<n<<" "<<options.getcanon<<std::endl;
        //std::cout<<b.white<<" "<<b.black<<std::endl;
        //display(b, true);
        if(b.node_type == OR) ADDONEEDGE(g,n-2,n-1,m);

        // === Add edges ===
        int line_ind = 0;
        for(auto line: all_linesinfo){
            bool is_free = !(line.line_board & b.black);
            if(is_free){
                for(auto field : line.points){
                    if(b.is_valid(field)){
                        //printf("%d %d\n", nodes+line_ind, index[field]);
                        ADDONEEDGE(g,nodes+line_ind,index[field],m);
                    }
                }
                line_ind++;
            }
        }
        // === INIT colors ===
        for(int i=0;i<n;i++){
            ptn[i] = 1;
            lab[i] = i;
        }
        ptn[nodes] = 0;   // 1. color nodes
        ptn[nodes-3] = 0; // 2. color edges
        ptn[n-1] = 0;     // 3. color OR/AND bit

        // === Compute the canonical graph ===
        densenauty(g,lab,ptn,orbits,&options,&stats,m,n,cg);

        std::vector<int> ret(cg, cg+n);
        return ret;
    }

private:
    graph* g;
    graph* cg;
    int lab[MAXN],ptn[MAXN],orbits[MAXN];
    optionblk options;
    statsblk stats;
};