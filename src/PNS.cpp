#include "PNS.h"
#include "limits.h"

void PNS::search(PNSNode* node){
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
            unsigned int child_pn = node->children[i]->pn;
            sum += node->children[i]->dn;
            if(child_pn < min && valid()){
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
        // Select child with minimal pn
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
}