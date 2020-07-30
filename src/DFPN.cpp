#include "PNS.h"
#include "limits.h"
#include "assert.h"
#include "common.h"

void PNS::init_DFPN_search(PNSNode* node){
    add_state(node->board, node);

    node->set_theta_th(UINT_MAX);
    node->set_delta_th(UINT_MAX);
}

void PNS::DFPN_search(PNSNode* node){
    assert(node != nullptr);
    if(node->pn == 0 || node->dn == 0) return;
    stats(node, true);

    while(node->theta() >= node->theta_th() && node->delta() >= node->delta_th()){
        if(node->type == OR){ // === OR  node ===
            unsigned int min_ind = get_min_children(node, PN, true);

            if(min_ind == (-1)) 0; // Disproof found
            else if(node->children[min_ind] == nullptr) extend(node, min_ind);
            else DFPN_search(node->children[min_ind]);
            // === Update PN and DN in node ===
            node->pn = get_min_children(node, PN, false);
            node->dn = get_sum_children(node, DN);
        }
        else{                 // === AND node ===
            unsigned int min_ind = get_min_children(node, DN, true);

            if(min_ind == (-1)) 0; // Proof found
            else if(node->children[min_ind] == nullptr) extend(node, min_ind);
            else DFPN_search(node->children[min_ind]);
            // === Update PN and DN in node ===
            node->pn = get_sum_children(node, PN);
            node->dn = get_min_children(node, DN, false);
        }
    }

    update_threshhold(node);

    // If PN or DN is 0, delete all unused descendents
    if(node->pn == 0 || node->dn == 0){
        delete_node(node);
    }
}

void PNS::update_threshhold(PNSNode* node){
    int second_ind;
    unsigned int min_ind = get_min_delta_index(node, second_ind);
    assert(min_ind != UINT_MAX);
    PNSNode* n_c = node->children[min_ind];

    unsigned int new_th = node->delta_th()+n_c->theta();
    for(unsigned int i=0;i<ACTION_SIZE;i++){
        if(node->board.is_valid(i)){
            new_th-=node->children[i]->theta();
        }
    }

    node->set_theta_th(new_th);
    node->set_delta_th(second_ind == UINT_MAX ? node->theta_th() : std::min(node->theta_th(), node->children[second_ind]->delta()+1));
}