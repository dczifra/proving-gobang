#pragma once

#include "common.h"

#define simple_max(A,B) ((A)<(B)?(B):(A))
#define MAXN (ROW*COL+2*ROW+3*COL+simple_max(0,COL-8)*ROW+1)
//#define MAXN (ROW*COL+2*ROW+3*COL+simple_max(0,COL-8)*ROW+1)

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
        options.getcanon = true;
        options.defaultptn = false;
    }

    ~CanonicalOrder(){
        delete[] g;
        delete[] cg;
    }

    std::vector<setword> get_canonical_graph(const Board& b, const std::vector<Line_info>& all_linesinfo, int* labs=NULL){
        // === Set new indexes for every empty field ===
        int index[ACTION_SIZE];
        int sum = 0;
        for(int i=0;i<ACTION_SIZE;i++){
            if(b.is_valid(i)) index[i] = (sum++);
            else index[i]=-1;
        }
        
        // === Init empty graph ===
        int nodes = b.get_valids_num();
        int n = nodes+b.get_active_line_num(all_linesinfo) + 1;
        int m = SETWORDSNEEDED(n);
        nauty_check(WORDSIZE,m,n,NAUTYVERSIONID);

        //std::cout<<n<< " "<<m<<std::endl;
        EMPTYGRAPH(g,m,n);
        if(b.node_type == OR){
            ADDONEEDGE(g,n-1,n-1,m);
        }

        // === Add edges ===
        int line_ind = 0;
        for(auto line: all_linesinfo){
            bool is_free = !(line.line_board & b.black);
            if(is_free){
                //display(line.line_board, true);
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
        ptn[nodes-1] = 0; // 1. color nodes
        ptn[n-2] = 0;     // 2. color edges
        ptn[n-1] = 0;     // 3. color OR/AND bit

        // === Compute the canonical graph ===
        densenauty(g,lab,ptn,orbits,&options,&stats,m,n,cg);
        //Traces(g,lab,ptn,orbits,&options,&stats,m,n,cg);

        std::vector<setword> ret(cg,cg+m*n);
        //std::vector<setword> ret(m,0);
        //for(int i=0;i<m;i++) ret[i] = cg[i];
        if(labs != nullptr) for(int i=0;i<n;i++) labs[i] = lab[i];

        return ret;
    }

    void get_conversion(const Board& b1, const Board& b2, const std::vector<Line_info>& all_linesinfo){
        int lab1[MAXN];
        int lab2[MAXN];
        std::vector<uint64_t> cg1 = get_canonical_graph(b1, all_linesinfo, lab1);
        std::vector<uint64_t> cg2 = get_canonical_graph(b2, all_linesinfo, lab2);
        
        printf("Canonical graphs:\n");
        for(int i=0;i<cg1.size();i++){
            std::cout<<cg1[i]<<" "<<cg2[i]<<std::endl;
            if(cg1[i]!=cg2[i]){
                printf("Not equal\n");
                return;
            }
        }
        printf("Mapping\n");

        std::vector<uint64_t> map(MAXN, 0);
        int n = b1.get_valids_num()+b1.get_active_line_num(all_linesinfo) + 1;
        for(int i=0;i<n;i++){
            //std::cout<<lab1[i]<<" "<<lab2[i]<<"\n";
            map[lab1[i]] = lab2[i];
        }

        for(int i=0;i<n;i++){
            printf("%d %d\n", i, (int)map[i]);
        }
    }

    void print(std::vector<int>& v){
        for(int i=0;i<v.size();i++) std::cout<<v[i]<<" ";
        std::cout<<std::endl;
    }

    void print(int* v, int n){
        for(int i=0;i<n;i++) std::cout<<v[i]<<" ";
        std::cout<<std::endl;
    }
private:
    graph* g;
    graph* cg;
    int lab[MAXN],ptn[MAXN],orbits[MAXN];
    optionblk options;
    statsblk stats;
};
