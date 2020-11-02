#include "PNS.h"
#include "logger.h"
#include "../nauty27r1/nauty.h"
#include <iostream>

// === Compile ===
// g++ -o main nauty.cpp ../nauty27r1/nauty.c ../nauty27r1/nautil.c ../nauty27r1/naugraph.c ../nauty27r1/schreier.c ../nauty27r1/naurng.c 
// TODO: set putstring second argument to const in nauty (for no warnings)!!!

// Questions:
//   * Free after malloc?
//   * global graph (avoid malloc all the time)
void init_graph(){
    // === Basic variables ===
    set s[MAXM];  /* a set */
    int xy[MAXN];  /* an array */
    graph g1[MAXN*MAXM], cg1[MAXN*MAXM];  /* a graph */
    graph g2[MAXN*MAXM], cg2[MAXN*MAXM];  /* a graph */

    // === (2. option) Init arrays  dinamically ===
    //DYNALLSTAT(set,s,s_sz);
    //DYNALLSTAT(graph,g,g_sz);
    //DYNALLSTAT(int,xy,xy_sz);
    //DYNALLOC1(int,lab,lab_sz,n,"malloc");

    int lab1[MAXN],ptn1[MAXN],orbits1[MAXN];
    int lab2[MAXN],ptn2[MAXN],orbits2[MAXN];
    static DEFAULTOPTIONS_GRAPH(options);
    statsblk stats;
    int n,m,v;

    //options.writeautoms = TRUE;
    options.getcanon = TRUE;
    options.defaultptn = TRUE;

    n = 6;
    m = SETWORDSNEEDED(n);
    nauty_check(WORDSIZE,m,n,NAUTYVERSIONID);
    for(int i=0;i<n;i++){
        lab1[i] = i;
        lab2[i] = i;
        ptn1[i] = 1;
        ptn2[i] = 1;
    }
    ptn1[n-1] = 0;
    ptn2[n-1] = 0;

    // === G1 ===
    EMPTYGRAPH(g1,m,n);
    ADDONEEDGE(g1,0,3,m);
    ADDONEEDGE(g1,1,3,m);
    ADDONEEDGE(g1,1,4,m);
    ADDONEEDGE(g1,2,3,m);
    ADDONEEDGE(g1,2,4,m);

    // === G2 ===
    EMPTYGRAPH(g2,m,n);
    ADDONEEDGE(g2,0,3, m);
    ADDONEEDGE(g2,0,4,m);
    ADDONEEDGE(g2,1,4,m);
    ADDONEEDGE(g2,2,4,m);

    densenauty(g1,lab1,ptn1,orbits1,&options,&stats,m,n,cg1);
    densenauty(g2,lab2,ptn2,orbits2,&options,&stats,m,n,cg2);

    printf("Automorphism group size = ");
    writegroupsize(stdout,stats.grpsize1,stats.grpsize2);
    printf("\n");

    printf("Canonical labeling:\n");
    for(int i=0;i<m*n;i++){
        std::cout<<cg1[i]<<" ";
    }
    std::cout<<std::endl;

    for(int i=0;i<m*n;i++){
        std::cout<<cg2[i]<<" ";
    }
    std::cout<<std::endl;
}

void convert_board(const Board& b){
    PNS tree;
    //std::cout<<(ROW*COL+tree.heuristic.all_linesinfo.size())<<" "<<MAXN<<std::endl;
    int nodes = b.get_valid_num();
    int n = nodes+b.get_active_line_num(tree.heuristic.all_linesinfo);
    int m = SETWORDSNEEDED(n);
    int index[ROW*COL];
    int sum = 0;
    for(int i=0;i<ROW*COL;i++){
        if(b.is_valid(i)) index[i] = (sum++);
        else index[i]=-1;
    }

    // === Declare variables ===
    //DYNALLSTAT(graph,g,g_sz);
    std::cout<<n<<" "<<nodes<<" "<<b.get_active_line_num(tree.heuristic.all_linesinfo)<<std::endl;
    graph* g = new graph[MAXM*MAXN];
    DYNALLSTAT(graph,cg,cg_sz);
    int lab[MAXN],ptn[MAXN],orbits[MAXN];
    static DEFAULTOPTIONS_GRAPH(options);
    statsblk stats;
    options.getcanon = TRUE;
    
    nauty_check(WORDSIZE,m,n,NAUTYVERSIONID);
    // === INIT nodes and edges ===
    //DYNALLOC2(graph,g,g_sz,m,n,"malloc");
    DYNALLOC2(graph,cg,cg_sz,m,n,"malloc");
    EMPTYGRAPH(g,m,n);
    printf("%d %d\n", n, m);

    int line_ind = 0;
    for(auto line: tree.heuristic.all_linesinfo){
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
    ptn[nodes-1] = 0; // 1. color nodes
    ptn[nodes-3] = 0; // 2. color edges
    ptn[n-1] = 0;     // 3. color OR/AND bit


    // === The labeling ===
    densenauty(g,lab,ptn,orbits,&options,&stats,m,n,cg);
    printf("Canonical labeling:\n");
    for(int i=0;i<n;i++){
        std::cout<<cg[i]<<" ";
    }
    std::cout<<std::endl;
    std::cout<<"End"<<std::endl;

    delete[] g;
}

Heuristic PNS::heuristic;
CanonicalOrder PNS::isom_machine;
Logger* PNS::logger = new Logger();
int main(){
    std::cout<<"###################\n";
    printf("# Nauty           #\n#  * Version: %d  #\n", 27);
    printf("#  * MAXN: %d     #\n#  * MAXM: %d      #\n", MAXN, MAXM);
    std::cout<<"###################\n\n";

    init_graph();

    Board b;
    b.white = 8726315008;
    b.black = 1348403199;
    //convert_board(b);

    return 0;
}
