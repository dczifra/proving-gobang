#include <fstream>

#include "PNS.h"
#include "limits.h"
#include "assert.h"
#include "common.h"
#include "artic_point.h"
#include "logger.h"
#include "counter.h"

#include <unistd.h>

inline var get_child_value(Node* child_node, const ProofType type){
  assert(child_node != nullptr);

  if(type == PN){
    return child_node->pn;
  }
  else{
    return child_node->dn;
  }
}

unsigned int PNS::get_min_children_index(Node* node, const ProofType type){
  var min = var_MAX;
  unsigned int min_ind = -1;
  
  for(int i=0;i<node->child_num;i++){
    if (node->children[i] == nullptr) continue;
    var child = get_child_value(node->children[i], type);
    // std::cout<<"Child: "<<i<<" value: "<<child<< " dn: "<<node->children[i]->dn<< " extended: "<<node->children[i]->extended<<" heur val: "<<node->children[i]->heuristic_value<<std::endl;

    if(child < min ){
      min = child;
      min_ind = i;
    }
  }
  
  return min_ind;
}

var PNS::get_min_children(Node* node, const ProofType type) {
  var min = var_MAX;
  
  for(int i=0;i<node->child_num;i++){
    if (node->children[i] == nullptr) continue;
    var child = get_child_value(node->children[i], type);
    
    if(child < min ){
      min = child;
    }
  }
  return min;
}

var PNS::get_sum_children(Node* node, const ProofType type) {
  var sum = 0;

  for(int i=0;i<node->child_num;i++){
    if (node->children[i] == nullptr) continue;
    
    var child = get_child_value(node->children[i], type);
    if(child == var_MAX) return var_MAX;
    sum += child;
  }
  return sum;
}

bool PNS::keep_only_one_child(Node* node){
    return ((node->type == OR) && (node->pn == 0)) ||
        ((node->type == AND) && (node->dn==0));
}

bool PNS::game_ended(const Board& b){
    if(b.white_win(heuristic.all_linesinfo)){
        return true;
    }
    else if((b.node_type == AND) && b.heuristic_stop(heuristic.all_linesinfo)){
        return true;
    }
    else if(b.white == 0 && b.black == FULL_BOARD){
        return true;
    }
    //else if((b.node_type == AND) && (b.heuristic_value(heuristic.all_linesinfo)*128 < args->potencial_n-0.000000001)){
    //    return true;
    //}
    return false;
}

void PNS::defender_get_favour_points(Board& next_state, int last_action){
    if(next_state.node_type == OR){
        int reward = Node::is_empty_side(last_action, next_state) ? 0 : licit.cover_forbiden_reward;
        // === The last action was a defender move ===
        if(heuristic.forbidden_fields_left & (1ULL <<last_action)){
            next_state.score_left += reward;
            next_state.score_left = clip(next_state.score_left, -licit.max_score, licit.max_score);
        }
        else if(heuristic.forbidden_fields_right & (1ULL <<last_action)){
            next_state.score_right += reward;
            next_state.score_right = clip(next_state.score_right, -licit.max_score, licit.max_score);
        }
    }
}

Node* PNS::licit_for_defender_move(const Board& act_board, int action){
    return new AttackerOnForbidden(this, act_board, action);
}

void PNS::simplify_board(Board& next_state){
    while(!game_ended(next_state)){
        if(next_state.node_type == OR){ // Last move: black
            next_state.remove_lines_with_two_ondegree(heuristic.all_linesinfo, heuristic.forbidden_all);
            next_state.remove_2lines_all(heuristic.all_linesinfo, heuristic.forbidden_all);
        }
        int temp_act = next_state.one_way(heuristic.all_linesinfo);
        if(temp_act > -1 && !((1ULL << temp_act) & heuristic.forbidden_all)){
            next_state.move(temp_act, next_state.node_type== OR ? 1 : -1);
        }
        else{
            break;
        }
    }
    next_state.remove_dead_fields_all(heuristic.all_linesinfo, heuristic.forbidden_all); // TODO can go out
}

