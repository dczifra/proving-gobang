#include "counter.h"

int Counter::count_nodes(PNS::Node* node){
    if (node == nullptr) return 0;

    PNS::PNSNode* heur_node = dynamic_cast<PNS::PNSNode*>(node);
    if(heur_node != nullptr && states.find(heur_node->board) != states.end()){
        return 0;
    }
    else if(heur_node != nullptr){
        states[heur_node->board] =  true;
    }

    
    int cnt = 1;
    for(int i=0;i<node->child_num;i++){
        cnt += count_nodes(node->children[i]);
    }
    return cnt;
}

int Counter::update_tree(PNS::Node* node){
    if(node == nullptr) return 0;

    PNS::PNSNode* heur_node = dynamic_cast<PNS::PNSNode*>(node);
    if(heur_node != nullptr){
        if(states.find(heur_node->board) != states.end()){
            return 0;
        }
        else if(!heur_node->extended) {
            states[heur_node->board] =  true;
            return 0;
        }
    }
        
    int cnt = 0;
    for(int i=0;i<node->child_num;i++){
        cnt += update_tree(node->children[i]);
    }
    unsigned int last_pn = node->pn;
    unsigned int last_dn = node->dn;        
    PNS::update_node(node);
    
    if(heur_node != nullptr) states[heur_node->board] =  true;
    
    if (node->pn != last_pn || node->dn != last_dn) {
        return cnt + 1;
    }
    else{
        return cnt;
    }
}
