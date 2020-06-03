#include "PNS.h"
#include "limits.h"
#include "assert.h"
#include "common.h"

NodeType operator!(const NodeType& type){
    return (type==OR?AND:OR);
}

int get_player(const NodeType& type){
    return (type==OR?1:-1);
}

inline unsigned int get_child_value(PNSNode* child_node, const ProofType type){
    if(child_node == nullptr){
        return 1; // Improvement: Initialization policy
    }
    else if(type == PN){
        return child_node->pn;
    }
    else{
        return child_node->dn;
    }
}

unsigned int PNS::get_min_children(PNSNode* node, const ProofType type, bool index = false){
    unsigned int min = UINT_MAX;
    unsigned int min_ind = -1;

    for(int i=0;i<ACTION_SIZE;i++){
        if(!node->board.is_valid(i)) continue;
        
        unsigned int child = get_child_value(node->children[i], type);

        if(child < min ){
            min = child;
            min_ind = i;
        }
    }
    
    if(index) return min_ind;
    else return min;
}

unsigned int PNS::get_sum_children(PNSNode* node, const ProofType type){
    unsigned int sum = 0;

    for(int i=0;i<ACTION_SIZE;i++){
        if(!node->board.is_valid(i)) continue;
        
        unsigned int child = get_child_value(node->children[i], type);

        if(child == UINT_MAX) return UINT_MAX;
        sum += child;
    }
    return sum;
}

void PNS::extend(PNSNode* node, unsigned int action){
    Board next_state(node->board, action, get_player(node->type));
    NodeType t = !(node->type);
    node->children[action] = new PNSNode(next_state, t);

    if((node->type == OR) && next_state.white_win(lines(action))){
        node->children[action]->pn = 0;
        node->children[action]->pn = UINT_MAX;
        display(node->children[action]->board, true);
    }
}

void PNS::search(PNSNode* node){
    // === This node is a leaf --> Extend ===
    assert(node != nullptr);
    //display(node->board, true);

    if(node->type == OR){ // === OR  node ===
        unsigned int min_ind = get_min_children(node, PN, true);
        assert(min_ind>=0);

        if(node->children[min_ind] == nullptr) extend(node, min_ind);
        else search(node->children[min_ind]);
        // === Update PN and DN in node ===
        node->pn = get_min_children(node, PN);
        node->dn = get_sum_children(node, DN);


    }
    else{               // === AND node ===
        unsigned int min_ind = get_min_children(node, DN, true);
        assert(min_ind>=0);

        if(node->children[min_ind] == nullptr) extend(node, min_ind);
        else search(node->children[min_ind]);
        // === Update PN and DN in node ===
        node->pn = get_sum_children(node, PN);
        node->dn = get_min_children(node, DN);
    }

    if(node->pn == 0){
        node->dn = UINT_MAX;
    }
    if(node->pn == UINT_MAX){
        node->dn = 0;
    }
    if(node->dn == 0){
        node->pn = UINT_MAX;
    }
    if(node->dn == UINT_MAX){
        node->pn = 0;
    }
}
/*
void PNS::search_old(PNSNode* node){
    if(node == nullptr && valid()){
        return;
    }

    if(node->type == OR){
        // Select child with minimal pn
        unsigned int min = UINT_MAX;
        unsigned int min_ind = -1;
        unsigned int min_ind2 = -1;

        unsigned int sum = 0;
        for(int i=0;i<ACTION_SIZE;i++){
            if(!node->board.is_valid(i)) continue;
            
            unsigned int child_pn = node->children[i]->pn;
            sum += node->children[i]->dn; // dn == infty ???
            if(child_pn < min ){
                min = child_pn;
                min_ind2 = min_ind;
                min_ind = i;
            }
        }

        sum-=node->children[min_ind]->dn;
        search(node->children[min_ind]);
        sum+=node->children[min_ind]->dn;
        if(min_ind2 >= 0 && node->children[min_ind]->pn > node->children[min_ind2]->pn){
            node->pn = node->children[min_ind2]->pn;
        }
        else{
            node->pn = node->children[min_ind]->pn;
        }
        // IF node->pn == (0 v infty)
    }
    else{
        // Select child with minimal dn
        unsigned int min = UINT_MAX;
        unsigned int min_ind = -1;
        unsigned int min_ind2 = -1;

        unsigned int sum = 0;
        for(int i=0;i<ACTION_SIZE;i++){
            unsigned int child_pn = node->children[i]->dn;
            sum += node->children[i]->pn;
            if(child_pn < min && valid()){
                min = child_pn;
                min_ind2 = min_ind;
                min_ind = i;
            }
        }
        
        sum-=node->children[min_ind]->pn;
        search(node->children[min_ind]);
        sum+=node->children[min_ind]->pn;

        if(min_ind2 >= 0 && node->children[min_ind]->dn > node->children[min_ind2]->dn){
            node->dn = node->children[min_ind2]->dn;
        }
        else{
            node->dn = node->children[min_ind]->dn;
        }
        // IF node->pn == (0 v infty)
    }
}*/