PNSNode* PNS::create_and_eval_node(Board& board, bool eval){
    PNSNode* node;

    node = get_states(board);
    if(node != nullptr){
        node -> parent_num += 1;
    }
    else{
        node = new PNSNode(board, args);
        add_board(board, node);
    }

    if(eval){
        evaluate_node_with_PNS(node, false, false);
    }
    return node;
}

PNSNode* PNS::evaluate_components(Board& base_board){
    assert(base_board.node_type == OR);
    bool delete_comps = true;
    
    int artic_point;
    Board small_board, big_board;
    Artic_point comps(base_board, heuristic.all_linesinfo, heuristic.linesinfo_per_field);
    std::tie(artic_point, small_board, big_board) = comps.get_parts();

    if(artic_point > -1){
        // 1. Evalute smaller component without artic point
        Board small_board_with_artic(small_board);        
        simplify_board(small_board);
        PNSNode* small_comp = create_and_eval_node(small_board, true);

        if(small_comp->pn == 0){
            return small_comp;
        }
        else{
            if (delete_comps) {
                delete_all(small_comp);
            }

            // 2. Evaluate small component with artic point
            (small_board_with_artic.white) |= ((1ULL)<<artic_point);
            simplify_board(small_board_with_artic);
            PNSNode* small_comp_mod = create_and_eval_node(small_board_with_artic, true);

            // 3-4. If Attacker wins: C* else C
            if(small_comp_mod->pn == 0){
                big_board.white |= ((1ULL)<<artic_point);
            }
            if (delete_comps){
              delete_all(small_comp_mod);
            }

            // TODO: Improve...
            simplify_board(big_board);
            PNSNode* big_comp = create_and_eval_node(big_board, false);
            return big_comp;
        }
    }
    else{
        PNSNode* node = new PNSNode(base_board, args);
        add_board(base_board, node);
        return node;
    }
}

void PNS::evaluate_node_with_PNS(Node* node, bool log, bool fast_eval){
    int i=0;
    // int update_iteration = 1000;
    while(node->pn*node->dn != 0){
        PN_search(node, fast_eval);

        if(log && i%10000 == 0){
            stats(node);
        }
        ++i;
    }
}

void PNS::extend_all(PNSNode* node, bool fast_eval){
    // std::cout<<"extend_all"<<std::endl;
    if(node == nullptr) std::cout<<"died in extend_all"<<std::endl;
    if((node->pn == 0) || (node->dn == 0) || node->extended) return;
    int slot = 0;

    board_int valids = node->board.get_valids_without_ondegree(heuristic.all_linesinfo);
    for(int i=0;i<ACTION_SIZE;i++){
        if(valids & (1ULL << i)){
            extend(node, i, slot, fast_eval);
            // if(node->children[slot]->type == OR){
            //   extend_all(node->children[slot], fast_eval);
            // }
            if ((node->type == OR) and (node->children[slot]->pn==0) or
		(node->type == AND) and (node->children[slot]->dn==0)) break;
            slot++;
        }
    }
    node->extended = true;

#if HEURISTIC
    // default DN is not useful in OR nodes, so we update them
    if(node->type == AND){
        float heur_parent = node->heuristic_value;
        float heur_min = FLT_MAX;
        for(auto child: node->children){
            if(child == nullptr) continue;
            if(child->type == OR){
                heur_min = std::min(heur_min, ((PNSNode*) child)->heuristic_value);
            }
        }
        for(Node* current_child: node->children){
            if(current_child == nullptr) continue;
            if(current_child->type == OR and current_child->dn != 0 and current_child->pn != 0){
                float curr_heur = ((PNSNode*) current_child)->heuristic_value;
                current_child->dn = std::pow(1000, heur_parent - heur_min + curr_heur);
            }
        }
    }
        
#endif

    update_node(node);
}

