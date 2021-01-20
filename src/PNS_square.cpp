#include "PNS.h"


void PNS::evaluate_node_with_PNS_square(PNSNode* node, bool log, bool fast_eval){
    int i=0;
    while(node->pn*node->dn != 0){
        PN_search_square(node, fast_eval);

        if(log && i%10000 == 0){
            stats(node);
        }
        ++i;
    }
}

void delete_all_child(PNS* tree, Node* node){
    for(int i=0;i<node->children.size();i++){
        if(node->children[i] != nullptr){
            tree->delete_all(node->children[i]);
            node->children[i] = nullptr;
        }
    }
    node->extended = false;
}

void PNS::search_and_keep_one_layer(Node* node, bool fast_eval, int iter){
    unsigned int x = states.size();
    float f = 1.0/(1+exp((args->A-x)/args->B));
    int N = iter == -1 ? (int) (states.size()*f) : iter;

    for(int i=0;i<N;i++){
        PN_search(node, fast_eval);
        if(node->pn*node->dn == 0){
            return;
        }
    }

    for(auto child: node->children){
        if(child == nullptr) continue;
        else if(child->is_inner()){
            // Inner node sholdn't be erase, erase granchilds intead!
            for(auto gchild: child->children){
                // All child of an inner node should be normal
                assert(!gchild->is_inner());
                delete_all_child(this, gchild);
            }
        }
        else{
            delete_all_child(this, child);
        }
    }
}

void PNS::PN_search_square(Node* node, bool fast_eval){
    assert(node != nullptr);
    // display_node(node);
    if(node->pn == 0 || node->dn == 0) return;

    // if we are in a leaf, we extend it
    if(!node->extended){
        PNSNode* heur_node = static_cast<PNSNode*>(node);
        extend_all(heur_node, fast_eval);
	    search_and_keep_one_layer(heur_node, fast_eval);
    }
    else{
        unsigned int min_ind = get_min_children_index(node, node->type == OR?PN:DN);
        if (min_ind != -1){
	        PN_search_square(node->children[min_ind], fast_eval);
        }
        update_node(node);
    }
    // If PN or DN is 0, delete all unused descendents
    if(node->pn == 0 || node->dn == 0){
        delete_and_log(node);
    }
}
