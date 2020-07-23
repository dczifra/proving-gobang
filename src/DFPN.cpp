#include "PNS.h"
#include "limits.h"
#include "assert.h"
#include "common.h"

void init_DFPN_search(PNSNode* node){
    add_state(node->board, node);

    unsigend int& theta_th = node->theta_th();
    theta_th = UINT_MAX;
    unsigned int& delta_th = node->delta_th();
    delta_th = UINT_MAX;
}

void PNS::DFPN_search(PNSNode* node){
    assert(node != nullptr);
    if(node->pn == 0 || node->dn == 0) return;

    while(node->theta() >= node->theta_th && node->delta() >= node->delta_th){
        if(node->type == OR){ // === OR  node ===
            unsigned int min_ind = get_min_children(node, PN, true);

            if(min_ind == (-1)) 0; // Disproof found
            else if(node->children[min_ind] == nullptr) extend(node, min_ind);
            else DFPN_search(node->children[min_ind]);
            // === Update PN and DN in node ===
            node->pn = get_min_children(node, PN);
            node->dn = get_sum_children(node, DN);
        }
        else{                 // === AND node ===
            unsigned int min_ind = get_min_children(node, DN, true);

            if(min_ind == (-1)) 0; // Proof found
            else if(node->children[min_ind] == nullptr) extend(node, min_ind);
            else DFPN_search(node->children[min_ind]);
            // === Update PN and DN in node ===
            node->pn = get_sum_children(node, PN);
            node->dn = get_min_children(node, DN);
        }
    }

    // If PN or DN is 0, delete all unused descendents
    if(node->pn == 0 || node->dn == 0){
        delete_node(node);
    }
}