void PNS::extend(PNSNode* node, unsigned int action, unsigned int slot,
		  bool fast_eval){
    if((1ULL << action) & node->board.forbidden_all){
        node->children[slot] = strategy.move_on_common(node->board, action);
        if(node->children[slot]->is_inner()) update_node(node->children[slot]);
        //update_node(node->children[slot]);
        return;
    }
    Board next_state(node->board, action, get_player(node->type));
    simplify_board(next_state);

    PNSNode* child = get_states(next_state);
    if(child != nullptr){
        node->children[slot] = child;
        node->children[slot] -> parent_num += 1;
        return;
    }
    else{
        assert(node->children[slot] == nullptr);
        int moves_before = next_state.get_valids_num();

        // 2-connected components, if not ended
        if(0 && !fast_eval && next_state.node_type == OR && !game_ended(next_state) && moves_before >= EVAL_TRESHOLD){ 
            child = evaluate_components(next_state);
            node->children[slot] = child;
        }
        else{
            child = new PNSNode(next_state, args);
            add_board(next_state, child);
            if(!fast_eval && moves_before < EVAL_TRESHOLD){
                evaluate_node_with_PNS(child, false, true);
            }
            node->children[slot] = child;
        }
        int moves_after = child->board.white != -1 ? child->board.get_valids_num() : 0;
    
        component_cut[moves_before][moves_before - moves_after] += 1;
    }
}

void PNS::init_PN_search(PNSNode* node){
    add_board(node->board, node);
}

void PNS::delete_and_log(Node* node){
    #if LOG
    if(false && keep_only_one_child(node) && node->child_num > 0){
        int ind = rand() % node->child_num;
        if(node->children[ind] != nullptr && node->children[ind]->board.get_valids_num() < ACTION_SIZE*0.75){
            evaluate_node_with_PNS(node->children[ind]);
            logger->log(node->children[ind], heuristic);
        }
    }
    #endif
    delete_node(node);
    #if LOG
    logger->log(node, heuristic);
    #endif
}

void PNS::PN_search(Node* node, bool fast_eval){
    assert(node != nullptr);
    // display_node(node);
    //if(print) display(node->board, true, {}, true);
    if(node->pn == 0 || node->dn == 0) return;

    // if we are in a leaf, we extend it
    if(!node->extended){
        PNSNode* heur_node = static_cast<PNSNode*>(node);
        extend_all(heur_node, fast_eval);
        if(states.size() > PNS2_START && !fast_eval){
            search_and_keep_one_layer(heur_node, true);
        }
    }
    else{
        unsigned int min_ind = get_min_children_index(node, node->type == OR?PN:DN);
        if (min_ind != -1){
            PN_search(node->children[min_ind], fast_eval);
        }
        update_node(node);
    }
    // If PN or DN is 0, delete all unused descendents
    if((node->pn == 0 || node->dn == 0)){
        delete_and_log(node);
    }
}

void PNS::update_node(Node* node){
  // std::cout<<"updating from:"<<node->pn<<" "<<node->dn<<std::endl;
        

  // === Update PN and DN in node ===
  if(node->type == OR){
    node->pn = get_min_children(node, PN);
    node->dn = get_sum_children(node, DN);
  }
  else{
    node->pn = get_sum_children(node, PN);
    node->dn = get_min_children(node, DN);
  }
}

// ============================================
//             DELETE UNUSED NODES
// ============================================
void PNS::delete_all(Node* node){
    if(node == nullptr) return;
    else if( node->parent_num > 1 ){
        node->parent_num -= 1;
    }
    else{
        for(int i=0;i<node->child_num;i++){
            //          std::cout<<"child "<<i<<std::endl;
            delete_all(node->children[i]);
            node->children[i]=nullptr;
        }

        if(!node->is_inner()){
            delete_from_map(node->get_board());
        }
        delete node;
    }
}

  //We only keep a single child that proves the given node
