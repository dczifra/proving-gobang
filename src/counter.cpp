#include "counter.h"

int Counter::count_nodes(PNS::PNSNode* node){
    if (node == nullptr) return 0;

    if(counted_states.find(node->board) != counted_states.end()){
        return 0;
    }
    else{
        counted_states[node->board] =  true;
    }

    
    int cnt = 1;
    for(int i=0;i<node->child_num;i++){
        cnt += count_nodes(node->children[i]);
    }
    return cnt;
}
