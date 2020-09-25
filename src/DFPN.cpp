#include "PNS.h"
#include "limits.h"
#include "assert.h"
#include "common.h"

void PNS::init_DFPN_search(PNSNode* node){
    add_state(node->board, node);

    node->set_theta_th(UINT_MAX);
    node->set_delta_th(UINT_MAX);
}

void f(){
    int a= 0;
}

void PNS::DFPN_search(PNSNode* node){
    assert(node != nullptr);
    if(node->pn == 0 || node->dn == 0) return;
    //stats(node, true);
    //display(node->board, true);
    //std::cout<<node->theta() <<" "<< node->theta_th() <<" "<< node->delta() <<" "<< node->delta_th()<<std::endl;

    while(node->theta() < node->theta_th() && node->delta() < node->delta_th()){
        unsigned int min_ind = update_threshhold(node);
        //int second_ind = -1;
        //unsigned int min_ind = get_min_delta_index(node, second_ind);
        if(node->type == OR){ // === OR  node ===

            if(min_ind == (-1)); // Disproof found
            else if(node->children[min_ind] == nullptr) extend(node, min_ind, false);
            else{
                update_threshhold(node);
                DFPN_search(node->children[min_ind]);
            }
            // === Update PN and DN in node ===
            node->pn = get_min_children(node, PN, false);
            node->dn = get_sum_children(node, DN);
            if(min_ind == (-1)) break;
        }
        else{                 // === AND node ===
            if(min_ind == (-1)); // Proof found
            else if(node->children[min_ind] == nullptr) extend(node, min_ind, false);
            else{
                update_threshhold(node);
                DFPN_search(node->children[min_ind]);
            }
            // === Update PN and DN in node ===
            node->pn = get_sum_children(node, PN);
            node->dn = get_min_children(node, DN, false);
            if(min_ind == (-1)) break;
        }

        if(node->pn == 0 || node->dn == 0) break;
        
    }
    
    // If PN or DN is 0, delete all unused descendents
    if(node->pn == 0 || node->dn == 0){
        delete_node(node);
    }
}

unsigned int PNS::update_threshhold(PNSNode* node){
    int second_ind;
    unsigned int min_ind = get_min_delta_index(node, second_ind);
    if(min_ind == UINT_MAX){
        //assert(0);
        return -1;
    }
    else if (node->children[min_ind] == nullptr){
        return min_ind;
    }

    PNSNode* n_c = node->children[min_ind];

    if(second_ind == UINT_MAX || node->children[second_ind] == nullptr){
        node->children[min_ind]->set_delta_th(node->theta_th());
        //node->set_theta_th(n_c->delta_th());
    }
    else{
        unsigned int pn_second = node->children[second_ind]->delta();
        node->children[min_ind]->set_delta_th(std::min(node->theta_th(), pn_second == (-1)? pn_second:pn_second +1));
    }

    unsigned int new_theta_th = node->theta_th()-node->theta()+n_c->theta();
    node->children[min_ind]->set_theta_th(new_theta_th);
    return min_ind;
}

/*
unsigned int PNS::update_threshhold(PNSNode* node){
    int second_ind;
    unsigned int min_ind = get_min_delta_index(node, second_ind);
    if(min_ind == UINT_MAX){
        assert(0);
        return -1;
    }
    else if (node->children[min_ind] == nullptr){
        //assert(0);
        return min_ind;
    }

    PNSNode* n_c = node->children[min_ind];

    unsigned int new_th = node->delta_th()+n_c->theta();
    for(unsigned int i=0;i<ACTION_SIZE;i++){
        if(node->board.is_valid(i) && node->children[i] != nullptr){
            new_th-=node->children[i]->theta();
        }
    }

    node->set_theta_th(new_th);
    node->set_delta_th((second_ind == UINT_MAX || node->children[second_ind] == nullptr) ? node->theta_th() : std::min(node->theta_th(), node->children[second_ind]->delta()+1));
    return min_ind;
}*/