void PNS::delete_node(Node* node){
    // === In this case, we need max 1 branch ===
    // === DON'T DELETE THIS IF ===
    if(keep_only_one_child(node)){
        ProofType proof_type = (node->pn == 0 ? PN:DN);
        unsigned int min_ind = get_min_children_index(node, proof_type);
    
        assert(min_ind !=-1);
        // === Delete all children, except min_ind
        for(int i=0;i<node->children.size();i++){
            if( (node->children[i]==nullptr) || (min_ind == i)) continue;
            else{
                delete_all(node->children[i]);
                node->children[i]=nullptr;
            }
        }
    }
    return;
}

void PNS::free_states(){
    for(auto node_it: states){
        delete node_it.second;
    }
}

// ============================================
//                STORING STATES
// ============================================
void PNS::add_board(const Board& board, PNSNode* node){
    #if ISOM
        std::vector<uint64_t> isom = isom_machine.get_canonical_graph(board, heuristic.all_linesinfo);
        assert(states.find(isom) == states.end());
        states[isom] = node;
    #else
        assert(states.find(board) == states.end());
        states[board] = node;
    #endif
        total_state_size+=1;
}

PNSNode* PNS::get_states(const Board& board){
    #if ISOM
        std::vector<uint64_t> isom = isom_machine.get_canonical_graph(board, heuristic.all_linesinfo);
        //assert(states.find(isom) != states.end());
        if(states.find(isom) != states.end()){
            return states[isom];
        }
        else{
            return nullptr;
        }
    #else
        //Board reversed(board);
        //reversed.flip();

        if(states.find(board) != states.end()){
            return states[board];
        }
        //else if(states.find(reversed) != states.end()){
        //    return states[reversed];
        //}
        else{
            //assert(states.find(board) != states.end());
            return nullptr;
        }
    #endif
}

void PNS::delete_from_map(const Board& board){
    #if ISOM
        std::vector<uint64_t> isom = isom_machine.get_canonical_graph(board, heuristic.all_linesinfo);
        assert(states.find(isom) != states.end());
        states.erase(isom);
    #else
        assert(states.find(board) != states.end());
        states.erase(board);
    #endif
}

void PNS::display_node(PNSNode* node){
    display(node->board, true);
    std::cout<<std::endl;
    std::cout<<"Node type: "<<node->type<<" "<<node->pn<<" - "<<node->dn<<std::endl;
    if(!node->extended){
        std::cout<<"   leaf"<<std::endl;
    }
    else{
        int i=0;
        for(auto child: node->children){
            if (child == nullptr) {
                std::cout<<"Child: "<<i<<" nullptr"<<std::endl;
            }
            else{
                PNSNode* act_child = static_cast<PNSNode*>(child);
                if(act_child != nullptr){
                    std::cout<<"Child: "<<i<< " dn: "<<act_child->dn<< " pn: "<<act_child->pn<<" extended: "<<
                        act_child->extended<<" heur val: "<<act_child->heuristic_value<<std::endl;
                }
            }
            i++;
        }
    }
}

// ============================================
//                   STATS
// ============================================
void PNS::component_stats() {
  std::cout<< "COMPONENT CUT"<<std::endl;
  for (int depth=1; depth <= ACTION_SIZE; depth++) {
    int cnt = 0;
    int gainsum = 0;
    std::cout<<"Depth "<< depth <<": ";
    for (int j=0; j < ACTION_SIZE - depth; j++) {
      int freq = component_cut[ACTION_SIZE-depth][j];
      cnt += freq;
      gainsum += freq * j;
      std::cout<< freq << " ";
    }
    if (cnt > 0) {
        std::cout<<"---> "<< (float) gainsum / (float) cnt << std::endl;
      }
    else {
        std::cout<<"---> "<< 0 << std::endl;
    }
  }
}

void PNS::stats(Node* node, bool end){
    unsigned int x = states.size();
    float f = 1.0/(1+exp((args->A-x)/args->B));
    int N = (int) (states.size()*f);
    if(node != nullptr) std::cout<<"\rPN: "<<node->pn<<" DN: "<<node->dn<<" ";
    std::cout<<"States size: "<<states.size()<<" "<<N<<"        "<<std::flush;
    if(end) std::cout<<std::endl;
}
