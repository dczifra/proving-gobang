#include "counter.h"

int Counter::count_nodes(PNS::PNSNode* node){
    if (node == nullptr) return 0;

    if(states.find(node->board) != states.end()){
        return 0;
    }
    else{
        states[node->board] =  true;
    }

    
    int cnt = 1;
    for(int i=0;i<node->child_num;i++){
        cnt += count_nodes(node->children[i]);
    }
    return cnt;
}

int Counter::update_tree(PNS::PNSNode* node){
    if (node == nullptr) return 0;

    if(states.find(node->board) != states.end()){
        return 0;
    }

    if(!node->extended) {
        states[node->board] =  true;
        return 0;
    }
    
    int cnt = 0;
    for(int i=0;i<node->child_num;i++){
        cnt += update_tree(node->children[i]);
    }
    unsigned int last_pn = node->pn;
    unsigned int last_dn = node->dn;        
    PNS::update_node(node);
    states[node->board] =  true;
    
    if (node->pn != last_pn || node->dn != last_dn) {
        return cnt + 1;
    }
    else{
        return cnt;
    }
}
