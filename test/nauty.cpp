#define MAXN 64
#include "../nauty27r1/nauty.h"
#include <iostream>

// === Compile ===
// g++ -o main nauty.cpp ../nauty27r1/nauty.c ../nauty27r1/nautil.c ../nauty27r1/naugraph.c ../nauty27r1/schreier.c ../nauty27r1/naurng.c 
// TODO: set putstring second argument to const in nauty (for no warnings)!!!
int main(){
    std::cout<<"Nauty 27\n";
    std::cout<<"Maxm: "<<MAXM<<" "<<std::endl;

    // === Basic variables ===
    set s[MAXM];  /* a set */
    graph g[MAXN*MAXM];  /* a graph */
    int xy[MAXN];  /* an array */

    // === Init arrays  dinamically (2. option)===
    //DYNALLSTAT(set,s,s_sz);
    //DYNALLSTAT(graph,g,g_sz);
    //DYNALLSTAT(int,xy,xy_sz);

    //std:cout<<s_sz<<std::endl;
    return 0;
}
